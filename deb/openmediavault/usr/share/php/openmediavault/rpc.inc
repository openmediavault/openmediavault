<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
require_once("openmediavault/util.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/session.inc");

/**
 * Autoloading of RPC classes.
 * RPC classes must be named like 'XyzRpc'. The correspnding include file
 * is named xyz.inc in this case.
 */
function __autoload($className) {
	if (substr($className, -3, 3) === "Rpc") {
		$classPath = sprintf("%s/rpc/%s.inc",
		  $GLOBALS['OMV_DOCUMENTROOT_DIR'], strtolower(substr(
		  $className, 0, -3)));
		require_once($classPath);
		return;
	}
}

/**
 * @class OMVJsonRpcServer
 */
class OMVJsonRpcServer extends OMVObject {
	protected $params = NULL;
	protected $paramsSchema = '{
		"type":"object",
		"properties":{
			"service":{
				"type":"string"
			},
			"method":{
				"type":"string"
			},
			"params":{
				"type":"any"
			}
		}
	}';

	/**
	 * Handle the RPC request.
	 * @return The RPC response as JSON encoded data.
	 */
	public function handle() {
		// Check and update the last accessed time only if the
		// session is authenticated. Every additional check will be
		// done later if required. This is because there are several
		// RPC's that must be executed without an authenticated
		// session, e.g. the 'Login' RPC.
		$session = &OMVSession::getInstance();
		if ($session->isAuthenticated()) {
			if ($session->isTimeout()) {
				$session->destroy();
				throw new OMVException(OMVErrorMsg::E_SESSION_TIMEOUT);
			}
			$session->updateLastAccess();
		}

		// Get the RPC params
		if (FALSE === $this->getParams()) {
			throw new OMVException(OMVErrorMsg::E_RPC_GET_PARAMS_FAILED);
		}

		// Validate RPC parameters
		$errors = NULL;
		if (FALSE === $this->validate($errors)) {
			throw new OMVException(OMVErrorMsg::E_RPC_INVALID_PARAMS,
			  implode("; ", $errors));
		}

		// Execute RPC
		$this->preExecute();
		$response = OMVRpc::exec($this->params['service'],
		  $this->params['method'], $this->params['params']);
		$this->postExecute($response);

		// Print response
		$this->handleResponse($response);
	}

	/**
	 * Get the RPC parameters.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	protected function getParams() {
		// See http://in.php.net/wrappers.php for more details
		$this->params = json_decode(file_get_contents("php://input"),
          TRUE);
		return !empty($this->params);
	}

	/**
	 * Validate the RPC parameters.
	 * @param errors Contains the error messages if validation fails.
	 * @return TRUE if valid, otherwise FALSE.
	 */
	protected function validate(&$errors) {
		return json_validate(json_encode($this->params), $this->paramsSchema,
		  $errors);
	}

	/**
	 * Do additional stuff before RPC is executed.
	 * @return None.
	 */
	protected function preExecute() {
	}

	/**
	 * Do additional stuff after RPC has been executed.
	 * @param response The RPC response
	 * @return None.
	 */
	protected function postExecute(&$response) {
	}

	/**
	 * Print the RPC responce.
	 * @param response The RPC response
	 * @return None.
	 */
	protected function handleResponse($response) {
		header("Content-Type: application/json");
		print json_encode(array(
			"response" => $response,
			"error" => null
		));
	}
}

/**
 * @class OMVUploadRpcServer
 */
class OMVUploadRpcServer extends OMVJsonRpcServer {
	protected function getParams() {
		$this->params = $_POST;
		if (empty($this->params))
			return FALSE;
		$this->params['params'] = empty($this->params['params']) ?
		  array() : $this->params['params'];
		return TRUE;
	}

	/**
	 * ExtJS wants to see the response in a special format.
	 */
	protected function handleResponse($response) {
		header("Content-Type: text/html");
		print json_encode(array(
			"success" => true, // required by ExtJS
			"responseText" => $response // required by ExtJS
		));
	}
}

/**
 * @class OMVDownloadRpcServer
 */
class OMVDownloadRpcServer extends OMVJsonRpcServer {
	protected function getParams() {
		$this->params = $_POST;
		if (empty($this->params))
			return FALSE;
		// Decode params field
		if (!empty($this->params['params'])) {
			$this->params['params'] = json_decode(
              htmlspecialchars_decode($this->params['params']));
		}
		return TRUE;
	}

