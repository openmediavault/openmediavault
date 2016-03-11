<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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

class OMVModuleSmartMonTools extends OMVModuleServiceAbstract
  implements OMVINotifyListener, OMVIModuleNotification {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "smartmontools";
	}

	/**
	 * Defines the modules that if present should start before the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	public function shouldStart() { return array("email"); }

	/**
	 * Generate the configuration.
	 */
	public function applyConfig() {
		$cmd = new \OMV\System\Process("omv-mkconf", "smartmontools");
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		$systemCtl = new \OMV\System\SystemCtl("smartd");
		$systemCtl->disable(TRUE);
	}

	/**
	 * Start the managed service.
	 */
	public function startService() {
		$db = &\OMV\Config\Database::getInstance();
		$object = $db->get("//services/smart", "conf.service.smartmontools");
		if (TRUE !== $object->get("enable"))
			return;
		$systemCtl = new \OMV\System\SystemCtl("smartd");
		$systemCtl->enable(TRUE);
	}

	/**
	 * Get the notification configuration.
	 */
	public function getNotificationConfig() {
		return array(
			  array(
				  "id" => "smartmontools",
				  "type" => getText("Storage"),
				  "title" => gettext("S.M.A.R.T.")
			  )
		  );
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smart",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smart.monitor.device",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.smart.scheduledtests.job",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.email",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.notification.notifications",
		  array($this, "setDirty"));
    }
}
