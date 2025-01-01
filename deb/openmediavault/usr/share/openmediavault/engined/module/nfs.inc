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

class Nfs extends \OMV\Engine\Module\ServiceAbstract implements
		\OMV\Engine\Notify\IListener, \OMV\Engine\Module\IServiceStatus {
	public function getName() {
		return "nfs";
	}

	public function deployAfter(): array {
		return ["fstab"];
	}

	public function getStatus() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.nfs");
		$systemCtl = new \OMV\System\SystemCtl("nfs-server");
		return [
			"name" => $this->getName(),
			"title" => gettext("NFS"),
			"enabled" => $object->get("enable"),
			"running" => $systemCtl->isActive()
		];
	}

	/**
	 * Helper function to find out whether the given shared folder
	 * configuration object is used by a NFS share. If it is used, then
	 * check if the associated bind mount needs to be adapted.
	 *
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 * @param oldObject The previous configuration object.
	 */
	final public function onSharedFolder($type, $path, $object, $oldObject) {
		$db = \OMV\Config\Database::getInstance();
		$shareObjects = $db->getByFilter("conf.service.nfs.share", [
			"operator" => "stringEquals",
			"arg0" => "sharedfolderref",
			"arg1" => $object['uuid']
		]);
		if (0 < count($shareObjects)) {
			// Check whether the '/export/{name}' bind mount needs to be
			// adapted. This has to be done when the file system or the
			// relative path of the shared folder has been changed.
			if ($oldObject['mntentref'] !== $object['mntentref'] ||
				$oldObject['reldirpath'] !== $object['reldirpath']
			) {
				// Get the mount point configuration of the modified
				// shared folder.
				$sfMeObject = $db->get(
					"conf.system.filesystem.mountpoint",
					$object['mntentref']);
				// Even if the filter query returns several NFS share
				// objects here, they all use the same '/export/{name}'
				// bind mount.
				$shareObject = $shareObjects[0];
				// Get the configuration of the bind mount that is
				// associated to this NFS share and update it.
				$shareMeObject = $db->get(
					"conf.system.filesystem.mountpoint",
					$shareObject->get("mntentref"));
				$shareMeObject->set("fsname", build_path(
					DIRECTORY_SEPARATOR,
					$sfMeObject->get("dir"),
					$object['reldirpath']));
				$db->set($shareMeObject);
				// Finally mark the associated modules as dirty.
				$this->setDirty();
				$this->setDirtyByName("fstab");
			}
		}
	}

	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.nfs",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.service.nfs.share",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.sharedfolder",
			[$this, "onSharedFolder"]);
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.nfs",
			[$moduleMngr->getModule("zeroconf"), "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.service.nfs.share",
			[$moduleMngr->getModule("zeroconf"), "setDirty"]);
	}
}
