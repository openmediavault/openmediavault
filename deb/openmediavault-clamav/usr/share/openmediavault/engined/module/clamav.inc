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
class OMVModuleClamAV extends \OMV\Engine\Module\ServiceAbstract implements
  \OMV\Engine\Notify\IListener, \OMV\Engine\Module\IServiceStatus {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "clamav";
	}

	/**
	 * Generate the configuration.
	 */
	public function applyConfig() {
		$cmd = new \OMV\System\Process("omv-mkconf", "clamav");
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		$systemCtl = new \OMV\System\SystemCtl("clamav-onaccess");
		$systemCtl->disable(FALSE);
		$systemCtl = new \OMV\System\SystemCtl("clamav-daemon");
		$systemCtl->disable(TRUE);
		$systemCtl = new \OMV\System\SystemCtl("clamav-freshclam");
		$systemCtl->disable(TRUE);
	}

	/**
	 * Start the managed service.
	 */
	public function startService() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.clamav");
		if (TRUE !== $object->get("enable"))
			return;
		$systemCtl = new \OMV\System\SystemCtl("clamav-onaccess");
		$systemCtl->enable();
		$systemCtl = new \OMV\System\SystemCtl("clamav-daemon");
		$systemCtl->enable(TRUE);
		if (TRUE === $object->get("freshclam.enable")) {
			$systemCtl = new \OMV\System\SystemCtl("clamav-freshclam");
			$systemCtl->enable(TRUE);
		}
	}

	/**
	 * Get the status of the managed service.
	 */
	public function getStatus() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.clamav");
		$systemCtl = new \OMV\System\SystemCtl("clamav-daemon");
		return [
			"name" => $this->getName(),
			"title" => gettext("Antivirus"),
			"enabled" => $object->get("enable"),
			"running" => $systemCtl->isActive()
		];
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
		if (TRUE === $db->exists("conf.service.clamav.job", [
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
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.clamav",
			[ $this, "setDirty" ]);
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.service.clamav.job",
			[ $this, "setDirty" ]);
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.service.clamav.onaccesspath",
			[ $this, "setDirty" ]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.sharedfolder",
			[ $this, "onSharedFolder" ]);
		$dispatcher->addListener(OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.network.proxy",
			[ $this, "setDirty" ]);
	}
}
