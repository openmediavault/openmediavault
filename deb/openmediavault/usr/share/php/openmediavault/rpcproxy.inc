<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
require_once("openmediavault/rpc.inc");
require_once("openmediavault/rpcservice.inc");

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
class OMVJsonRpcProxy extends OMVObject {
	protected $params = NULL;
	protected $paramsSchema = '{
		"type":"object",
		"properties":{
			"service":{"type":"string"},
			"method":{"type":"string"},
			"params":{"type":"any"},
			"options":{"type":"object","optional":true}
		}
	}';

	/**
	 * Handle the RPC request.
	 * @return The RPC response as JSON encoded data.
	 * @throw E_SESSION_TIMEOUT
	 * @throw E_RPC_GET_PARAMS_FAILED
	 * @throw E_RPC_INVALID_PARAMS
	 */
	public function handle() {
		// Get the RPC params.
		if(FALSE === $this->getParams()) {
			throw new OMVException(OMVErrorMsg::E_RPC_GET_PARAMS_FAILED);
		}

		// Validate RPC parameters.
		$errors = NULL;
		if(FALSE === $this->validate($errors)) {
			throw new OMVException(OMVErrorMsg::E_RPC_INVALID_PARAMS,
			  implode("; ", $errors));
		}

		// Process optional RPC options.
		$updateLastAccess = TRUE;
		if(array_key_exists("options", $this->params) && !is_null(
		  $this->params['options']) && array_key_exists("updatelastaccess",
		  $this->params['options'])) {
			$updateLastAccess = boolval(
			  $this->params['options']['updatelastaccess']);
		}

		// Check and update the last accessed time only if the
		// session is authenticated. Every additional check will be
		// done later if required. This is because there are several
		// RPCs that must be executed without an authenticated
		// session, e.g. the 'Login' RPC.
		$session = &OMVSession::getInstance();
		if($session->isAuthenticated()) {
			if($session->isTimeout()) {
				$session->destroy();
				throw new OMVException(OMVErrorMsg::E_SESSION_TIMEOUT);
			}
			if(TRUE === $updateLastAccess) {
				$session->updateLastAccess();
			}
		}

		// Check if it is a local or remote RPC. If it is a RPC that is
		// redirected from the WebGUI backend to the omv-engined daemon,
		// then we can commit the session (these RPCs are executed in
		// another process thus they can not access this session).
		$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
		if(FALSE === ($rpcService = $rpcServiceMgr->getService(
		  $this->params['service']))) {
			// Session MUST be authenticated at this point.
			if(!$session->isAuthenticated()) {
				throw new OMVException(
				  OMVErrorMsg::E_SESSION_NOT_AUTHENTICATED);
			}
			// If service is not available locally, then we can commit
			// the session due the fact that the RPC is redirected to
			// the omv-engined daemon which does not have access to this
			// session.
			$session->commit();
		}

		// Execute the RPC.
		$this->preExecute();
		$context = array(
			"username" => $session->getUsername(),
			"role" => $session->getRole()
		);
		$response = OMVRpc::exec($this->params['service'],
		  $this->params['method'], $this->params['params'], $context,
		  OMV_RPC_MODE_LOCAL | OMV_RPC_MODE_REMOTE);
		$this->postExecute($response);

		// Print response.
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
		return json_validate(json_encode_safe($this->params),
		  $this->paramsSchema, $errors);
	}

	/**
	 * Do additional stuff before RPC is executed.
	 * @return None.
	 */
	protected function preExecute() {
		// Nothing to do here.
	}

	/**
	 * Do additional stuff after RPC has been executed.
	 * @param response The RPC response
	 * @return None.
	 */
	protected function postExecute(&$response) {
		// Nothing to do here.
	}

	/**
	 * Print the RPC responce.
	 * @param response The RPC response
	 * @return None.
	 */
	protected function handleResponse($response) {
		header("Content-Type: application/json");
		print json_encode_safe(array(
			"response" => $response,
			"error" => null
		));
	}

	/**
	 * The function that is called after the RPC has been successfully
	 * executed or it has been failed.
	 */
	public function cleanup() {
		// Nothing to do here.
	}
}

/**
 * A specialized RPC proxy that handles file uploads.
 * @ingroup api
 */
