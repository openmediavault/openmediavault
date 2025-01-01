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
namespace OMV\Rpc\Proxy;

require_once("openmediavault/functions.inc");

/**
 * The generic RPC proxy implementation that handles JSON RPC requests.
 * A RPC request must contain the fields:
 *   \em service The name of the service.
 *   \em method The name of the method.
 *   \em params The method parameters as JSON string.
 *   \em options An optional array of RPC options with the fields:
 *     \em updatelastaccess Set to FALSE to do not update the time on which
 *     the last access took place.
 * @ingroup api
 */
class Json {
	protected $params = NULL;

	/**
	 * Handle the RPC request.
	 * @return The RPC response as JSON encoded data.
	 */
	public function handle() {
		// Get the RPC parameters `service`, `method`, `params` and `options`.
		if (FALSE === $this->getParams()) {
			throw new Exception("Failed to get the RPC parameters.");
		}
		// Validate RPC parameters.
		$this->validate();
		// Handle the optional `options` parameter.
		$updateLastAccess = TRUE;
		if (array_key_exists("options", $this->params) && !is_null(
		  $this->params['options']) && array_key_exists("updatelastaccess",
		  $this->params['options'])) {
			$updateLastAccess = boolvalEx(
			  $this->params['options']['updatelastaccess']);
		}

		// Check and update the last accessed time only if the
		// session is authenticated. Every additional check will be
		// done later if required. This is because there are several
		// RPCs that must be executed without an authenticated
		// session, e.g. the 'Login' RPC.
		$session = &\OMV\Session::getInstance();
		if ($session->isAuthenticated()) {
			$session->validateTimeout();
			if (TRUE === $updateLastAccess)
				$session->updateLastAccess();
		}

		// Check if it is a local or remote RPC. If it is a RPC that is
		// redirected from the WebGUI backend to the omv-engined daemon,
		// then we can commit the session (these RPCs are executed in
		// another process thus they can not access this session).
		$rpcServiceMngr = &\OMV\Rpc\ServiceManager::getInstance();
		if (FALSE === ($rpcService = $rpcServiceMngr->getService(
		  $this->params['service']))) {
			// Session MUST be authenticated at this point.
			$session->validateAuthentication();
			// Ensure user exists.
			$session->validateUser();
			// If service is not available locally, then we can commit
			// the session due the fact that the RPC is redirected to
			// the omv-engined daemon which does not have access to this
			// session.
			$session->commit();
		}

		// Execute the RPC.
		$this->preExecute();
		$context = \OMV\Rpc\Rpc::createContext($session->getUsername(),
		  $session->getRole());
		$response = \OMV\Rpc\Rpc::call($this->params['service'],
		  $this->params['method'], $this->params['params'], $context,
		  \OMV\Rpc\Rpc::MODE_LOCAL | \OMV\Rpc\Rpc::MODE_REMOTE);
		$this->postExecute($response);

		// Print response.
		$this->handleResponse($response);
	}

	/**
	 * Get the RPC parameters from POST or GET.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	protected function getParams() {
		if (!empty($_GET)) {
			$this->params = [];
		 	foreach ($_GET as $key => $value) {
		 		$this->params[$key] = $value;
		 	}
		} else {
			$this->params = @file_get_contents("php://input");
			if (FALSE === $this->params) {
				return FALSE;
			}
			$this->params = json_decode($this->params, TRUE);
		}
		if (!is_assoc_array($_POST)) {
			return FALSE;
		}
		// Make sure the `params` property exists and is properly
		// decoded if it is a JSON string.
		$paramsProp = array_value($this->params, "params");
		if (!empty($paramsProp)) {
			if (is_string($paramsProp)) {
				$paramsProp = htmlspecialchars_decode($paramsProp);
			}
			if (is_json($paramsProp)) {
				$paramsProp = json_decode($paramsProp, TRUE);
			}
			$this->params['params'] = $paramsProp;
		} else {
			$this->params['params'] = [];
		}
	}

	/**
	 * Validate the RPC parameters. An exception is thrown when
	 * the validation fails.
	 * @return void
	 */
	protected function validate() {
		$schema = new \OMV\Json\Schema('{
			"type":"object",
			"properties":{
				"service":{"type":"string","required":true},
				"method":{"type":"string","required":true},
				"params":{"type":"any","required":true},
				"options":{"type":"any","required":false}
			}
		}');
		$schema->validate(json_encode_safe($this->params));
	}

	/**
	 * Do additional stuff before RPC is executed.
	 * @return void
	 */
	protected function preExecute() {
		// Nothing to do here.
	}

	/**
	 * Do additional stuff after RPC has been executed.
	 * @param response The RPC response.
	 * @return void
	 */
	protected function postExecute(&$response) {
		// Nothing to do here.
	}

	/**
	 * Print the RPC response.
	 * @param response The RPC response.
	 * @return void
	 */
	protected function handleResponse($response) {
		header("Content-Type: application/json");
		header("Cache-Control: max-age=0, no-cache, no-store, must-revalidate");
		header("Pragma: no-cache");
		header("Expires: 0");
		print json_encode_safe([
			"response" => $response,
			"error" => null
		]);
	}

	/**
	 * The function that is called after the RPC has been successfully
	 * executed or it has been failed.
	 */
	public function cleanup() {
		// Nothing to do here.
	}
}
