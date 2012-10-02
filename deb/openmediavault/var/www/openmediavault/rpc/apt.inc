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

class AptRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"upgrade" => array('{
				"type":"object",
				"properties":{
					"packages":{"type":"array","items":{"type":"string"}}
				}
			}')
		);
	}

	/**
	 * Get a list of all packages that are to be upgraded.
	 * @return An array of objects containing the fields \em name, \em version,
	 *   \em oldversion, \em repository, \em architecture, \em package,
	 *   \em priority, \em section, \em installedsize, \em maintainer,
	 *   \em filename, \em size, \em md5sum, \em sha1, \em sha256,
	 *   \em description and \em homepage. The following fields are optional:
	 *   \em longdescription, \em depends, \em replaces and \em conflicts.
	 */
	function getUpgraded() {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get list of upgraded packages. Use 'dist-upgrade' instead of
		// 'upgrade' to allow the installation of packages with modified
		// dependencies.
		$cmd = "export LANG=C; sudo apt-get --show-upgraded --simulate " .
		  "dist-upgrade 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
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
		foreach ($output as $row) {
			$regex = "/^Inst\s+(\S+)\s+(\[(\S+)|\])?\s*\((\S+)\s+(.+)\s+".
			  "\[(\S+)\]\).*$/";
			if (1 !== preg_match($regex, $row, $matches))
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
			$cmd = sprintf("export LANG=C; sudo apt-cache --no-all-versions ".
			  "show %s", $matches[1]);
			unset($output2);
			OMVUtil::exec($cmd, $output2, $result);
			if ($result !== 0) {
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
				} else {
					$fieldname = strtolower($matches[1]);
					// Map various field names.
					switch ($fieldname) {
					case "description-en":
						$fieldname = "description";
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
			// Append package information object.
			$objects[] = $object;
		}
		return $objects;
	}

	/**
	 * Upgrade the given packages.
	 * @param data An array containing the following fields:
	 *   \em packages An array of package names to upgrade.
	 * @return The process id of the 'apt-get' command.
	 */
	function upgrade($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Upgrade packages.
		// http://raphaelhertzog.com/2010/09/21/debian-conffile-configuration-file-managed-by-dpkg/
		$outputFile = tempnam(sys_get_temp_dir(), "aptgetupgrade");
		$cmd = sprintf("export LANG=C; sudo apt-get --yes --force-yes " .
		  "--fix-missing --auto-remove --allow-unauthenticated " .
		  "--show-upgraded --option DPkg::Options::=\"--force-confnew\" " .
		  "install %s >%s 2>&1 & echo $!", implode(" ", $data['packages']),
		  $outputFile);
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

	/**
	 * Update APT cache.
	 * http://newbiedoc.sourceforge.net/system/apt-get-intro.html
	 * http://www.cyberciti.biz/tips/linux-debian-package-management-cheat-sheet.html
	 * @return The process id of the 'apt-get' command.
	 */
	function update() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Update APT cache
		$outputFile = tempnam(sys_get_temp_dir(), "aptgetupdate");
		$cmd = sprintf("export LANG=C; sudo apt-get update >%s 2>&1 & echo $!",
		  $outputFile);
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

	/**
	 * Upload a package to the local package archive.
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
		// Get the real file name.
		$fileName = $_FILES['file']['name'];
		// Check the file type.
		$finfo = new finfo(FILEINFO_NONE);
		$fileType = $finfo->file($_FILES['file']['tmp_name']);
		if(0 == preg_match("/^Debian binary package.+$/", $fileType)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to upload file '%s'. The file is " .
			  "no Debian binary package.", $fileName));
		}
		// Move file to local package archive.
		move_uploaded_file($_FILES['file']['tmp_name'],
		  "{$GLOBALS['OMV_DPKGARCHIVE_DIR']}/{$fileName}");
		// Create the 'Packages' file required by local APT archives.
		// The 'packages' command should be run in the root of the tree.
		$cmd = sprintf("export LANG=C; cd %s && apt-ftparchive " .
		  "packages . > Packages", $GLOBALS['OMV_DPKGARCHIVE_DIR']);
		unset($output);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}
}
?>