	/**
	 * The response contains the following fields:
	 * - filename
	 * - content or filepath
	 * The field \em content contains the content to be returned, \em filepath
	 * contains the name of the file which content should be returned.
	 */
	protected function handleResponse($response) {
		header("Content-Type: text/plain");
		header('Expires: 0');
		header("Content-Disposition: attachment; " .
		  "filename={$response['filename']}");
		if(array_key_exists("content", $response)) {
			$contentLength = strlen($response['content']);
			header("Content-Length: {$contentLength}");
			print($response['content']);
		} else {
			$cmd = sprintf("sudo cat %s", $response['filepath']);
			$proc = proc_open($cmd, array(
				0 => array("pipe", "r"), // stdin
				1 => array("pipe", "w"), // stdout
				2 => array("pipe", "w")  // stderr
			  ), $pipes);
			if(!is_resource($proc)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, "");
			}
			fpassthru($pipes[1]);
			proc_close($proc);
		}
	}
}

/**
 * @class OMVRpc
 */
class OMVRpc extends OMVObject {
	protected $methodSchemata = array();

	/**
	 * Execute the given RPC.
	 * @param name The name of the RPC class.
	 * @param method The name of the method to call.
	 * @param params The parameters to be passed to the method, as
	 * an indexed array.
	 * @return The RPC response.
	 */
	public static function exec($name, $method, $params = array()) {
		// Create new class object
		$className = sprintf("%sRpc", $name);
		$classObject = new $className();
		// Does class exist?
		if (!class_exists($className)) {
			throw new OMVException(
			  OMVErrorMsg::E_RPC_SERVICE_CLASS_NOT_FOUND, $className);
		}
		// Does method exist?
		if (!method_exists($classObject, $method)) {
			throw new OMVException(
			  OMVErrorMsg::E_RPC_SERVICE_METHOD_NOT_FOUND, $method, $name);
		}
		// Now execute the RPC
		if (!is_array($params))
			$params = array($params);
		return call_user_func_array(array($classObject, $method),
          $params);
	}

	/**
	 * Validate the given RPC parameters using JSON schema.
	 * @param method The name of the RPC method. This is used as index to
	 * identify the JSON schema definition to use for validation.
	 * @param param The RPC parameters to validate.
	 * @return None. Throws an exception if validation fails.
	 */
	protected function validateParams($method, $params) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Extract method name.
		if (preg_match("/^\S+::(\S+)$/", $method, $matches))
			$method = $matches[1];
		// Check if there is a schema defined for the given method.
		if (!array_key_exists($method, $this->methodSchemata)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_SCHEMA_NOT_DEFINED,
			  $method);
		}
		// Validate all parameters
		if (!is_array($params)) {
			$params = array($params);
		}
		foreach ($params as $paramk => $paramv) {
			if (FALSE === ($result = json_validate(json_encode($paramv),
			  $this->methodSchemata[$method][$paramk], $errors))) {
				throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PARAM,
				  implode("; ", $errors));
			}
		}
	}

	/**
	 * Validate the session. This must be done individually per method because
	 * there are RPCs available that do not need or must work regardless if
	 * there is no valid session. An exception will be thrown in case of an
	 * error.
	 */
	protected function validateSession() {
		$session = &OMVSession::getInstance();
		if (!$session->isAuthenticated()) {
			throw new OMVException(
			  OMVErrorMsg::E_SESSION_NOT_AUTHENTICATED);
		}
		$session->validate();
	}

	/**
	 * Commit session. Use this function when you do not need to have
	 * access to the session data anymore. The session is blocked until
	 * this function is called which means no other RPC's can be
	 * executed in the meanwhile (this can result in a timeout of the
	 * blocked RPC).
	 */
	protected function commitSession() {
		$session = &OMVSession::getInstance();
		$session->commit();
	}

	/**
	 * Check if the given role is available.
	 * @param role The required role to execute the RPC.
	 * @return TRUE if the given role is available, otherwise FALSE.
	 */
	protected function hasRole($role) {
		$session = &OMVSession::getInstance();
		if ($session->getRole() !== $role) {
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Helper function to filter the method result using the given
	 * filter arguments.
	 * @param array The array of objects to filter.
	 * @param start The index where to start.
	 * @param count The number of elements to process.
	 * @param sortField The name of the column used to sort.
	 * @param sortDir The sort direction, ASC or DESC.
	 * @return An array containing the elements matching the given
	 * restrictions. The field \em total contains the total number of elements,
	 * \em data contains the elements as array. An exception will be thrown
	 * in case of an error.
	 */
	protected function applyFilter($array, $start, $count, $sortField = NULL,
	  $sortDir = NULL) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$total = count($array);
		if ($total > 0) {
			if (!is_null($sortField)) {
				array_sort_key($array, $sortField);
			}
			if (!is_null($sortDir) && $sortDir === "DESC") {
				$array = array_reverse($array);
			}
			if (($start >= 0) && ($count >= 0)) {
				$array = array_slice($array, $start, $count);
			}
		}
		return array(
			"total" => $total,
			"data" => $array
		);
	}
}
?>
