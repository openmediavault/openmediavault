<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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
namespace Engined\Rpc;

class PluginMgmt extends \OMV\Rpc\ServiceAbstract {
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
	 * @throw \OMV\Exception
	 */
	function enumeratePlugins($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Read the plugin information from the plugin index. This will
		// be updated automatically via APT trigger.
		$indexFile = new \OMV\Json\File(\OMV\Environment::get(
		  "OMV_APT_PLUGINS_INDEX_FILE"));
		if (!$indexFile->exists()) {
			throw new \OMV\Exception(
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Enumerate all available packages/plugins.
		$objects = $this->callMethod("enumeratePlugins", NULL, $context);
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
			$params['limit'], $params['sortfield'], $params['sortdir'],
			$params['search']);
	}

	/**
	 * Remove a plugin.
	 * @param params An array containing the following fields:
	 *   \em packages An array of package names to remove.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw \OMV\Exception
	 */
	function remove($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.plugin.remove");
		// Validate package names.
		$prd = new \OMV\ProductInfo();
		$regex = sprintf("/^%s-.+$/", $prd->getPackageName());
		foreach ($params['packages'] as $packagek => $packagev) {
			if (0 == preg_match($regex, $packagev)) {
				throw new \OMV\Exception(
				  "Failed to remove package '%s'. The name of the package ".
				  "does not match the plugin naming convention.",
				  $packagev);
			}
		}
		// Check if the package database is locked.
		\OMV\System\Apt::assertNotLocked();
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params) {
			// Remove the packages.
			$cmdArgs = [];
			$cmdArgs[] = "--yes";
			$cmdArgs[] = "--auto-remove";
			$cmdArgs[] = "--purge";
			$cmdArgs[] = "remove";
			foreach ($params['packages'] as $packagek => $packagev) {
				$cmdArgs[] = escapeshellarg($packagev);
			}
			$cmd = new \OMV\System\Process("apt-get", $cmdArgs);
			$cmd->setEnv("DEBIAN_FRONTEND", "noninteractive");
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
			return $output;
		});
	}

	/**
	 * Upload a package/plugin. Do some plugin specific checks, e.g. the
	 * package name must match a given naming convention.
	 * @param params An array containing the following fields:
	 *   \em filename The original name of the file.
	 *   \em filepath The path to the uploaded file.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\Exception
	 */
	function upload($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.plugin.upload");
		// Extract the package name.
		$cmd = sprintf("export LC_ALL=C.UTF-8; dpkg --field %s | ".
		  "grep 'Package: ' | awk '{print \$2}'", escapeshellarg(
		  $params['filepath']));
		if (0 !== $this->exec($cmd, $output))
			throw new \OMV\ExecException($cmd, $output);
		$packageName = end($output);
		// Get product informations.
		$prd = new \OMV\ProductInfo();
		// Validate package name
		$regex = sprintf("/^%s-.+$/", $prd->getPackageName());
		if (0 == preg_match($regex, $packageName)) {
			throw new \OMV\Exception(
			  "Failed to upload package '%s'. The package name ".
			  "does not match the plugin naming convention.",
			  $params['filename']);
		}
		// Continue to upload the package.
		return \OMV\Rpc\Rpc::call("Apt", "upload", $params, $context);
	}

	/**
	 * Install an package/plugin.
	 * @param params An array containing the following fields:
	 *   \em packages An array of package names to install.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw \OMV\Exception
	 */
	function install($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.plugin.install");
		// Validate package names.
		$prd = new \OMV\ProductInfo();
		$regex = sprintf("/^%s-.+$/", $prd->getPackageName());
		foreach ($params['packages'] as $packagek => $packagev) {
			if (0 == preg_match($regex, $packagev)) {
				throw new \OMV\Exception(
				  "Failed to install package '%s'. The name of the package ".
				  "does not match the plugin naming convention.",
				  $packagev);
			}
		}
		// Check if the package database is locked.
		\OMV\System\Apt::assertNotLocked();
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params) {
			// Install the packages.
			$cmdArgs = [];
			$cmdArgs[] = "--yes";
			$cmdArgs[] = "--allow-downgrades";
			$cmdArgs[] = "--allow-change-held-packages";
			$cmdArgs[] = "--fix-missing";
			$cmdArgs[] = "--allow-unauthenticated";
			$cmdArgs[] = "--reinstall";
			$cmdArgs[] = "install";
			foreach ($params['packages'] as $packagek => $packagev) {
				$cmdArgs[] = escapeshellarg($packagev);
			}
			$cmd = new \OMV\System\Process("apt-get", $cmdArgs);
			$cmd->setEnv("DEBIAN_FRONTEND", "noninteractive");
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
			return $output;
		});
	}
}
