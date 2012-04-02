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
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");

class PluginRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getList" => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}' // sortDir
			),
			"remove" => array(
				'{"type":"string"}'
			),
			"install" => array(
				'{"type":"string"}'
			)
		);
	}

	/**
	 * Get list of installed packages/plugins.
	 */
	function getList($start, $count, $sortField, $sortDir) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// List of packages to ignore
		$except = array("openmediavault-keyring");
		// Get list of available packages
		$cmd = "sudo apt-cache search openmediavault-";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		$data = array();
		$pkgAvail = array();
		$regex = "/^(openmediavault-\S+) - (.+)$/";
		foreach ($output as $row) {
			if (1 !== preg_match($regex, $row, $matches))
				continue;
			if (in_array($matches[1], $except))
				continue;
			// Get the package version
			$cmd = sprintf("sudo apt-cache --no-all-versions show %s | " .
			  "grep '^Version: .*$' | awk '{print \$2}'", $matches[1]);
			$version = OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			$data[] = array(
				"name" => $matches[1],
				"version" => $version,
				"description" => $matches[2],
				"installed" => false,
				"_readOnly" => true
			);
		}
		// Get list of installed packages
		$cmd = "sudo COLUMNS=200 dpkg -l";
		unset($output);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		$pkgInstalled = array();
		$regex = "/^ii\s+(openmediavault-\S+)\s+(\S+)\s+(.+)$/";
		foreach ($output as $row) {
			if (1 !== preg_match($regex, $row, $matches))
				continue;
			if (in_array($matches[1], $except))
				continue;
			// Update existing package entry?
			$found = FALSE;
			foreach ($data as $datak => &$datav) {
				if ($datav['name'] === $matches[1]) {
					$datav['installed'] = TRUE;
					$datav['_readOnly'] = FALSE;
					$found = TRUE;
					break;
				}
			}
			// Append new package entry
			if (FALSE === $found) {
				$data[] = array(
					"name" => $matches[1],
					"version" => $matches[2],
					"description" => $matches[3],
					"installed" => TRUE,
					"_readOnly" => FALSE
				);
			}
		}
		// Filter result
		return $this->applyFilter($data, $start, $count, $sortField, $sortDir);
	}

	/**
	 * Remove a package/plugin.
	 * @param pkgName The name of the package/plugin to remove.
	 */
	function remove($pkgName) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Validate package name
		if (0 == preg_match("/^openmediavault-.+$/", $pkgName)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to remove package '%s'. The name of the " .
			  "package does not match the plugin naming convention.",
			  $pkgName));
		}
		$cmd = sprintf("sudo apt-get --yes --auto-remove --purge remove " .
		  "%s 2>&1", $pkgName);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Upload a package/plugin. Do some plugin specific checks, e.g. the
	 * package name must match a given naming convention.
	 */
	function upload() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Is file successfully upload?
		if (!is_uploaded_file($_FILES['file']['tmp_name'])) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  "Invalid upload request");
		}
		// Get the real file name
		$fileName = $_FILES['file']['name'];
		// Extract the package name
		$cmd = sprintf("dpkg --field %s | grep 'Package: ' | " .
		  "awk '{print \$2}'", escapeshellarg($_FILES['file']['tmp_name']));
		$packageName = OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Validate package name
		if (0 == preg_match("/^openmediavault-.+$/", $packageName)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to install package '%s'. The package name " .
			  "does not match the plugin naming convention.", $fileName));
		}
		// Continue to upload the package
		OMVRpc::exec("Apt", "upload");
	}

	/**
	 * Install an package/plugin.
	 * @param pkgName The name of the package/plugin to install.
	 * @return The process id of the 'apt-get' command.
	 */
	function install($pkgName) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Validate package name
		if (0 == preg_match("/^openmediavault-.+$/", $pkgName)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to install package '%s'. The name of the " .
			  "package does not match the plugin naming convention.",
			  $pkgName));
		}
		// Install package
		$outputFile = tempnam(sys_get_temp_dir(), "aptgetinstall");
		$cmd = sprintf("sudo apt-get --yes --force-yes --fix-missing " .
		  "--allow-unauthenticated --reinstall install %s >%s 2>&1 & echo $!",
		  $pkgName, $outputFile);
		OMVUtil::exec($cmd, $output, $res);
		if ($res !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
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
