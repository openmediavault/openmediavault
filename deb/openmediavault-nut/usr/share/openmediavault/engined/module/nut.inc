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
class OMVModuleNetworkUPSTools extends \OMV\Engine\Module\ServiceAbstract
  implements \OMV\Engine\Notify\IListener, \OMV\Engine\Module\IServiceStatus {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "nut";
	}

	/**
	 * Defines the modules that if present should start before the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	public function shouldStart() { return [ "monit" ]; }

	/**
	 * Defines the modules that if present should start after the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	function startBefore() { return [ "collectd" ]; }

	/**
	 * Defines the modules that if present should be stopped after the
	 * service provided by this module.
	 * @return An array of modules.
	 */
	function shouldStop() { return [ "monit" ]; }

	/**
	 * Generate the configuration.
	 */
	public function applyConfig() {
		$cmd = new \OMV\System\Process("omv-mkconf", "nut");
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		// Disable monitoring of this service.
		$monit = new \OMV\System\Monit("nut");
		$monit->setGroup(TRUE);
		$monit->unmonitor(TRUE);
		// Stop this service and disable the unit file.
		$systemCtl = new \OMV\System\SystemCtl("nut-monitor");
		$systemCtl->disable(TRUE);
		$systemCtl = new \OMV\System\SystemCtl("nut-server");
		$systemCtl->disable(TRUE);
		$systemCtl = new \OMV\System\SystemCtl("nut-driver");
		$systemCtl->disable(TRUE);
	}

	/**
	 * Start the managed service.
	 */
	public function startService() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.nut");
		if (TRUE !== $object->get("enable"))
			return;
		// Request device events from the kernel.
		$cmd = new \OMV\System\Process("udevadm", "trigger");
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Start this service and enable the unit file.
		$mode = $object->get("mode");
		if ("standalone" === $mode) {
			$systemCtlDrv = new \OMV\System\SystemCtl("nut-driver");
			$systemCtlDrv->enable(TRUE);
			$systemCtlSrv = new \OMV\System\SystemCtl("nut-server");
			$systemCtlSrv->enable(TRUE);
			$systemCtlSrv->waitUntilActive(15);
			$systemCtlMon = new \OMV\System\SystemCtl("nut-monitor");
			$systemCtlMon->enable(TRUE);
		} else if ("netclient" === $mode) {
			$systemCtlMon = new \OMV\System\SystemCtl("nut-monitor");
			$systemCtlMon->enable(TRUE);
			$systemCtlMon->waitUntilActive(15);
		}
		// Enable monitoring of this service.
		$monit = new \OMV\System\Monit("nut");
		$monit->setGroup(TRUE);
		$monit->monitor();
	}

	/**
	 * Get the status of the managed service.
	 */
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

	/**
	 * Bind listeners.
	 */
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
