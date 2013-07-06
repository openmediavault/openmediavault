<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
require_once("openmediavault/object.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/rpcservice.inc");

/**
 * This class provides methods regarding remote procedure calls.
 * @ingroup api
 */
class OMVRpc extends OMVObject {
	/**
	 * Execute the given RPC.
	 * @param service The name of the service.
	 * @param method The name of the method.
	 * @param params The parameters hash object to be passed to the method
	 *   of the given service.
	 * @param context The context hash object of the caller containing the
	 *   fields \em username and \em role.
	 * @param mode The mode how to execute this RPC. Defaults to local only.
	 * @return The RPC response.
	 * @throw E_RPC_SERVICE_NOT_FOUND
	 * @throw E_RPC_SERVICE_METHOD_NOT_EXISTS
	 * @throw E_MISC_FAILURE
	 * @throw E_ENGINED_CREATE_SOCKET
	 * @throw E_ENGINED_CONNECT_SOCKET
	 * @throw E_ENGINED_WRITE_SOCKET
	 * @throw E_ENGINED_READ_SOCKET
	 */
	public static function exec($service, $method, $params, $context,
	  $mode = OMV_RPC_MODE_LOCAL) {
//		$this->debug(var_export(func_get_args(), TRUE));
		///////////////////////////////////////////////////////////////////////
		// Try to execute local RPC first.
		///////////////////////////////////////////////////////////////////////
		if($mode & OMV_RPC_MODE_LOCAL) {
			$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
			if(FALSE !== ($rpcService = $rpcServiceMgr->getService(
			  $service))) {
				if(TRUE === $rpcService->hasMethod($method)) {
					// Execute the RPC service method.
					return $rpcService->callMethod($method, $params, $context);
				} else {
					// Throw an exception if the RPC service method has not
					// been found and redirection to omv-engined is disabled.
					if(~$mode & OMV_RPC_MODE_REMOTE) {
						throw new OMVException(
						  OMVErrorMsg::E_RPC_SERVICE_METHOD_NOT_EXISTS,
						  $method, $service);
					}
				}
			} else {
				// Throw an exception if the RPC service has not been found
				// and redirection to omv-engined is disabled.
				if(~$mode & OMV_RPC_MODE_REMOTE) {
					throw new OMVException(
					  OMVErrorMsg::E_RPC_SERVICE_NOT_FOUND,
					  $service);
				}
			}
		}

		///////////////////////////////////////////////////////////////////////
		// Redirect RPC to omv-engined daemon to execute it remote.
		///////////////////////////////////////////////////////////////////////
		if($mode & OMV_RPC_MODE_REMOTE) {
			// Create and connect to the socket.
			if(FALSE === ($socket = @socket_create(AF_UNIX, SOCK_STREAM, 0))) {
				throw new OMVException(
				  OMVErrorMsg::E_ENGINED_CREATE_SOCKET,
				  socket_strerror(socket_last_error()));
			}
			if(FALSE === @socket_connect($socket,
			  $GLOBALS['OMV_ENGINED_SO_ADDRESS'])) {
				throw new OMVException(
				  OMVErrorMsg::E_ENGINED_CONNECT_SOCKET,
				  socket_strerror(socket_last_error()));
			}
			// Set send and receive timeouts.
			socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, array(
			  "sec" => $GLOBALS['OMV_ENGINED_SO_SNDTIMEO'], "usec" => 0));
			socket_set_option($socket, SOL_SOCKET, SO_RCVTIMEO, array(
			  "sec" => $GLOBALS['OMV_ENGINED_SO_RCVTIMEO'], "usec" => 0));
			// Create the request.
			$request = json_encode_safe(array(
				"service" => $service,
				"method" => $method,
				"params" => $params,
				"context" => $context
			));
			// Build the header.
			$header = pack("NH*", strlen($request), hash("crc32", $request));
			// Build the buffer to be written to the socket.
			$request = $header . $request;
			// Write to socket.
			if(FALSE === @socket_write($socket, $request, strlen($request))) {
				throw new OMVException(
				  OMVErrorMsg::E_ENGINED_WRITE_SOCKET,
				  socket_strerror(socket_last_error()));
			}
			// Read the RPC response header.
			$header = "";
			$headerLen = $GLOBALS['OMV_ENGINED_SO_RPC_HEADER_LENGTH'];
			while(($read = strlen($header)) < $headerLen) {
				$data = @socket_read($socket, $headerLen - $read,
				  PHP_BINARY_READ);
				if(FALSE === $data) {
					throw new OMVException(
					  OMVErrorMsg::E_ENGINED_READ_SOCKET,
					  socket_strerror(socket_last_error()));
				}
				$header .= $data;
			}
			// Unpack header.
			$header = unpack("Nlength/H*crc32", $header);
			// Read the RPC response data.
			$response = "";
			while(($read = strlen($response)) < $header['length']) {
				$data = @socket_read($socket, $header['length'] - $read,
				  PHP_BINARY_READ);
				if(FALSE === $data) {
					throw new OMVException(
					  OMVErrorMsg::E_ENGINED_READ_SOCKET,
					  socket_strerror(socket_last_error()));
				}
				$response .= $data;
			}
			// Check RPC response data.
			$crc32 = hash("crc32", $response);
			if($header['crc32'] != $crc32) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("Invalid checksum (required=0x%s, calculated=0x%s) ".
				  "of RPC response.", $header['crc32'], $crc32));
			}
			// Close the socket.
			@socket_close($socket);
			// Decode JSON string to PHP array.
			$response = json_decode($response, TRUE);
			// Trigger an exception if the RPC has been failed. Inject the
			// stack trace coming with the remote RPC response.
			if(array_key_exists("error", $response) && !is_null(
			  $response['error'])) {
				$error = $response['error'];
				throw new OMVException2($error['code'], $error['message'],
				  $error['trace']);
			}
			return $response['response'];
		}
	}
}
?>
