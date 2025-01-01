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
class NetworkUPSTools extends \OMV\Engine\Module\ServiceAbstract implements
		\OMV\Engine\Notify\IListener, \OMV\Engine\Module\IServiceStatus,
		\OMV\Engine\Module\INotification {
	public function getName() {
		return "nut";
	}

	public function deployAfter(): array {
		return ["monit"];
	}

	public function deployBefore(): array {
		return ["collectd"];
	}

	public function getStatus() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.nut");
		$mode = $object->get("mode");
		if ("netclient" === $mode) {
			$systemCtl = new \OMV\System\SystemCtl("nut-monitor");
		} else {
			$systemCtl = new \OMV\System\SystemCtl("nut-server");
		}
		return [
			"name" => $this->getName(),
			"title" => gettext("UPS"),
			"enabled" => $object->get("enable"),
			"running" => $systemCtl->isActive()
		];
	}

	public function getNotificationConfig() {
		return [
			[
				"id" => "nut",
				"type" => getText("Administration"),
				"title" => gettext("UPS")
			]
		];
	}

	/**
	 * Update service if a network interface has been modified. Note,
	 * the service must not be updated if remote monitoring is disabled.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 */
	final public function onUpdateInterface($type, $path, $object) {
		$db = \OMV\Config\Database::getInstance();
		$object2 = $db->get("conf.service.nut");
		// Is remote monitoring enabled?
		if (TRUE === boolvalEx($object2->get("remotemonitor")))
			$this->setDirty();
	}

	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.nut",
			[ $this, "setDirty" ]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.notification.email",
			[ $this, "setDirty" ]);
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.notification.notification",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.system.network.interfaces.iface",
			[ $this, "onUpdateInterface" ]);
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.system.network.interfaces.bondiface",
			[ $this, "onUpdateInterface" ]);
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.nut",
			[ $moduleMngr->getModule("collectd"), "setDirty" ]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.nut",
			[ $moduleMngr->getModule("monit"), "setDirty" ]);
	}
}
