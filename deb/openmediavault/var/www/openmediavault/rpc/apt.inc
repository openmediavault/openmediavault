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
			"upgrade" => array(
				'{"type":"array","items":{"type":"string"}}'
			)
		);
	}

	/**
	 * Get a list of all packages that are to be upgraded.
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
		$data = array();
		// Parse output:
		// Inst libisccc60 [1:9.7.3.dfsg-1~squeeze3] (1:9.7.3.dfsg-1~squeeze4 Debian:6.0.4/stable, Debian-Security:6.0/stable [amd64])
		// Inst file [5.04-5] (5.04-5+squeeze1 Debian-Security:6.0/stable [i386]) []
		// Inst libxml2 [2.7.8.dfsg-2+squeeze2] (2.7.8.dfsg-2+squeeze3 Debian-Security:6.0/stable [i386])
		// Inst libssl1.0.0 (1.0.0h-1 localhost [amd64])
		$regex = "/^Inst\s+(\S+)\s+(\[(\S+)|\])?\s*\((\S+)\s+(.+)\s+\[(\S+)\]\).*$/";
		foreach ($output as $row) {
			if (1 !== preg_match($regex, $row, $matches))
				continue;
			// Get the package description
			$cmd = sprintf("export LANG=C; sudo apt-cache --no-all-versions " .
			  "show %s | grep '^Description: .*$'", $matches[1]);
			$description = OMVUtil::exec($cmd);
			if ($result !== 0) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			$description = substr($description, 13);
			// Append package information
			$data[] = array(
				"name" => $matches[1],
				"version" => $matches[3],
				"newversion" => $matches[4],
				"repository" => $matches[5],
				"architecture" => $matches[6],
				"description" => $description ? $description : gettext("n/a")
			);
		}
		return $data;
	}

	/**
	 * Upgrade the given packages.
	 * @param pkgNames Array of package names to upgrade.
	 * @return The process id of the 'apt-get' command.
	 */
	function upgrade($pkgNames) {
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
		  "install %s >%s 2>&1 & echo $!", implode(" ", $pkgNames),
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
		// Get the real file name
		$fileName = $_FILES['file']['name'];
		// Move file to local package archive
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