class OMVUploadRpcProxy extends OMVJsonRpcProxy {
	protected function getParams() {
		$this->params = $_POST;
		if(empty($this->params))
			return FALSE;
		$this->params['params'] = empty($this->params['params']) ?
		  array() : json_decode(htmlspecialchars_decode(
		  $this->params['params']), TRUE);
		return TRUE;
	}

	/**
	 * Copy the uploaded file to a temporary location. Modify this
	 * filename to allow the RPC service method to process the correct
	 * file.
	 * @throw E_MISC_FAILURE
	 */
	protected function preExecute() {
		// Is file successfully upload?
		if(!is_uploaded_file($_FILES['file']['tmp_name'])) {
			throw new OMVException( OMVErrorMsg::E_MISC_FAILURE,
			  "Invalid upload request.");
		}
		// Create the temporary filename.
		$tmpFilename = tempnam(sys_get_temp_dir(), "upload");
		// Copy file to a temporary location.
		if(!move_uploaded_file($_FILES['file']['tmp_name'], $tmpFilename)) {
			throw new OMVException( OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  "Failed to move uploaded file (src=%s, dst=%s, size=%d).",
			  $_FILES['file']['tmp_name'], $tmpFilename,
			  $_FILES['file']['size']));
		}
		// Modify the service method parameters.
		$this->params['params']['filename'] = $_FILES['file']['name'];
		$this->params['params']['filepath'] = $tmpFilename;
	}

	/**
	 * ExtJS wants to see the response in a special format.
	 */
	protected function handleResponse($response) {
		header("Content-Type: text/html");
		print json_encode_safe(array(
			"success" => true, // required by ExtJS
			"responseText" => $response // required by ExtJS
		));
	}

	/**
	 * The function that is called after the RPC has been successfully
	 * executed or it has been failed.
	 */
	public function cleanup() {
		// Ensure the uploaded file is removed.
		if(array_key_exists("filepath", $this->params['params'])) {
			if(file_exists($this->params['params']['filepath']))
				unlink($this->params['params']['filepath']);
		}
	}
}

/**
 * A specialized RPC proxy that handles file downloads.
 * @ingroup api
 */
class OMVDownloadRpcProxy extends OMVJsonRpcProxy {
	protected function getParams() {
		$this->params = $_POST;
		if(empty($this->params))
			return FALSE;
		// Decode params field
		if(!empty($this->params['params'])) {
			$this->params['params'] = json_decode(htmlspecialchars_decode(
			  $this->params['params']), TRUE);
		}
		return TRUE;
	}

	/**
	 * @param response The response contains the following fields
	 *   \em filename, \em content or \em filepath and \em contenttype which
	 *   is optional. The field \em content contains the content to be
	 *   returned, \em filepath contains the name of the file which content
	 *   should be returned. Please note that the file must be readable for
	 *   everybody. If \em unlink is set then the given file will be
	 *   removed after use. The fields \em content and \em filepath exclude
	 *   each other. Only The field \em contenttype contains the
	 *   'Content-Type' header string if set, otherwise it is set to
	 *   'text/plain'.
	 */
	protected function handleResponse($response) {
		header("Content-Type: ". (array_key_exists("contenttype", $response) ?
		  $response['contenttype'] : "text/plain"));
		header('Expires: 0');
		header("Content-Disposition: attachment; " .
		  "filename={$response['filename']}");
		if(array_key_exists("content", $response)) {
			$contentLength = strlen($response['content']);
			header("Content-Length: {$contentLength}");
			print($response['content']);
		} else {
			@readfile($response['filepath']);
			if(array_key_exists("unlink", $response) &&
			  (TRUE === $response['unlink'])) {
				@unlink($response['filepath']);
			}
		}
	}
}

// Include all RPC service classes.
$rpcServiceDir = sprintf("%s/rpc", $GLOBALS['OMV_DOCUMENTROOT_DIR']);
foreach(new DirectoryIterator($rpcServiceDir) as $item) {
	if($item->isDot())
		continue;
	if($item->isFile()) {
		$pathInfo = pathinfo($item->getFilename());
		if(strtolower($pathInfo['extension']) == "inc") {
			require_once("{$rpcServiceDir}/{$item->getFilename()}");
		}
	}
}
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->initializeServices();
?>
