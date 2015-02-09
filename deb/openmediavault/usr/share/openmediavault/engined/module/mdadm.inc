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
require_once("openmediavault/error.inc");

class OMVModuleMdadm extends OMVModuleServiceAbstract
  implements OMVINotifyListener, OMVIModuleNotification {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "mdadm";
	}

	/**
	 * Defines the modules that if present should start before the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	public function shouldStart() { return array("email"); }

	/**
	 * Generate the configuration.
	 * @throw E_EXEC_FAILED
	 */
	public function applyConfig() {
		$cmd = "export LANG=C; omv-mkconf mdadm 2>&1";
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Get the notification configuration.
	 */
	public function getNotificationConfig() {
		return array(
			  array(
				  "id" => "mdadm",
				  "type" => getText("Storage"),
				  "title" => gettext("Software RAID")
			  )
		  );
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.raid.device",
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

// Register module.
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleMdadm());
?>
