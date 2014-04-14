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
require_once("openmediavault/util.inc");
require_once("openmediavault/product.inc");
require_once("openmediavault/rpcservice.inc");

class OMVRpcServicePluginMgmt extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Plugin";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("enumeratePlugins");
		$this->registerMethod("getList");
		$this->registerMethod("remove");
		$this->registerMethod("upload");
		$this->registerMethod("install");
		$this->registerMethod("makeIndex");
	}

	/**
	 * Enumerate all available plugin packages.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects with the following fields: \em name,
	 *   \em version, \em oldversion, \em architecture, \em package,
	 *   \em priority, \em section, \em installedsize, \em maintainer,
	 *   \em filename, \em size, \em md5sum, \em sha1, \em sha256,
	 *   \em shortdescription, \em homepage, \em installed, \em repository
	 *   and \em _readOnly. The following fields are optional:
	 *   \em description, \em depends, \em replaces and \em conflicts.
	 * @throw E_MISC_FAILURE
	 */
	function enumeratePlugins($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Read the plugin information from the plugin index. This will
		// be updated automatically via APT trigger.
		$indexFile = new OMVJsonFile($GLOBALS['OMV_PLUGIN_INDEX_FILE']);
		if (!$indexFile->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  "Plugin index does not exist.");
		}
		$indexFile->open("r");
		$objects = $indexFile->read();
		$indexFile->close();
		return $objects;
	}

	/**
	 * Get list of available plugin packages.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An object contains the following fields: \em name, \em version,
	 *   \em oldversion, \em architecture, \em package, \em priority,
	 *   \em section, \em installedsize, \em maintainer, \em filename, \em size,
	 *   \em md5sum, \em sha1, \em sha256, \em shortdescription, \em homepage,
	 *   \em installed, \em repository and \em _readOnly. The following fields
	 *   are optional: \em description, \em depends, \em replaces and
	 *   \em conflicts. An exception will be thrown in case of an error.
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
				  "sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
			  }
		  }');
		// Enumerate all available packages/plugins.
		$objects = $this->callMethod("enumeratePlugins", NULL, $context);
		// Add additional information.
		foreach ($objects as $objectk => &$objectv) {
			$objectv['_readOnly'] = !$objectv['installed'];
		}
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Remove a plugin.
	 * @param params An array containing the following fields:
	 *   \em packages An array of package names to remove.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 */
	function remove($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "packages":{"type":"array","items":{"type":"string"}}
			  }
		  }');
		// Create a background process.
		$bgStatusFilename = $this->createBgProcStatus();
		$pid = $this->fork();
		if($pid > 0) { // Parent process.
			$this->initializeBgProcStatus($bgStatusFilename, $pid);
			return $bgStatusFilename;
		}
		// Child process.
		try {
			$bgOutputFilename = $this->createBgProcOutput();
			$this->updateBgProcStatus($bgStatusFilename, "outputfilename",
			  $bgOutputFilename);
			// Get product informations.
			$prd = new OMVProduct();
			// Validate package names.
			$regex = sprintf("/^%s-.+$/", $prd->getPackageName());
			foreach($params['packages'] as $packagek => $packagev) {
				if(0 == preg_match($regex, $packagev)) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf("Failed to remove package '%s'. The name ".
					  "of the package does not match the plugin naming ".
					  "convention.", $packagev));
				}
			}
			// Remove the packages.
			$cmd = sprintf("export LANG=C; apt-get --yes --auto-remove ".
			  "--purge remove %s 2>&1", implode(" ", $params['packages']));
			if(0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			$this->finalizeBgProcStatus($bgStatusFilename, $output);
			exit(0);
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}

	/**
	 * Upload a package/plugin. Do some plugin specific checks, e.g. the
	 * package name must match a given naming convention.
	 * @param params An array containing the following fields:
	 *   \em filename The original name of the file.
	 *   \em filepath The path to the uploaded file.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_FAILED
	 * @throw E_MISC_FAILURE
	 */
	function upload($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "filename":{"type":"string"},
				  "filepath":{"type":"string"}
			  }
		  }');
		// Extract the package name.
		$cmd = sprintf("export LANG=C; dpkg --field %s | ".
		  "grep 'Package: ' | awk '{print \$2}'", escapeshellarg(
		  $params['filepath']));
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		$packageName = end($output);
		// Get product informations.
		$prd = new OMVProduct();
		// Validate package name
		$regex = sprintf("/^%s-.+$/", $prd->getPackageName());
		if(0 == preg_match($regex, $packageName)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to upload package '%s'. The package name " .
			  "does not match the plugin naming convention.",
			  $params['filename']));
		}
		// Continue to upload the package.
		return OMVRpc::exec("Apt", "upload", $params, $context);
	}

	/**
	 * Install an package/plugin.
	 * @param params An array containing the following fields:
	 *   \em packages An array of package names to install.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 */
	function install($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "packages":{"type":"array","items":{"type":"string"}}
			  }
		  }');
		// Create a background process.
		$bgStatusFilename = $this->createBgProcStatus();
		$pid = $this->fork();
		if($pid > 0) { // Parent process.
			$this->initializeBgProcStatus($bgStatusFilename, $pid);
			return $bgStatusFilename;
		}
		// Child process.
		try {
			$bgOutputFilename = $this->createBgProcOutput();
			$this->updateBgProcStatus($bgStatusFilename, "outputfilename",
			  $bgOutputFilename);
			// Get product informations.
			$prd = new OMVProduct();
			// Validate package names.
			$regex = sprintf("/^%s-.+$/", $prd->getPackageName());
			foreach($params['packages'] as $packagek => $packagev) {
				if(0 == preg_match($regex, $packagev)) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf("Failed to install package '%s'. The name ".
					  "of the package does not match the plugin naming ".
					  "convention.", $packagev));
				}
			}
			// Install the packages.
			$cmd = sprintf("export LANG=C; export DEBIAN_FRONTEND=noninteractive; ".
			  "apt-get --yes --force-yes --fix-missing --allow-unauthenticated ".
			  "--reinstall install %s 2>&1", implode(" ", $params['packages']));
			if(0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			$this->finalizeBgProcStatus($bgStatusFilename, $output);
			exit(0);
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}

	/**
	 * Create an index of all available plugins.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects with the following fields: \em name,
	 *   \em version, \em oldversion, \em architecture, \em package,
	 *   \em priority, \em section, \em installedsize, \em maintainer,
	 *   \em filename, \em size, \em md5sum, \em sha1, \em sha256,
	 *   \em shortdescription, \em homepage, \em installed, \em repository
	 *   and \em _readOnly. The following fields are optional:
	 *   \em description, \em depends, \em replaces and \em conflicts.
	 * @throw E_EXEC_FAILED
	 * @throw E_MISC_FAILURE
	 */
	function makeIndex($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get product information.
		$prd = new OMVProduct();
		// List of packages to ignore.
		$except = array();
		$except[] = sprintf("%s-keyring", $prd->getPackageName());
		// Get list of available plugin packages.
		$cmd = sprintf("export LANG=C; apt-cache search %s-",
		  $prd->getPackageName());
		if (0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		$objects = array();
		foreach ($output as $row) {
			$regex = sprintf("/^(%s-\S+) - (.+)$/", $prd->getPackageName());
			if (1 !== preg_match($regex, $row, $matches))
				continue;
			// Skip packages listed in the exception/igore list.
			if (in_array($matches[1], $except))
				continue;
			// Set default package informations.
			$object = array(
				"name" => $matches[1],
				"shortdescription" => $matches[2],
				"description" => "",
				"installed" => FALSE,
				"repository" => ""
			);
			// Get additional package information.
			$cmd = sprintf("export LANG=C; apt-cache --no-all-versions ".
			  "show %s 2>&1", escapeshellarg($object['name']));
			if (0 !== $this->exec($cmd, $output2)) {
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
					$fieldvalue = trim($row);
					if ($fieldvalue === ".")
						$fieldvalue = "\n";
					$object[$fieldname] .= $fieldvalue;
					continue;
				}
				$fieldname = strtolower(str_replace("-", "", $matches[1]));
				$fieldvalue = trim($matches[2]);
				$object[$fieldname] = $fieldvalue;
			}
			if (boolval($GLOBALS['OMV_PLUGIN_ENUMERATE_GET_REPOSITORY'])) {
				// Get the repository where the package comes from. !!! Note,
				// this is a really time consuming process. Because parsing
				// 'apt-cache' output seems to be too difficult, simply
				// simulate the package installation. The 'apt-get' output
				// includes the repository where the package comes from.
				$cmd = sprintf("export LANG=C; apt-get --simulate ".
				  "--reinstall install %s 2>&1", $object['name']);
				if (0 !== $this->exec($cmd, $output2)) {
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
					$regex = "/^Inst\s+(\S+)\s+(\[(\S+)|\])?\s*\((\S+)\s+(.+)".
					  "\s+\[(\S+)\]\).*$/";
					if (1 !== preg_match($regex, $row, $matches))
						continue;
					if (0 == strcasecmp($object['name'], $matches[1])) {
						$object['repository'] = $matches[5];
						break;
					}
				}
			}
			// Append package information object.
			$objects[] = $object;
		}
		// Get list of installed packages.
		unset($output);
		$cmd = sprintf("export LANG=C; dpkg-query --showformat='\${Status}\\t".
		  "\${Package}\\n' --show '%s-*' 2>&1", $prd->getPackageName());
		if (0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Parse output:
		// unknown ok not-installed	openmediavault-ad
		// install ok installed	openmediavault-clamav
		// install ok installed	openmediavault-forkeddaapd
		foreach ($output as $row) {
			$regex = "/^install ok installed\t(\S+)$/";
			if (1 !== preg_match($regex, $row, $matches))
				continue;
			// Skip packages listed in the ignore list.
			if (in_array($matches[1], $except))
				continue;
			// Mark plugin package as installed?
			foreach ($objects as $objectk => &$objectv) {
				if (0 == strcasecmp($objectv['name'], $matches[1])) {
					$objectv['installed'] = TRUE;
					break;
				}
			}
		}
		// Write the index file.
		$indexFile = new OMVJsonFile($GLOBALS['OMV_PLUGIN_INDEX_FILE']);
		$indexFile->open("c+");
		$indexFile->write($objects);
		$indexFile->close();
		// Return the list of plugins.
		return $objects;
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServicePluginMgmt());
?>
