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
require_once("openmediavault/globals.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/initscript.inc");

class ConfigRpc extends OMVRpc {
	/**
	 * Download the configuration.
	 * @return Array as it is required by the OMVDownloadRpcServer.
	 */
	function backup() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration file content.
		return array(
			"filename" => sprintf("config_%s_%s.xml",
			  OMVNetwork::getHostname("--fqdn"), date("YmdHis", time())),
			"filepath" => $GLOBALS['OMV_CONFIG_FILE']
		);
	}

	/**
	 * Restore configuration.
	 * @return The process id of the config import process.
	 */
	function restore() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Is file successfully upload?
		if (!is_uploaded_file($_FILES['file']['tmp_name'])) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  "Invalid configuration restore request");
		}
		// Make a backup of the current configuration
		copy($GLOBALS['OMV_CONFIG_FILE'], "{$GLOBALS['OMV_CONFIG_FILE']}.old");
		// Replace current configuration with the uploaded file
		move_uploaded_file($_FILES['file']['tmp_name'],
		  $GLOBALS['OMV_CONFIG_FILE']);
		// Execute all registered scripts to create the system services
		// configuration files using the currently restored configuration.
		$outputFile = tempnam(sys_get_temp_dir(), "configimport");
		$cmd = sprintf("sudo omv-configimport >%s 2>&1 & echo $!", $outputFile);
		OMVUtil::exec($cmd, $output, $res);
		if ($res !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Execute init script after system reboot. It will execute
		// configuration import scripts which require various prerequisites,
		// e.g. mounted filesystems to access the shared folders.
		$initd = new OMVSysVInitScript("openmediavault-configimport", TRUE,
		  TRUE, "start 99 2 3 4 5 .");
		$initd->exec();
		// Remember pid and content filename in session
		$pid = intval($output[0]);
		$_SESSION['execjob'][$pid] = array(
			"pid" => $pid,
			"outputFile" => $outputFile
		);
		return $pid;
	}
}
?>
