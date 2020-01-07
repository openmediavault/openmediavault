<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2020 Volker Theile
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
class OMVModuleSharedfolders extends \OMV\Engine\Module\ServiceAbstract
		implements \OMV\Engine\Notify\IListener {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "sharedfolders";
	}

	/**
	 * Defines the modules that if present should start before the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	public function shouldStart() { return [ "systemd" ]; }

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
		$objects = $db->get("conf.system.sharedfolder");
		foreach ($objects as $objectk => $objectv) {
			$unitName = \OMV\System\SystemCtl::escape(build_path(
				DIRECTORY_SEPARATOR,
				\OMV\Environment::get("OMV_SHAREDFOLDERS_DIR",
					"/sharedfolders"),
				$objectv->get("name")), [ "--path", "--suffix=mount" ]);
			// Mount the shared folder bind mount.
			$systemCtl = new \OMV\System\SystemCtl($unitName);
			// Enable the mount unit if this is not done already, otherwise
			// restart it to take care that the unit file is up-to-date. This
			// is necessary because it is possible that the file system of the
			// shared folder may have been changed in the meanwhile.
			if (FALSE === $systemCtl->isEnabled())
				$systemCtl->enable(TRUE);
			else
				$systemCtl->restart();
		}
	}

	/**
	 * Helper function to add a new task when a shared folder has been deleted.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 * @return void
	 */
	final public function onDelete($type, $path, $object) {
		$this->addTask("delete", "deleteEntry", $object);
	}

	/**
	 * Helper method to umount a shared folder alias.
	 * @param params The method parameters.
	 */
	final public function deleteEntry($params) {
		$dirPath = build_path(DIRECTORY_SEPARATOR,
			\OMV\Environment::get("OMV_SHAREDFOLDERS_DIR"),
			$params['name']);
		$unitName = \OMV\System\SystemCtl::escape($dirPath,
			[ "--path", "--suffix=mount" ]);
		// Umount and disable the shared folder bind mount.
		$systemCtl = new \OMV\System\SystemCtl($unitName);
		if ($systemCtl->isActive())
			$systemCtl->disable(TRUE);
		// Unlink the directory.
		$mp = new \OMV\System\MountPoint($dirPath);
		if (FALSE === $mp->isMountPoint())
			$mp->unlink();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.system.sharedfolder",
			[ $this, "setDirty" ]);
		$dispatcher->addListener(
			OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.system.sharedfolder",
			[ $this, "onDelete" ]);
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.system.sharedfolder",
			[ $moduleMngr->getModule("systemd"), "setDirty" ]);
	}
}
