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

class Apt extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Apt";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getSoftwareSettings");
		$this->registerMethod("setSoftwareSettings");
		$this->registerMethod("getUpdatesSettings");
		$this->registerMethod("setUpdatesSettings");
		$this->registerMethod("enumerateUpgraded");
		$this->registerMethod("getUpgradedList");
		$this->registerMethod("install");
		$this->registerMethod("upgrade");
		$this->registerMethod("update");
		$this->registerMethod("upload");
		$this->registerMethod("getChangeLog");
	}

	/**
	 * Get software settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function getSoftwareSettings($params, $context) {
		return \OMV\Rpc\Rpc::call("Config", "get", [
			"id" => "conf.system.apt.distribution"
		], $context);
	}

	/**
	 * Set software settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setSoftwareSettings($params, $context) {
		$object = \OMV\Rpc\Rpc::call("Config", "set", [
			"id" => "conf.system.apt.distribution",
			"data" => $params
		], $context);
		// Apply the changes immediately (no user interaction is wanted).
		\OMV\Rpc\Rpc::call("Config", "applyChanges", [
			"modules" => [ "apt" ],
			"force" => TRUE
		], $context);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Get updates settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function getUpdatesSettings($params, $context) {
		return \OMV\Rpc\Rpc::call("Config", "get", [
			"id" => "conf.system.apt.updates"
		], $context);
	}

	/**
	 * Set updates settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setUpdatesSettings($params, $context) {
		return \OMV\Rpc\Rpc::call("Config", "set", [
			"id" => "conf.system.apt.updates",
			"data" => $params
		], $context);
	}

	/**
	 * Enumerate all packages that are available to be upgraded.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return array An array of objects containing the fields \em name,
	 *   \em version, \em oldversion, \em repository, \em architecture,
	 *   \em package, \em priority, \em section, \em installedsize,
	 *   \em maintainer, \em filename, \em size, \em md5sum, \em sha1,
	 *   \em sha256, \em abstract and \em homepage.
	 *   The following fields are optional: \em description, \em depends,
	 *   \em replaces and \em conflicts.
	 */
	function enumerateUpgraded($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		return \OMV\System\System::getAptUpgradeList();
	}

	/**
	 * Get a list of all packages that are to be upgraded.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An object contains the following fields: \em name, \em version,
	 *   \em oldversion, \em repository, \em architecture, \em package,
	 *   \em priority, \em section, \em installedsize, \em maintainer,
	 *   \em filename, \em size, \em md5sum, \em sha1, \em sha256,
	 *   \em abstract and \em homepage. The following fields are
	 *   optional: \em description, \em extendeddescription, \em depends,
	 *   \em replaces and \em conflicts.
	 */
	function getUpgradedList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Enumerate all packages that are to be upgraded.
		$objects = $this->callMethod("enumerateUpgraded", NULL, $context);
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Install the given packages.
	 * @param params An array containing the following fields:
	 *   \em packages An array of package names to upgrade.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 */
	function install($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.apt.install");
		// Check if the package database is locked.
		\OMV\System\Apt::assertNotLocked();
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params) {
			// Upgrade the given packages.
			// http://raphaelhertzog.com/2010/09/21/debian-conffile-configuration-file-managed-by-dpkg/
			$cmdArgs = [];
			$cmdArgs[] = "--yes";
			$cmdArgs[] = "--allow-downgrades";
			$cmdArgs[] = "--allow-change-held-packages";
			$cmdArgs[] = "--fix-broken"; // Fix broken dependencies in place.
			$cmdArgs[] = "--fix-missing";
			$cmdArgs[] = "--auto-remove";
			$cmdArgs[] = "--allow-unauthenticated";
			$cmdArgs[] = "--show-upgraded";
			$cmdArgs[] = "--option DPkg::Options::=\"--force-confold\"";
			$cmdArgs[] = "install";
			foreach ($params['packages'] as $packagek => $packagev) {
				$cmdArgs[] = escapeshellarg($packagev);
			}
			$cmd = new \OMV\System\Process("apt-get", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->setEnv("DEBIAN_FRONTEND", "noninteractive");
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
			return $output;
		});
	}

	/**
	 * Packages currently installed with new versions available are
	 * retrieved and upgraded. In addition to performing the function
	 * of upgrade, changing dependencies with new versions of packages
	 * are intelligently handled.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 */
	function upgrade($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Check if the package database is locked.
		\OMV\System\Apt::assertNotLocked();
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params) {
			// See https://manpages.debian.org/apt/apt-get.8.en.html
			$cmdArgs = [];
			$cmdArgs[] = "--yes";
			$cmdArgs[] = "--allow-downgrades";
			$cmdArgs[] = "--allow-change-held-packages";
			$cmdArgs[] = "--fix-broken"; // Fix broken dependencies in place.
			$cmdArgs[] = "--fix-missing";
			$cmdArgs[] = "--auto-remove";
			$cmdArgs[] = "--allow-unauthenticated";
			$cmdArgs[] = "--show-upgraded";
			$cmdArgs[] = "--option DPkg::Options::=\"--force-confold\"";
			$cmdArgs[] = "dist-upgrade";
			$cmd = new \OMV\System\Process("apt-get", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->setEnv("DEBIAN_FRONTEND", "noninteractive");
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
			return $output;
		});
	}

	/**
	 * Update APT cache.
	 * http://newbiedoc.sourceforge.net/system/apt-get-intro.html
	 * http://www.cyberciti.biz/tips/linux-debian-package-management-cheat-sheet.html
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 */
	function update($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		if ($this->isModuleDirty("apt")) {
			throw new \OMV\Config\ConfigDirtyException();
		}
		// Check if the package database is locked.
		\OMV\System\Apt::assertNotLocked();
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params) {
			// Update package database.
			$cmd = new \OMV\System\Process("apt-get", "update");
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
			return $output;
		});
	}

	/**
	 * Upload a package to the local package archive.
	 * @param params An array containing the following fields:
	 *   \em filename The original name of the file.
	 *   \em filepath The path to the uploaded file.
	 * @param context The context of the caller.
	 * @return void
	 */
	function upload($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.apt.upload");
		// Check the file type.
		$finfo = new \finfo(FILEINFO_NONE);
		$fileType = $finfo->file($params['filepath']);
		if (0 == preg_match("/^Debian binary package.+$/", $fileType)) {
			throw new \OMV\Exception(
			  "Failed to upload the file '%s'. It is no Debian binary package.",
			  $params['filename']);
		}
		// Move file to local package archive.
		if (!rename($params['filepath'], build_path(DIRECTORY_SEPARATOR,
		  \OMV\Environment::get("OMV_DPKGARCHIVE_DIR"),
		  $params['filename']))) {
			throw new \OMV\Exception(
			  "Failed to move the package '%s' to local package repository.",
			  $params['filename']);
		}
		// Create the 'Packages' file required by local APT archives.
		// The 'packages' command should be run in the root of the tree.
		$cmd = sprintf("export LC_ALL=C.UTF-8; cd %s && apt-ftparchive " .
		  "packages . > Packages", \OMV\Environment::get(
		  "OMV_DPKGARCHIVE_DIR"));
		if (0 !== $this->exec($cmd, $output))
			throw new \OMV\ExecException($cmd, $output);
	}

	/**
	 * Get the changelog of an Debian package. The package is downloaded
	 * if necessary to be able to extract the changelog file.
	 * @param params An array containing the following fields:
	 *   \em filename The name of the file, e.g. <ul>
	 *   \li openssl_0.9.8o-4squeeze13_i386.deb
	 *   \li pool/updates/main/e/eglibc/libc6_2.13-38+deb7u4_amd64.deb
	 *   </ul>
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 */
	function getChangeLog($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.apt.getchangelog");
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params) {
			// Get the package name. Note, there are files like
			// bind9-host_1%3a9.9.5.dfsg-9+deb8u6_amd64.deb
			// that need to be processed correctly. The problem is
			// that the filename retrieved from the backend is
			// bind9-host_9.9.5.dfsg-9+deb8u6_amd64.deb in this case.
			$parts = explode("_", basename($params['filename'], ".deb"));
			$packageName = $parts[0];
			// Build the file path to the package archive using wildcards
			// to be able to find packages mentioned above.
			// A package name is build like this:
			// <name>_<version>_<architecture>.deb
			$packagePath = build_path(DIRECTORY_SEPARATOR,
			  "/var/cache/apt/archives", sprintf("%s_*%s_%s.deb",
			  $parts[0], $parts[1], $parts[2]));
			// Force download of the package if it does not exist in the
			// package archive (/var/cache/apt/archives). Do not redirect
			// the command output to the backgropund output file. Only the
			// changelog will be redirected to this file.
			if (TRUE === empty(glob($packagePath))) {
				$cmdArgs = [];
				$cmdArgs[] = "--yes";
				$cmdArgs[] = "--allow-downgrades";
				$cmdArgs[] = "--allow-change-held-packages";
				$cmdArgs[] = "--download-only";
				$cmdArgs[] = "--reinstall";
				$cmdArgs[] = "install";
				$cmdArgs[] = escapeshellarg($packageName);
				$cmd = new \OMV\System\Process("apt-get", $cmdArgs);
				$cmd->setRedirect2to1();
				$cmd->execute();
			}
			// The package should exist now.
			$matches = glob($packagePath);
			$packagePath = $matches[0];
			if (FALSE === file_exists($packagePath)) {
				throw new \OMV\Exception("Package file '%s' not found.",
				  $params['filename']);
			}
//			// Extract the changelog content and redirect it to the
//			// background output file.
//			$cmdArgs = [];
//			$cmdArgs[] = "--all";
//			$cmdArgs[] = "--frontend=text";
//			$cmdArgs[] = "--which=changelogs";
//			$cmdArgs[] = escapeshellarg($packagePath);
//			$cmd = new \OMV\System\Process("apt-listchanges", $cmdArgs);
//			$cmd->setRedirect2to1();
//			if (0 !== $this->exec($cmd, $output, $bgOutputFilename))
//				throw new \OMV\ExecException($cmd, $output);
//			return $output;
			// Extract the changelog from the Debian package.
			if (FALSE === ($tmpDir = mkdtemp())) {
				throw new \OMV\Exception(
				  "Failed to created a temporary directory.");
			}
			$cmd = sprintf("export LC_ALL=C.UTF-8; dpkg-deb --fsys-tarfile %s | ".
			  "tar --extract --wildcards --directory=%s ".
			  "./usr/share/doc/%s/changelog\* 2>&1",
			  escapeshellarg($packagePath), escapeshellarg($tmpDir),
			  $packageName);
			if (0 !== $this->exec($cmd, $output))
				throw new \OMV\ExecException($cmd, $output);
			// Process the extracted changelog. Note, the file is also
			// compressed. The changelog file may be named like:
			// - changelog.gz
			// - changelog.Debian.gz
			$matches = glob(build_path(DIRECTORY_SEPARATOR, $tmpDir,
			  "usr/share/doc", $packageName, "changelog*.gz"));
			if (TRUE === empty($matches))
				throw new \OMV\Exception("No changelog found.");
			// Extract the changelog content and redirect it to the
			// background output file.
			$cmdArgs = [];
			$cmdArgs[] = "--decompress";
			$cmdArgs[] = "--stdout";
			$cmd = new \OMV\System\Process("gzip", $cmdArgs);
			$cmd->setInputFromFile($matches[0]);
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
			return $output;
		});
	}
}
