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
require_once("openmediavault/globals.inc");
require_once("openmediavault/object.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/log.inc");
require_once("openmediavault/rpcservice.inc");

class OMVRpcServiceLogFile extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "LogFile";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getList");
		$this->registerMethod("clear");
		$this->registerMethod("getContent");
	}

	/**
	 * Helper method to parse a logfile.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 *   \em id The identifier of the registered log file type.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 * @throw E_MISC_FAILURE
	 */
	function getList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "start":{"type":"integer"},
				  "limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
				  "sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
				  "sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'},
				  "id":{"type":"string"}
			  }
		  }');
		// Check if the given log file type if registered.
		$logfile = new OMVLogFile($params['id']);
		if(FALSE === $logfile->isRegistered()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  gettext("Unknown log file type (id=%s)"), $params['id']));
		}
		// Read file content.
		if(FALSE === ($fh = fopen($logfile->getFilePath(), "r"))) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to open the log file (filename=%s).",
			  $logfile->getFilePath()));
		}
		$objects = array();
		$rowNum = 0;
		while(($line = fgets($fh)) !== FALSE) {
			if($rowNum > $GLOBALS['OMV_MAX_LOGFILE_LINES'])
				break;
			// Skip invalid pattern matches.
			$result = preg_match($logfile->getRegex(), $line, $matches);
			if((FALSE === $result) || (0 == $result))
				continue;
			$object = array(
				"rownum" => $rowNum++
			);
			foreach($logfile->getColumns() as $columnk => $columnv) { 
				// Execute user defined function, e.g. to convert
				// column content?
				if(is_array($columnv)) {
					$object[$columnk] = $columnv['func'](
					  $matches[$columnv['index']]);
				} else {
					$object[$columnk] = $matches[$columnv];
				}
			}
			$objects[] = $object;
		}
		if(FALSE === fclose($fh)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to close the log file (filename=%s).",
			  $logfile->getFilePath()));
		}
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

 	/**
	 * Clear a log file.
	 * @param params An array containing the following fields:
	 *   \em id The identifier of the registered log file type.
	 * @param context The context of the caller.
	 * @return None
	 * @throw E_MISC_FAILURE
	 */
	function clear($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "id":{"type":"string"}
			  }
		  }');
		// Check if the given log file type if registered.
		$logfile = new OMVLogFile($params['id']);
		if(FALSE === $logfile->isRegistered()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  gettext("Unknown log file type (id=%s)"), $params['id']));
		}
		// Clear log file.
		if(FALSE === ($fh = fopen($logfile->getFilePath(), "w"))) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to open the log file (filename=%s).",
			  $logfile->getFilePath()));
		}
		if(FALSE === ftruncate($fh, 0)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to truncate the log file (filename=%s).",
			  $logfile->getFilePath()));
		}
		rewind($fh);
		if(FALSE === fclose($fh)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to close the log file (filename=%s).",
			  $logfile->getFilePath()));
		}
	}

 	/**
	 * Get the content of a log file. Note, the RPC does not return the
	 * file content itself, instead the file path is returned which is
	 * used by generic download RPC mechanism to deliver the file content.
	 * @param params An array containing the following fields:
	 *   \em id The identifier of the registered log file type.
	 * @param context The context of the caller.
	 * @return An array with the keys \em filename and \em filepath.
	 * @throw E_MISC_FAILURE
	 */
	function getContent($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "id":{"type":"string"}
			  }
		  }');
		// Check if the given log file type if registered.
		$logfile = new OMVLogFile($params['id']);
		if(FALSE === $logfile->isRegistered()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  gettext("Unknown log file type (id=%s)"), $params['id']));
		}
		// Copy the log file to a temporary location and modify the file
		// mode/owner to allow the WebGUI PHP backend to unlink it.
		$tmpFilePath = tempnam(sys_get_temp_dir(), "logfile");
		if(FALSE === copy($logfile->getFilePath(), $tmpFilePath)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to copy the log file (source=%s, dest=%s).",
			  $logfile->getFilePath(), $tmpFilePath));
		}
		chmod($tmpFilePath, 0700);
		chown($tmpFilePath, $GLOBALS['OMV_APACHE2_SUEXECUSERGROUP']);
		// Return values required by generic download RPC implementation.
		return array(
			"filename" => $logfile->getFileName(),
			"filepath" => $tmpFilePath,
			"unlink" => TRUE
		);
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceLogFile());
?>