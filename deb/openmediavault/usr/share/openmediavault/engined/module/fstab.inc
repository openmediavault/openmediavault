<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2017 Volker Theile
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
class OMVModuleFsTab extends \OMV\Engine\Module\ServiceAbstract
  implements \OMV\Engine\Notify\IListener {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "fstab";
	}

	/**
	 * Generate the configuration.
	 */
	public function applyConfig() {
		$cmd = new \OMV\System\Process("omv-mkconf", "fstab");
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		$this->execTasks("delete");
	}

	/**
	 * Start the managed service.
	 */
	public function startService() {
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.system.filesystem.mountpoint");
		// Process all configured mount points and mount them if necessary.
		foreach ($objects as $objectk => $objectv) {
			// Ensure the mount point exists.
			$mp = new \OMV\System\MountPoint($objectv->get("dir"));
			$mp->create();
			// Mount the filesystem?
			$fs = \OMV\System\Filesystem\Filesystem::getImpl(
				$objectv->get("fsname"));
			if (is_null($fs) || !$fs->exists())
				continue;
			if (FALSE === $fs->isMounted())
				$fs->mount();
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
	 */
	final public function deleteEntry($params) {
		// Try to get the file system. If it is not found, then it might
		// already be wiped or the device is not accessible anymore, e.g.
		// an unplugged USB device.
		$fs = \OMV\System\Filesystem\Filesystem::getImpl($params['fsname']);
		if (is_null($fs) || !$fs->exists())
			return;
		// Check whether the file system is mounted and unmount it if
		// necessary.
		if (TRUE === $fs->isMounted())
			$fs->umount(TRUE, TRUE);
		// Check whether the directory is still a mount point. If this is
		// the case, then throw an error, otherwise unlinking the
		// directory might delete the file systems data. This can happen
		// when the file system is unmounted but busy (e.g. NFS exports).
		$mp = new \OMV\System\MountPoint($params['dir']);
		if (TRUE === $mp->isMountPoint()) {
			throw new \OMV\Exception("Removing the directory '%s' has ".
				"been aborted, the resource is busy.", $params['dir']);
		}
		// Finally unlink the mount point.
		$mp->unlink();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_DELETE,
		  "org.openmediavault.conf.system.filesystem.mountpoint",
		  [ $this, "setDirty" ]);
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.conf.system.filesystem.mountpoint",
		  [ $this, "onDelete" ]);
        $moduleMngr = \OMV\Engine\Module\Manager::getInstance();
        $dispatcher->addListener(
          OMV_NOTIFY_CREATE | OMV_NOTIFY_DELETE,
          "org.openmediavault.conf.system.filesystem.mountpoint",
          [ $moduleMngr->getModule("monit"), "setDirty" ]);
	}
}
