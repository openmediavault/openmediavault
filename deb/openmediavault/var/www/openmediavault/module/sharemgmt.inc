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
require_once("openmediavault/module.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");

class OMVShareMgmtModule extends OMVModuleAbstract
  implements OMVINotifyListener {
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
		$cmd = sprintf("export LANG=C; sudo mkdir -p %s 2>&1",
		  escapeshellarg($dirpath));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Change directory permissions.
		if (array_key_exists("mode", $object)) {
			$cmd = sprintf("export LANG=C; sudo chmod %s %s 2>&1",
			  $object['mode'], escapeshellarg($dirpath));
			OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
		}
		// Change group owner of directory to configured default group,
		// e.g. 'users'.
		$cmd = sprintf("sudo chown :%s %s 2>&1",
		  $GLOBALS['OMV_USERMGMT_DEFAULT_GROUP'], escapeshellarg($dirpath));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Do some checks before a shared folder is deleted.
	 * @param object The shared folder configuration object.
	 * @param recursive Remove the shared folder and its content recursively.
	 */
	public function preDeleteFolder($object, $recursive) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Remove the shared folder and its content recursively? 
		if (!isset($recursive) || ($recursive === FALSE))
			return;
		// Do not delete the shared folder content if a sub-directory is
		// part of another shared folder.
		$xpath = sprintf("//system/shares/sharedfolder[mntentref='%s']",
		  $object['mntentref']);
		$sfobjects = $xmlConfig->getList($xpath);
		if (!is_null($sfobjects)) {
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
			$absDirPath = OMVUtil::readlink(sprintf("%s/%s", $meobject['dir'],
			  $object['reldirpath']));
			// Ensure the path ends with an slash, otherwise comparison may be
			// false positive.
			// Example:
			// To delete = /media/4ee04694-e849-4b97-b31a-a928cc084e8f/test/
			// To check  = /media/4ee04694-e849-4b97-b31a-a928cc084e8f/test_folder
			$absDirPath = rtrim($absDirPath, "/")."/";
			foreach ($sfobjects as $sfobjectk => $sfobjectv) {
				// Skip current processed shared folder configuration object.
				if ($sfobjectv['uuid'] === $object['uuid'])
					continue;
				// Check if the shared folder to be deleted is a top
				// directory of another shared folder configuration object.
				// In this case throw an exception because unlinking the
				// content recursively will delete the content of another
				// shared folder.
				//
				// Example:
				// <volume>
				//  |-dir1
				//  |-dir2
				//  |  |-dir2.1
				//  |  '-dir2.x
				//  .
				//
				// Deleting dir2 recursively will be forbidden when dir2.1
				// is used by another shared folder.
				$sfAbsDirPath = OMVUtil::readlink(sprintf("%s/%s",
				  $meobject['dir'], $sfobjectv['reldirpath']));
				if (0 === stripos($sfAbsDirPath, $absDirPath)) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  gettext("Cannot unlink shared folder content because another shared folder is referencing a subdirectory"));
				}
			}
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
		if (!isset($recursive) || ($recursive === FALSE))
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
		$cmd = sprintf("sudo rm -fr %s 2>&1", escapeshellarg($dirpath));
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
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder",
		  array($this, "createFolder"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.system.shares.sharedfolder",
		  array($this, "preDeleteFolder"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.shares.sharedfolder",
		  array($this, "deleteFolder"));
	}
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVShareMgmtModule());
?>
