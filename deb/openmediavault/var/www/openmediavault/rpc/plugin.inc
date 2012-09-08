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
require_once("openmediavault/product.inc");

class PluginRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"remove" => array('{
				"type":"object",
				"properties":{
					"packages":{"type":"array","items":{"type":"string"}}
				}
			}'),
			"install" => array('{
				"type":"object",
				"properties":{
					"packages":{"type":"array","items":{"type":"string"}}
				}
			}')
		);
	}

	/**
	 * Get list of installed packages/plugins.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An object contains the following fields: \em name, \em version,
	 *   \em oldversion, \em architecture, \em package, \em priority,
	 *   \em section, \em installedsize, \em maintainer, \em filename, \em size,
	 *   \em md5sum, \em sha1, \em sha256, \em description, \em homepage,
	 *   \em installed, \em repository and \em _readOnly. The following fields
	 *   are optional: \em longdescription, \em depends, \em replaces and
	 *   \em conflicts. An exception will be thrown in case of an error.
	 */
	function getList($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get product informations
		$prd = new OMVProduct();
		// List of packages to ignore.
		$except = array(sprintf("%s-keyring", $prd->getPackageName()));
		// Get list of available plugin packages.
		$cmd = sprintf("export LANG=C; sudo apt-cache search %s-",
		  $prd->getPackageName());
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		$objects = array();
		foreach ($output as $row) {
			$regex = sprintf("/^(%s-\S+) - (.+)$/", $prd->getPackageName());
			if (1 !== preg_match($regex, $row, $matches))
				continue;
			// Skip packages listed in the ignore list.
			if (in_array($matches[1], $except))
				continue;
			// Set default package informations.
			$object = array(
				"name" => $matches[1],
				"description" => $matches[2],
				"installed" => FALSE,
				"_readOnly" => TRUE
			);
			// Get additional package informations.
			$cmd = sprintf("export LANG=C; sudo apt-cache --no-all-versions ".
			  "show %s", $object['name']);
			OMVUtil::exec($cmd, $output2, $result);
			if ($result !== 0) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output2));
			}
			// Parse output:
			// Package: openmediavault-forkeddaapd
			// Priority: optional
			// ...
			// <fieldname>: <fieldvalue>
			$fieldname = $fieldvalue = "";
			foreach ($output2 as $row) {
				$regex = "/^(\S+): (.+)$/";
				if (1 !== preg_match($regex, $row, $matches)) {
					// Process folded/multiline fields. See
					// http://www.debian.org/doc/debian-policy/ch-controlfields.html
					switch ($fieldname) {
					case "description":
						$fieldname = "longdescription";
						$object[$fieldname] = "";
						break;
					default:
						break;
					}
					$fieldvalue = trim($row);
					if ($fieldvalue === ".")
						$fieldvalue = "\n";
					$object[$fieldname] .= sprintf("%s\n", $fieldvalue);
					continue;
				}
				$fieldname = strtolower(str_replace("-", "", $matches[1]));
				$fieldvalue = trim($matches[2]);
				$object[$fieldname] = $fieldvalue;
			}
			// Get the repository. Because parsing 'apt-cache' output seems
			// to be too difficult, simply simulate the package installation.
			// The 'apt-get' output includes the repository where the package
			// comes from.
			$cmd = sprintf("export LANG=C; sudo apt-get --simulate ".
			  "--reinstall install %s 2>&1", $object['name']);
			OMVUtil::exec($cmd, $output2, $result);
			if ($result !== 0) {
				// Skip packages that fail, e.g. because of unmet
				// dependencies.
				if ($result == 100)
					continue;
				// Throw an exception for all other errors.
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output2));
			}
			// Parse output:
			// Inst libisccc60 [1:9.7.3.dfsg-1~squeeze3] (1:9.7.3.dfsg-1~squeeze4 Debian:6.0.4/stable, Debian-Security:6.0/stable [amd64])
			// Inst file [5.04-5] (5.04-5+squeeze1 Debian-Security:6.0/stable [i386]) []
			// Inst libxml2 [2.7.8.dfsg-2+squeeze2] (2.7.8.dfsg-2+squeeze3 Debian-Security:6.0/stable [i386])
			// Inst libssl1.0.0 (1.0.0h-1 localhost [amd64])
			foreach ($output2 as $row) {
				$regex = "/^Inst\s+(\S+)\s+(\[(\S+)|\])?\s*\((\S+)\s+(.+)\s+".
				  "\[(\S+)\]\).*$/";
				if (1 !== preg_match($regex, $row, $matches))
					continue;
				if ($object['name'] === $matches[1]) {
					$object['repository'] = $matches[5];
					break;
				}
			}
			// Append package information object.
			$objects[] = $object;
		}
		// Get list of installed packages.
		unset($output);
		$cmd = sprintf("export LANG=C;sudo dpkg-query --list %s-*",
		  $prd->getPackageName());
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		foreach ($output as $row) {
			$regex = sprintf("/^ii\s+(%s-\S+)\s+(\S+)\s+(.+)$/",
			  $prd->getPackageName());
			if (1 !== preg_match($regex, $row, $matches))
				continue;
			// Skip packages listed in the ignore list.
			if (in_array($matches[1], $except))
				continue;
			// Mark plugin package as installed?
			foreach ($objects as $objectk => &$objectv) {
				if ($objectv['name'] === $matches[1]) {
					$objectv['installed'] = TRUE;
					$objectv['_readOnly'] = FALSE;
					break;
				}
			}
		}
		// Filter result
		return $this->applyFilter($objects, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Remove a plugin.
	 * @param data An array containing the following fields:
	 *   \em packages An array of package names to remove.
	 * @return None
	 */
	function remove($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get product informations
		$prd = new OMVProduct();
		// Validate package names
		$regex = sprintf("/^%s-.+$/", $prd->getPackageName());
		foreach ($data['packages'] as $packagek => $packagev) {
			if (0 == preg_match($regex, $packagev)) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("Failed to remove package '%s'. The name of the " .
				  "package does not match the plugin naming convention.",
				  $packagev));
			}
		}
		// Remove the packages
		$outputFile = tempnam(sys_get_temp_dir(), "aptgetremove");
		$cmd = sprintf("export LANG=C; sudo apt-get --yes --auto-remove ".
		  "--purge remove %s >%s 2>&1 & echo $!", implode(" ",
		  $data['packages']), $outputFile);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
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
		$cmd = sprintf("export LANG=C; dpkg --field %s | grep 'Package: ' | ".
		  "awk '{print \$2}'", escapeshellarg($_FILES['file']['tmp_name']));
		$packageName = OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Get product informations
		$prd = new OMVProduct();
		// Validate package name
		$regex = sprintf("/^%s-.+$/", $prd->getPackageName());
		if (0 == preg_match($regex, $packageName)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to install package '%s'. The package name " .
			  "does not match the plugin naming convention.", $fileName));
		}
		// Continue to upload the package
		OMVRpc::exec("Apt", "upload");
	}

	/**
	 * Install an package/plugin.
	 * @param data An array containing the following fields:
	 *   \em packages An array of package names to install.
	 * @return The process ID of the 'apt-get' command.
	 */
	function install($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get product informations
		$prd = new OMVProduct();
		// Validate package names
		$regex = sprintf("/^%s-.+$/", $prd->getPackageName());
		foreach ($data['packages'] as $packagek => $packagev) {
			if (0 == preg_match($regex, $packagev)) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("Failed to install package '%s'. The name of the " .
				  "package does not match the plugin naming convention.",
				  $packagev));
			}
		}
		// Install the packages
		$outputFile = tempnam(sys_get_temp_dir(), "aptgetinstall");
		$cmd = sprintf("export LANG=C;sudo apt-get --yes --force-yes ".
		  "--fix-missing --allow-unauthenticated --reinstall install %s >%s ".
		  "2>&1 & echo $!", implode(" ", $data['packages']), $outputFile);
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
