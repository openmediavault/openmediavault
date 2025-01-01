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
namespace Engined\Module;

class FSTab extends \OMV\Engine\Module\ServiceAbstract
		implements \OMV\Engine\Notify\IListener {
	public function getName() {
		return "fstab";
	}

	public function getDescription() {
		return gettext("Management of the file systems to be mounted.");
	}

	public function preDeploy() {
		$this->execTasks("delete");
	}

	/**
	 * Helper function to add a new task when a fstab entry has been deleted.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 * @return void
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
		$mp = new \OMV\System\MountPoint($params['dir']);
		if ("/" == $mp->getPath()) {
			throw new \OMV\Exception("Unmounting the root file system ".
				"has been aborted.");
		}
		if (FALSE === $mp->exists()) {
			return;
		}
		// Check whether the file system is mounted and unmount it if
		// necessary.
		if (TRUE === $mp->isMounted()) {
			$mp->umount(TRUE, TRUE);
		}
		// Check whether the directory is still a mount point. If this is
		// the case, then throw an error, otherwise unlinking the
		// directory might delete the file systems data. This can happen
		// when the file system is unmounted but busy (e.g. NFS exports).
		if (TRUE === $mp->isMountPoint()) {
			throw new \OMV\Exception("Removing the mount directory '%s' has ".
				"been aborted, the resource is busy.", $params['dir']);
		}
		// Finally unlink the mount point if it is empty (which should be
		// the normal case).
		$mp->unlink(FALSE, TRUE);
	}

	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.system.filesystem.mountpoint",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.system.filesystem.mountpoint",
			[$this, "onDelete"]);
	}
}
