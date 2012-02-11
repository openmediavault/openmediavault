<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
require_once("openmediavault/rpc.inc");
require_once("openmediavault/log.inc");

class LogFileRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getList" => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}', // sortDir
				'{"type":"string"}' // type
			),
			"clear" => array(
				'{"type":"string"}'
			),
			"getContent" => array(
				'{"type":"string"}'
			)
		);
	}

	/**
	 * Helper method to parse a logfile.
	 */
	function getList($start, $count, $sortField, $sortDir, $id) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Check if the given log file type if registered.
		$logfile = new OMVLogFile($id);
		if(FALSE === $logfile->isRegistered()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  gettext("Unknown log file type (id=%s)"), $id));
		}
		// Read file content.
		$cmd = sprintf("sudo tail -n %s %s", $GLOBALS['OMV_MAX_LOGFILE_LINES'],
		  $logfile->getFilePath());
		$proc = proc_open($cmd, array(
			0 => array("pipe", "r"), // stdin
			1 => array("pipe", "w"), // stdout
			2 => array("pipe", "w")  // stderr
		  ), $pipes);
		if(!is_resource($proc)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, "");
		}
		$objects = array();
		while(($line = fgets($pipes[1])) !== false) {
			// Skip invalid pattern matches.
			$result = preg_match($logfile->getRegex(), $line, $matches);
			if((FALSE === $result) || (0 == $result))
				continue;
			$object = array();
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
		proc_close($proc);
		// Filter result.
		return $this->applyFilter($objects, $start, $count, $sortField,
		  $sortDir);
	}

 	/**
	 * Clear a log file.
	 * @param id The identifier of the registered log file type.
	 */
	function clear($id) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Check if the given log file type if registered.
		$logfile = new OMVLogFile($id);
		if(FALSE === $logfile->isRegistered()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  gettext("Unknown log file type (id=%s)"), $id));
		}
		// Empty log file
		$cmd = sprintf("cat /dev/null | sudo tee %s", $logfile->getFilePath());
		OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

 	/**
	 * Get the content of a log file. Note, the RPC does not return the file
	 * content itself, instead the file path is returned which is used by
	 * generic download RPC mechanism to deliver the file content.
	 * @param id The identifier of the registered log file type.
	 * @return An object with the keys \em filename and \em filepath.
	 */
	function getContent($id) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Check if the given log file type if registered.
		$logfile = new OMVLogFile($id);
		if(FALSE === $logfile->isRegistered()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  gettext("Unknown log file type (id=%s)"), $id));
		}
		// Return values required by generic download RPC implementation.
		return array(
			"filename" => $logfile->getFileName(),
			"filepath" => $logfile->getFilePath()
		);
	}
}
?>