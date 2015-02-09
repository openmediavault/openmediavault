<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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
	}

	/**
	 * Enumerate all available plugin packages. This list is automatically
	 * generated via an APT trigger which is executed when 'apt-get update'
	 * is called. Thus it is not necessary to execute the very cost intensive
	 * code every RPC call.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects with the following fields: \em name,
	 *   \em version, \em oldversion, \em architecture, \em package,
	 *   \em priority, \em section, \em installedsize, \em maintainer,
	 *   \em filename, \em size, \em md5sum, \em sha1, \em sha256,
	 *   \em abstract, \em homepage, \em installed and \em repository.
	 *   The following fields are optional: \em description,
	 *   \em extendeddescription, \em depends, \em replaces
	 *   and \em conflicts.
	 * @throw E_MISC_FAILURE
	 */
	function enumeratePlugins($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Read the plugin information from the plugin index. This will
		// be updated automatically via APT trigger.
		$indexFile = new OMVJsonFile($GLOBALS['OMV_APT_PLUGINS_INDEX_FILE']);
		if (!$indexFile->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  "The index of available plugins does not exist. Please ".
			  "re-synchronize the package index files from their sources.");
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
	 *   \em md5sum, \em sha1, \em sha256, \em abstract, \em homepage,
	 *   \em installed and \em repository. The following fields are optional:
	 *   \em description, \em depends, \em replaces and \em conflicts. An
	 *   exception will be thrown in case of an error.
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
			// Finalize the background process.
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
			// Finalize the background process.
			$this->finalizeBgProcStatus($bgStatusFilename, $output);
			exit(0);
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServicePluginMgmt());
?>
