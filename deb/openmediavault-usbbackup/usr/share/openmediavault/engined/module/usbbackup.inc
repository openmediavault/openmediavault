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
class OMVModuleUsbBackup extends \OMV\Engine\Module\ServiceAbstract
  implements \OMV\Engine\Notify\IListener {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "usbbackup";
	}

	/**
	 * Generate the configuration.
	 */
	public function applyConfig() {
		// Build the rsync scripts and systemd unit files.
		$cmd = new \OMV\System\Process("omv-mkconf", "usbbackup");
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Reload systemd unit files.
		$cmd = new \OMV\System\Process("systemctl", "daemon-reload");
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		$this->updateUnitFiles("disable");
	}

	/**
	 * Start the managed service.
	 */
	public function startService() {
		$this->updateUnitFiles("enable");
	}

	/**
	 * Helper function to enable/disable the systemd unit files.
	 */
	private function updateUnitFiles($action) {
		$directory = \OMV\Environment::get(
		  "OMV_USBBACKUP_SYSTEMD_SCRIPTS_DIR",
		  "/lib/systemd/system");
		$prefix = \OMV\Environment::get(
		  "OMV_USBBACKUP_SYSTEMD_UNITFILE_PREFIX",
		  "openmediavault-usbbackup-");
		foreach (new \DirectoryIterator($directory) as $item) {
			if ($item->isDot())
				continue;
			if (FALSE === $item->isFile())
				continue;
			if (FALSE === fnmatch($prefix."*", $item->getFilename()))
				continue;
			$systemCtl = new \OMV\System\SystemCtl($item->getFilename());
			switch ($action) {
			case "enable":
				// Enable the unit, but do not start it.
				$systemCtl->enable(FALSE);
				break;
			case "disable":
				// Disable the unit, but do not stop it.
				$systemCtl->disable(FALSE);
				break;
			}
		}
	}

	/**
	 * Helper function to find out whether the given shared folder
	 * configuration object is used. If it is used, then mark the
	 * module as dirty.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 */
	final public function onSharedFolder($type, $path, $object) {
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $db->exists("conf.service.usbbackup.job", [
			"operator" => "stringEquals",
			"arg0" => "sharedfolderref",
			"arg1" => $object['uuid']
		])) {
			$this->setDirty();
		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.service.usbbackup.job",
			[ $this, "setDirty" ]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.sharedfolder",
			[ $this, "onSharedFolder" ]);
	}
}
