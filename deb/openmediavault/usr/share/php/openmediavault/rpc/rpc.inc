<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
 */
namespace OMV\Rpc;

require_once("openmediavault/functions.inc");

/**
 * This class provides methods regarding remote procedure calls.
 * @ingroup api
 */
class Rpc {
	/**
	 * Create a context.
	 * @return The context using the specified parameters.
	 */
	public static function createContext($username, $role) {
		return [
			"username" => $username,
			"role" => $role
		];
	}

	/**
	 * Execute the given RPC.
	 * @param service The name of the service.
	 * @param method The name of the method.
	 * @param params The parameters hash object to be passed to the method
	 *   of the given service.
	 * @param context The context hash object of the caller containing the
	 *   fields \em username and \em role.
	 * @param mode The mode how to execute this RPC. The following modes
	 *   are available:<ul>
	 *   \li OMV_RPC_MODE_LOCAL
	 *   \li OMV_RPC_MODE_REMOTE
	 *   </ul>
	 *   Defaults to OMV_RPC_MODE_LOCAL.
	 * @param restoreSrvEnv Restore various web server and execution
	 *   environment information. This might be helpful in some cases if
	 *   these information are required in the engine backend. Note, this
	 *   only takes action when mode is OMV_RPC_MODE_REMOTE. Defauts to FALSE.
	 * @return The RPC response.
	 */
	public static function call($service, $method, $params, $context,
	  $mode = OMV_RPC_MODE_LOCAL, $restoreSrvEnv = FALSE) {
//		$this->debug(var_export(func_get_args(), TRUE));
		///////////////////////////////////////////////////////////////////////
		// Try to execute local RPC first.
		///////////////////////////////////////////////////////////////////////
		if ($mode & OMV_RPC_MODE_LOCAL) {
			$rpcServiceMngr = &\OMV\Rpc\ServiceManager::getInstance();
			if (FALSE !== ($rpcService = $rpcServiceMngr->getService(
			  $service))) {
				if (TRUE === $rpcService->hasMethod($method)) {
					// Restore server and execution environment information
					// if exists.
					if (array_key_exists("_SERVER", $context)) {
						foreach ($context['_SERVER'] as $key => $value)
							$_SERVER[$key] = $value;
						unset($context['_SERVER']);
					}
					// Execute the RPC service method.
					return $rpcService->callMethod($method, $params, $context);
				} else {
					// Throw an exception if the RPC service method has not
					// been found and redirection to omv-engined is disabled.
					if (~$mode & OMV_RPC_MODE_REMOTE) {
						throw new Exception("The method '%s' does not exist ".
						  "for the RPC service '%s'.", $method, $service);
					}
				}
			} else {
				// Throw an exception if the RPC service has not been found
				// and redirection to omv-engined is disabled.
				if (~$mode & OMV_RPC_MODE_REMOTE) {
					throw new Exception("RPC service '%s' not found.",
					  $service);
				}
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Redirect RPC to omv-engined daemon to execute it remote.
		///////////////////////////////////////////////////////////////////////
		if ($mode & OMV_RPC_MODE_REMOTE) {
			// Store the web server and execution environment information?
			// The information is stored in the given context.
			if (TRUE === $restoreSrvEnv) {
				$variables = [ "SERVER_PROTOCOL", "GATEWAY_INTERFACE",
				  "SERVER_SOFTWARE", "REMOTE_ADDR", "REMOTE_PORT",
				  "SERVER_ADDR", "SERVER_PORT", "SERVER_NAME", "HTTPS",
				  "REDIRECT_STATUS", "HTTP_HOST", "HTTP_ORIGIN",
				  "HTTP_USER_AGENT", "HTTP_CONTENT_TYPE", "HTTP_REFERER" ];
				$context['_SERVER'] = [];
				foreach ($variables as $key => $value) {
					if (!array_key_exists($value, $_SERVER))
						continue;
					$context['_SERVER'][$value] = $_SERVER[$value];
				}
			}
			// Create and connect to the socket.
			if (FALSE === ($socket = @socket_create(AF_UNIX, SOCK_STREAM, 0))) {
				throw new Exception("Failed to create socket: %s",
				  socket_strerror(socket_last_error()));
			}
			// Try to connect to the socket. If the connection fails, then try
			// to establish the connection the given number of attempts.
			$attempt = 0;
			$success = FALSE;
			while (($GLOBALS['OMV_ENGINED_SO_CONNECT_MAX_ATTEMPT'] >
			  $attempt++) && (FALSE === ($success = @socket_connect($socket,
			  $GLOBALS['OMV_ENGINED_SO_ADDRESS'])))) {
				sleep($attempt);
			}
			if (FALSE === $success) {
				throw new Exception("Failed to connect to socket: %s",
				  socket_strerror(socket_last_error()));
			}
			// Set send and receive timeouts.
			socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, [
				"sec" => $GLOBALS['OMV_ENGINED_SO_SNDTIMEO'], "usec" => 0
			]);
			socket_set_option($socket, SOL_SOCKET, SO_RCVTIMEO, [
				"sec" => $GLOBALS['OMV_ENGINED_SO_RCVTIMEO'], "usec" => 0
			]);
			// Create the request.
			$request = json_encode_safe([
				"service" => $service,
				"method" => $method,
				"params" => $params,
				"context" => $context
			]);
			// Build the header.
			$header = pack("NH*", strlen($request), hash("crc32", $request));
			// Build the buffer to be written to the socket.
			$request = $header . $request;
			// Write to socket.
			if (FALSE === @socket_write($socket, $request, strlen($request))) {
				throw new Exception("Failed to write to socket: %s",
				  socket_strerror(socket_last_error()));
			}
			// Read the RPC response header.
			$header = "";
			$headerLen = $GLOBALS['OMV_ENGINED_SO_RPC_HEADER_LENGTH'];
			while (($read = strlen($header)) < $headerLen) {
				$data = @socket_read($socket, $headerLen - $read,
				  PHP_BINARY_READ);
				if (FALSE === $data) {
					throw new Exception("Failed to read from socket: %s",
					  socket_strerror(socket_last_error()));
				}
				$header .= $data;
			}
			// Unpack header.
			$header = unpack("Nlength/H*crc32", $header);
			// Read the RPC response data.
			$response = "";
			while (($read = strlen($response)) < $header['length']) {
				$data = @socket_read($socket, $header['length'] - $read,
				  PHP_BINARY_READ);
				if (FALSE === $data) {
					throw new Exception("Failed to read from socket: %s",
					  socket_strerror(socket_last_error()));
				}
				$response .= $data;
			}
			// Check RPC response data.
			$crc32 = hash("crc32", $response);
			if ($header['crc32'] != $crc32) {
				throw new Exception("Invalid checksum (required=0x%s, ".
				  "calculated=0x%s) of RPC response.", $header['crc32'],
				  $crc32);
			}
			// Close the socket.
			@socket_close($socket);
			// Decode JSON string to PHP array.
			$response = json_decode($response, TRUE);
			// Trigger an exception if the RPC has been failed. Inject the
			// stack trace coming with the remote RPC response.
			if (array_key_exists("error", $response) && !is_null(
			  $response['error'])) {
				$error = $response['error'];
				throw new TraceException($error['message'], $error['code'],
				  $error['trace']);
			}
			return $response['response'];
		}
	}
}
