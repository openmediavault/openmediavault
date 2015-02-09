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
require_once("openmediavault/module.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/system.inc");

class OMVModuleFsTab extends OMVModuleServiceAbstract
  implements OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "fstab";
	}

	/**
	 * Generate the configuration.
	 * @throw E_EXEC_FAILED
	 */
	public function applyConfig() {
		$cmd = "export LANG=C; omv-mkconf fstab 2>&1";
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		$this->execTasks("delete");
	}

	/**
	 * Start the managed service.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_MISC_FAILURE
	 */
	public function startService() {
		global $xmlConfig;
		// Make sure all mount points are mounted.
		$xpath = "//system/fstab/mntent";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Process all configured mount points and mount them if necessary.
		foreach($objects as $objectk => $objectv) {
			$me = new OMVMntEnt($objectv['fsname'], $objectv['dir']);
			// Does the directory exists.
			if(FALSE === $me->mkDir()) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("Failed to create the directory '%s'",
				  $objectv['dir']));
			}
			// Is it mounted?
			if(FALSE === $me->isMounted()) {
				// If it's no loop or bind mount then skip mounting if the
				// filesystem does not exist.
				if (is_uuid($objectv['fsname']) || is_devicefile(
				  $objectv['fsname'])) {
					$fs = OMVFilesystemFactory::get($objectv['fsname']);
					if (is_null($fs) || !$fs->exists())
						continue;
				}
				// Mount it.
				if (FALSE === $me->mount()) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf("Failed to mount '%s': %s", $objectv['fsname'],
					  $me->getLastError()));
				}
			}
		}
	}

	/**
	 * Helper function to add a new task when a fstab entry has been deleted.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 * @return None.
	 */
	final public function onDelete($type, $path, $object) {
		$this->addTask("delete", "deleteEntry", $object);
	}

	/**
	 * Helper method to umount a fstab entry and remove it.
	 * @param params The method parameters.
	 * @throw E_MISC_FAILURE
	 */
	final public function deleteEntry($params) {
		$me = new OMVMntEnt($params['fsname'], $params['dir']);
		// Check whether the file system is mounted and unmount it if
		// necessary.
		if (TRUE === $me->isMounted()) {
			if (FALSE === $me->umount(TRUE, TRUE)) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("Failed to unmount '%s': %s", $params['fsname'],
				  $me->getLastError()));
			}
		}
		// Check whether the directory is still a mount point. If this is
		// the case, then throw an error, otherwise unlinking the
		// directory might delete the file systems data. This can happen
		// when the file system is unmounted but busy (e.g. NFS exports).
		if (TRUE === $me->isMountPoint()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Removing the directory '%s' has been aborted, ".
			  "the resource is busy", $params['dir']));
		}
		// Unlink the mount point directory.
		if (FALSE === $me->unlinkDir()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to unlink the directory '%s': %s",
			  $params['dir'], $me->getLastError()));
		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.fstab.mntent",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.fstab.mntent",
		  array($this, "onDelete"));
        $moduleMgr = &OMVModuleMgr::getInstance();
        $dispatcher->addListener(
          OMV_NOTIFY_CREATE | OMV_NOTIFY_DELETE,
          "org.openmediavault.system.fstab.mntent",
          array($moduleMgr->getModule("monit"), "setDirty"));
	}
}

// Register module.
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleFsTab());
?>
