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
require_once("openmediavault/error.inc");
require_once("openmediavault/rpcservice.inc");

class OMVRpcServiceExec extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Exec";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("execBg");
		$this->registerMethod("stop");
		$this->registerMethod("getOutput");
		$this->registerMethod("isRunning");
	}

	/**
	 * Execute the given RPC as a background process.
	 * @param params An array containing the following fields:
	 *   \em service The name of the service.
	 *   \em method The name of the method.
	 *   \em params The parameters hash object to be passed to the method
	 *   of the given service.
	 * @param context The context of the caller.
	 * @return The name of the background process status file if
	 *   \em background is set to TRUE.
	 */
	public function execBg($params, $context) {
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "service":{"type":"string"},
				  "method":{"type":"string"},
				  "params":{"type":"any"}
			  }
		  }');
		// Create a background process.
		$bgStatusFilename = $this->createBgProcStatus();
		$pid = $this->fork();
		if ($pid > 0) { // Parent process.
			$this->initializeBgProcStatus($bgStatusFilename, $pid);
			return $bgStatusFilename;
		}
		// Child process.
		try {
			// Execute the given RPC.
			OMVRpc::exec($params['service'], $params['method'],
			  $params['params'], $context);
			// Update the background process status file.
			$this->finalizeBgProcStatus($bgStatusFilename);
			exit(0);
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}

	/**
	 * Stop the given background process.
	 * @param params An array containing the following fields:
	 *   \em filename The name of the background process status file.
	 * @param context The context of the caller.
	 * @return None
	 */
	public function stop($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "filename":{"type":"string"}
			  }
		  }');
		// Get the background process status.
		$status = new OMVRpcBgStatus($this->getBgProcStatus(
		  $params['filename']));
		// Kill the background process.
		if(!posix_kill($status->getPid(), SIGTERM)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to kill process (pid=%d).", $status->getPid()));
		}
		// Unlink the background process status file.
		$this->unlinkBgProcStatus($params['filename']);
	}

	/**
	 * Get the output of the given background process.
	 * @param params An array containing the following fields:
	 *   \em filename The name of the background process status file.
	 *   \em pos The file position withing the command output file.
	 * @return An array with the fields \em filename containing the background
	 *   process status file, \em running which indicates the command execution
	 *   state, \em output which contains the command output and \em pos which
	 *   is the current position within the output file (more detailed it is
	 *   the size of the output file or EOF position) will be returned.
	 */
	public function getOutput($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "filename":{"type":"string"},
				  "pos":{"type":"integer"}
			  }
		  }');
		// Get the background process status file.
		$status = new OMVRpcBgStatus($this->getBgProcStatus(
		  $params['filename']));
		// Check if there was an error and trigger an exception if
		// necessary. Inject the stack trace coming from the background
		// process.
		if($status->hasError()) {
			$error = $status->getError();
			throw new OMVException2($error['code'], $error['message'],
			  $error['trace']);
		}
		// Prepare the result object.
		$result = array(
			"filename" => $params['filename'],
			"pos" => 0,
			"output" => "",
			"running" => $status->isRunning()
		);
		// Check if there is a background process output file and try to
		// read the content starting from the given position to the end
		// of the file.
		if($status->hasOutputFilename()) {
			$bgOutputFilename = $status->getOutputFilename();
			// Read the command output from the given file.
			if(FALSE === ($fp = fopen($bgOutputFilename, "r"))) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("Failed to open the command output file ".
					"(filename=%s).", $bgOutputFilename));
			}
			// Get the requested command output from the given position to
			// the end of the file.
			fseek($fp, $params['pos'], SEEK_SET);
			while(!feof($fp)) {
				$result['output'] .= fread($fp, 1024);
			}
			// Get the current position within the command output file.
			$result['pos'] = ftell($fp);
			fclose($fp);
		}
		// Cleanup if process is not running anymore.
		if(FALSE === $status->isRunning()) {
			$this->unlinkBgProcStatus($params['filename']);
		}
		return $result;
	}

	/**
	 * Check if the given background process is still running.
	 * @param params An array containing the following fields:
	 *   \em filename The name of the background process status file.
	 * @param context The context of the caller.
	 * @return An array with the fields \em filename containing the background
	 *   process status file and \em running which indicates the command
	 *   execution state.
	 */
	public function isRunning($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "filename":{"type":"string"}
			  }
		  }');
		// Get the background process status.
		$status = new OMVRpcBgStatus($this->getBgProcStatus(
		  $params['filename']));
		// Check if there was an error and trigger an exception if
		// necessary. Inject the stack trace coming from the background
		// process.
		if($status->hasError()) {
			$error = $status->getError();
			throw new OMVException2($error['code'], $error['message'],
			  $error['trace']);
		}
		// Cleanup if process is not running anymore.
		if(FALSE === $status->isRunning()) {
			$this->unlinkBgProcStatus($params['filename']);
		}
		// Return the running state.
		return array(
			"filename" => $params['filename'],
			"running" => $status->isRunning()
		);
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceExec());
?>
