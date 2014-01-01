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
require_once("openmediavault/product.inc");

class OMVRpcServiceApt extends OMVRpcServiceAbstract {
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
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
		$this->registerMethod("getUpgraded");
		$this->registerMethod("upgrade");
		$this->registerMethod("update");
		$this->registerMethod("upload");
		$this->registerMethod("getChangeLog");
	}

	/**
	 * Get settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw 
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$xpath = "//system/apt/distribution";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the result object.
		$object['proposed'] = boolval($object['proposed']);
		$object['partner'] = boolval($object['partner']);
		return $object;
	}

	/**
	 * Set settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "proposed":{"type":"boolean"},
				  "partner":{"type":"boolean"}
			  }
		  }');
		// Prepare configuration objects
		$object = array(
			  "proposed" => array_boolval($params, 'proposed'),
			  "partner" => array_boolval($params, 'partner')
		  );
		// Set the configuration object.
		$xpath = "//system/apt/distribution";
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.apt.distribution", $object);
		// Apply the changes immediatelly (no user interaction is wanted).
		OMVRpc::exec("Config", "applyChanges", array(
			  "modules" => array("apt"),
			  "force" => TRUE
		  ), $context);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Get a list of all packages that are to be upgraded.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects containing the fields \em name, \em version,
	 *   \em oldversion, \em repository, \em architecture, \em package,
	 *   \em priority, \em section, \em installedsize, \em maintainer,
	 *   \em filename, \em size, \em md5sum, \em sha1, \em sha256,
	 *   \em description and \em homepage. The following fields are optional:
	 *   \em longdescription, \em depends, \em replaces and \em conflicts.
	 * @throw E_EXEC_FAILED
	 */
	function getUpgraded($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get list of upgraded packages. Use 'dist-upgrade' instead of
		// 'upgrade' to allow the installation of packages with modified
		// dependencies.
		$cmd = "export LANG=C; apt-get --show-upgraded --simulate " .
		  "dist-upgrade 2>&1";
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Get the packages to be installed/updated.
		$objects = array();
		// Parse output:
		// Inst libisccc60 [1:9.7.3.dfsg-1~squeeze3] (1:9.7.3.dfsg-1~squeeze4 Debian:6.0.4/stable, Debian-Security:6.0/stable [amd64])
		// Inst file [5.04-5] (5.04-5+squeeze1 Debian-Security:6.0/stable [i386]) []
		// Inst libxml2 [2.7.8.dfsg-2+squeeze2] (2.7.8.dfsg-2+squeeze3 Debian-Security:6.0/stable [i386])
		// Inst libssl1.0.0 (1.0.0h-1 localhost [amd64])
		foreach($output as $row) {
			$regex = "/^Inst\s+(\S+)\s+(\[(\S+)|\])?\s*\((\S+)\s+(.+)\s+".
			  "\[(\S+)\]\).*$/";
			if(1 !== preg_match($regex, $row, $matches))
				continue;
			// Set default package informations.
			$object = array(
				"name" => $matches[1],
				"oldversion" => $matches[3],
				"version" => $matches[4],
				"repository" => $matches[5],
				"architecture" => $matches[6]
			);
			// Get additional package informations.
			$cmd = sprintf("export LANG=C; apt-cache ".
			  "--no-all-versions show %s 2>&1", $matches[1]);
			if(0 !== $this->exec($cmd, $output2)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output2));
			}
			// Parse output:
			// <fieldname>: <fieldvalue>
			//
			// Package: sudo
			// Version: 1.7.4p4-2.squeeze.3
			// Installed-Size: 920
			// Maintainer: Bdale Garbee <bdale@gag.com>
			// Architecture: i386
			// Replaces: sudo-ldap
			// Depends: libc6 (>= 2.11), libpam0g (>= 0.99.7.1), libpam-modules
			// Conflicts: sudo-ldap
			// Description-en: Provide limited super user privileges to specific users
			//  Sudo is a program designed to allow a sysadmin to give limited root
			//  privileges to users and log root activity.  The basic philosophy is to give
			//  as few privileges as possible but still allow people to get their work done.
			//  .
			//  This version is built with minimal shared library dependencies, use the
			//  sudo-ldap package instead if you need LDAP support for sudoers.
			// Section: admin
			// Priority: optional
			// Filename: pool/updates/main/s/sudo/sudo_1.7.4p4-2.squeeze.3_i386.deb
			// Size: 593224
			// MD5sum: d60e6f516e63b8542d9ee205b83d4295
			// SHA1: c134fa071ff9ca9203dabafe9c0737865e47e737
			// SHA256: 7b5df56299ca8c1e56bb74482f202034eaa40134499baa33a8535ccc366e21e8
			$fieldname = $fieldvalue = "";
			foreach($output2 as $row) {
				$regex = "/^(\S+): (.+)$/";
				if(1 !== preg_match($regex, $row, $matches)) {
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
					if($fieldvalue === ".")
						$fieldvalue = "\n";
					$object[$fieldname] .= sprintf("%s\n", $fieldvalue);
					continue;
				} else {
					$fieldname = strtolower($matches[1]);
					// Map various field names.
					switch ($fieldname) {
					case "description-en":
						$fieldname = "description";
						break;
					case "filename":
						// Extract the filename.
						$matches[2] = basename($matches[2]);
						break;
					default:
						break;
					}
					// Remove '-' character in field name.
					$fieldname = str_replace("-", "", $fieldname);
				}
				$fieldvalue = trim($matches[2]);
				$object[$fieldname] = $fieldvalue;
			}
			// Override 'filename' because it is not the real filename of
			// the package in the filesystem. Instead it is the name of
			// it in the package repository, but this does not contain the
			// epoch of the package and how it is stored in the package
			// archive at /var/cache/apt/archives.
			// E.g.:
			// bind9-host_9.7.3.dfsg-1~squeeze8_amd64.deb (via apt-cache)
			// bind9-host_1:9.7.3.dfsg-1~squeeze8_amd64.deb (in filesystem)
			$object['filename'] = sprintf("%s_%s_%s.deb", $object['package'],
			  $object['version'], $object['architecture']);
			// Append package information object.
			$objects[] = $object;
		}
		return $objects;
	}

	/**
	 * Upgrade the given packages.
	 * @param params An array containing the following fields:
	 *   \em packages An array of package names to upgrade.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 */
	function upgrade($params, $context) {
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
			// Upgrade packages.
			// http://raphaelhertzog.com/2010/09/21/debian-conffile-configuration-file-managed-by-dpkg/
			$bgOutputFilename = $this->createBgProcOutput();
			$this->updateBgProcStatus($bgStatusFilename, "outputfilename",
			  $bgOutputFilename);
			// Install new packages.
			$cmd = sprintf("export LANG=C; export DEBIAN_FRONTEND=noninteractive; ".
			  "apt-get --yes --force-yes --fix-missing --auto-remove ".
			  "--allow-unauthenticated --show-upgraded --option ".
			  "DPkg::Options::=\"--force-confold\" install %s 2>&1",
			  implode(" ", $params['packages']));
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
	 * Update APT cache.
	 * http://newbiedoc.sourceforge.net/system/apt-get-intro.html
	 * http://www.cyberciti.biz/tips/linux-debian-package-management-cheat-sheet.html
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw E_EXEC_FAILED
	 */
	function update($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
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
			// Update package database.
			$cmd = "export LANG=C; apt-get update 2>&1";
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
	 * Upload a package to the local package archive.
	 * @param params An array containing the following fields:
	 *   \em filename The original name of the file.
	 *   \em filepath The path to the uploaded file.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 * @throw E_EXEC_FAILED
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
		// Check the file type.
		$finfo = new finfo(FILEINFO_NONE);
		$fileType = $finfo->file($params['filepath']);
		if(0 == preg_match("/^Debian binary package.+$/", $fileType)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to upload file '%s'. The file is " .
			  "no Debian binary package.", $params['filename']));
		}
		// Move file to local package archive.
		if(!rename($params['filepath'], sprintf("%s/%s",
		  $GLOBALS['OMV_DPKGARCHIVE_DIR'], $params['filename']))) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  "Failed to move package '%s' to local package repository.",
			  $params['filename']));
		}
		// Create the 'Packages' file required by local APT archives.
		// The 'packages' command should be run in the root of the tree.
		$cmd = sprintf("export LANG=C; cd %s && apt-ftparchive " .
		  "packages . > Packages", $GLOBALS['OMV_DPKGARCHIVE_DIR']);
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Get the changelog of an Debian package. The package is downloaded
	 * if necessary to be able to extract the changelog file.
	 * @param params An array containing the following fields:
	 *   \em filename The name of the file, e.g. openssl_0.9.8o-4squeeze13_i386.deb.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw E_EXEC_FAILED
	 * @throw E_MISC_FAILURE
	 */
	function getChangeLog($params, $context) {
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
			// Replace special characters in filename.
			$params['filename'] = str_replace(":", "%3a",
			  $params['filename']);
			// Get the package name.
			$parts = explode("_", $params['filename']);
			$packageName = $parts[0];
			// Force download of the package, thus we are sure it exists in
			// /var/cache/apt/archives. Do not redirect the command output to
			// the backgropund output file. Only the changelog will be
			// redirected to this file.
			$cmd = sprintf("export LANG=C; apt-get --yes --force-yes ".
			  "--download-only --reinstall install %s 2>&1",
			  escapeshellarg($packageName));
			if(0 !== $this->exec($cmd, $output)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			// Extract the changelog from the Debian package.
			if(FALSE === ($tmpDir = mkdtemp())) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  "Failed to created temporary directory.");
			}
			$cmd = sprintf("export LANG=C; dpkg-deb --fsys-tarfile %s/%s | ".
			  "tar -x --wildcards -C %s ./usr/share/doc/%s/changelog* 2>&1",
			  "/var/cache/apt/archives", $params['filename'], $tmpDir,
			  $packageName);
			if(0 !== $this->exec($cmd, $output)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			// Process the extracted changelog. Note, the file is also
			// compressed.
			$found = FALSE;
			foreach(array("changelog.Debian.gz", "changelog.gz") as $filename) {
				$filepath = sprintf("%s/usr/share/doc/%s/%s", $tmpDir,
				  $packageName, $filename);
				if(file_exists($filepath)) {
					// Extract the changelog content and redirect it to the
					// background output file.
					$cmd = sprintf("export LANG=C; gzip -dc < %s", $filepath);
					if(0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
						throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
						  $cmd, implode("\n", $output));
					}
					$found = TRUE;
					break;
				}
			}
			// Changelog found?
			if(FALSE === $found) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  "No changelog found.");
			}
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
$rpcServiceMgr->registerService(new OMVRpcServiceApt());
?>
