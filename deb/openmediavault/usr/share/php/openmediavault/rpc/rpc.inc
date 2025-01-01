<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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
	use \OMV\DebugTrait;

	const MODE_LOCAL = 0x1;
	const MODE_REMOTE = 0x2;

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
	 * @param string $service The name of the service.
	 * @param string $method The name of the method.
	 * @param array $params The parameters to be passed to the method of
	 *   the given service.
	 * @param array $context The context of the caller containing the keys
	 *   \em username and \em role.
	 * @param int $mode The mode how to execute this RPC. The following
	 *   modes are available:<ul>
	 *   \li MODE_LOCAL
	 *   \li MODE_REMOTE
	 *   </ul>
	 *   Defaults to MODE_LOCAL.
	 * @param bool $restoreSrvEnv Restore various web server and execution
	 *   environment information. This might be helpful in some cases if
	 *   these information are required in the engine backend. Note, this
	 *   only takes action when mode is MODE_REMOTE. Defaults to FALSE.
	 * @return The RPC response.
	 */
	public static function call($service, $method, $params, $context,
	  $mode = Rpc::MODE_LOCAL, $restoreSrvEnv = FALSE) {
//		self::debug(var_export(func_get_args(), TRUE));
		///////////////////////////////////////////////////////////////////////
		// Try to execute local RPC first.
		///////////////////////////////////////////////////////////////////////
		if ($mode & Rpc::MODE_LOCAL) {
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
					if (~$mode & Rpc::MODE_REMOTE) {
						throw new \OMV\HttpErrorException(404,
							"The method '%s' does not exist for the RPC ".
							"service '%s'.", $method, $service);
					}
				}
			} else {
				// Throw an exception if the RPC service has not been found
				// and redirection to omv-engined is disabled.
				if (~$mode & Rpc::MODE_REMOTE) {
					throw new \OMV\HttpErrorException(404,
						"RPC service '%s' not found.", $service);
				}
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Redirect RPC to omv-engined daemon to execute it remote.
		///////////////////////////////////////////////////////////////////////
		if ($mode & Rpc::MODE_REMOTE) {
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
			while ((\OMV\Environment::getInteger(
			  "OMV_ENGINED_SO_CONNECT_MAX_ATTEMPT") > $attempt++) &&
			  (FALSE === ($success = @socket_connect($socket,
			  \OMV\Environment::get("OMV_ENGINED_SO_ADDRESS"))))) {
				sleep($attempt);
			}
			if (FALSE === $success) {
				throw new Exception("Failed to connect to socket: %s",
				  socket_strerror(socket_last_error()));
			}
			// Set send and receive timeouts.
			socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, [
				"sec" => \OMV\Environment::get("OMV_ENGINED_SO_SNDTIMEO"),
				"usec" => 0
			]);
			socket_set_option($socket, SOL_SOCKET, SO_RCVTIMEO, [
				"sec" => \OMV\Environment::get("OMV_ENGINED_SO_RCVTIMEO"),
				"usec" => 0
			]);
			// Create the request.
			$request = json_encode_safe([
				"service" => $service,
				"method" => $method,
				"params" => $params,
				"context" => $context
			]);
			$request .= "\0"; // Append the EOF byte.
			// Write the RPC request.
			if (FALSE === @socket_write($socket, $request, strlen($request))) {
				throw new Exception("Failed to write to socket: %s",
				  socket_strerror(socket_last_error()));
			}
			// Read the RPC response.
			$response = "";
			while (TRUE) {
				$data = @socket_read($socket, 4096, PHP_BINARY_READ);
				// Check for errors.
				if (FALSE === $data) {
					throw new Exception("Failed to read from socket: %s",
					  socket_strerror(socket_last_error()));
				}
				$response .= $data;
				// Abort if response is complete.
				if (empty($data) || "\0" == substr($response, -1))
					break;
			}
			$response = substr($response, 0, -1); // Remove the EOF byte.
			// Close the socket.
			@socket_close($socket);
			// Check if the RPC response is valid.
			if (!is_json($response)) {
				// If we are here, then in most cases the omv-engined
				// process has been stopped because of a PHP fatal error.
				throw new Exception("Invalid RPC response. Please check the ".
					"syslog for more information.");
			}
			// Decode JSON string to PHP array.
			if (NULL === ($response = json_decode($response, TRUE))) {
				throw new Exception("Failed to decode JSON string: %s",
					json_last_error_msg());
			}
			// Trigger an exception if the RPC has been failed. Inject the
			// stack trace coming with the remote RPC response.
			if (array_key_exists("error", $response) && !is_null(
			  $response['error'])) {
				$error = $response['error'];
				$e = new TraceException($error['message'], $error['code'],
					$error['trace']);
				if (is_int($error['http_status_code'])) {
					$e->setHttpStatusCode($error['http_status_code']);
				}
				throw $e;
			}
			return $response['response'];
		}
	}

	/**
	 * Execute the given RPC as background process.
	 * @param string $service The name of the service.
	 * @param string $method The name of the method.
	 * @param mixed $params The parameters hash object to be passed to
	 *   the method of the given service.
	 * @param object $context The context hash object of the caller containing
	 *   the fields \em username and \em role.
	 * @return object The RPC response.
	 */
	public static function callBg($service, $method, $params, $context) {
		$rpcServiceMngr = &\OMV\Rpc\ServiceManager::getInstance();
		if (FALSE !== ($rpcService = $rpcServiceMngr->getService($service))) {
			if (TRUE === $rpcService->hasMethod($method)) {
				// Restore server and execution environment information
				// if exists.
				if (array_key_exists("_SERVER", $context)) {
					foreach ($context['_SERVER'] as $key => $value) {
						$_SERVER[$key] = $value;
					}
					unset($context['_SERVER']);
				}
				// Execute the RPC service method as a background process.
				return $rpcService->callMethodBg($method, $params, $context);
			} else {
				throw new \OMV\HttpErrorException(404,
					"The method '%s' does not exist for the RPC service '%s'.",
					$method, $service);
			}
		} else {
			throw new \OMV\HttpErrorException(404,
				"RPC service '%s' not found.", $service);
		}
	}
}
