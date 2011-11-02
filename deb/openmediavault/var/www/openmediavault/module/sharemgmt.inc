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
require_once("openmediavault/module.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");

class ShareMgmtModule extends OMVModuleAbstract implements OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "sharemgmt";
	}

	/**
	 * Create a shared folder.
	 */
	public function createFolder($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Get the mount point configuration object to build the absolute
		// shared folder path.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $object['mntentref']);
		$meobject = $xmlConfig->get($xpath);
		if (is_null($meobject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Build the absolute shared folder path.
		$dirpath = sprintf("%s/%s", $meobject['dir'], $object['reldirpath']);
		// Create the shared folder directory.
		$cmd = sprintf("sudo mkdir -p --mode %s '%s' 2>&1", $object['umask'],
		  $dirpath);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Update a shared folder permissions.
	 */
	public function updateFolder($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Get the mount point configuration object to build the absolute
		// shared folder path.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $object['mntentref']);
		$meobject = $xmlConfig->get($xpath);
		if (is_null($meobject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Build the absolute shared folder path.
		$dirpath = sprintf("%s/%s", $meobject['dir'], $object['reldirpath']);
		// Update the shared folder permissions.
		$cmd = sprintf("sudo chmod %s '%s' 2>&1", $object['umask'], $dirpath);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Delete a shared folder.
	 * @param object The shared folder configuration object.
	 * @param recursive Remove the shared folder and its content recursively.
	 */
	public function deleteFolder($object, $recursive) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Remove the shared folder and its content recursively? 
		if (isset($recursive) && ($recursive === FALSE))
			return;
		// Get the mount point configuration object to build the absolute
		// shared folder path.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $object['mntentref']);
		$meobject = $xmlConfig->get($xpath);
		if (is_null($meobject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Build the absolute shared folder path.
		$dirpath = sprintf("%s/%s", $meobject['dir'], $object['reldirpath']);
		// Delete the shared folder directory.
		$cmd = sprintf("sudo rm -fr '%s' 2>&1", $dirpath);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Update a shared folder privileges.
	 */
	public function updatePrivileges($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$cmd = sprintf("sudo omv-mkconf facl %s 2>&1", $object['uuid']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		// Create/update shared folder if configuration has been modified
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.shares.sharedfolder",
		  array($this, "createFolder"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder",
		  array($this, "updateFolder"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.system.shares.sharedfolder",
		  array($this, "deleteFolder"));
		// Update ACL privileges if configuration has been modified
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder.privileges",
		  array($this, "updatePrivileges"), 1);
	}
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new ShareMgmtModule());
?>
