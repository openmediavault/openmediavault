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

class OMVModuleNtp extends OMVModuleServiceAbstract
  implements OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "ntp";
	}

	/**
	 * Generate the configuration.
	 * @throw E_EXEC_FAILED
	 */
	public function applyConfig() {
		$cmd = new \OMV\System\Process("omv-mkconf", "ntp");
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		// Disable NTP.
		$cmd = new \OMV\System\Process("timedatectl", "set-ntp no");
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
		// Stop the timesyncd daemon.
		$systemCtl = new \OMV\System\SystemCtl("systemd-timesyncd");
		$systemCtl->disable(TRUE);
	}

	/**
	 * Start the managed service.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function startService() {
		$db = &\OMV\Config\Database::getInstance();
		$object = $db->get("//system/time", "conf.system.time");
		if (TRUE !== $object->get("ntp.enable"))
			return;
		// Enable NTP.
		$cmd = new \OMV\System\Process("timedatectl", "set-ntp yes");
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
		// Start the timesyncd daemon.
		$systemCtl = new \OMV\System\SystemCtl("systemd-timesyncd");
		$systemCtl->enable(TRUE);
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.time",
		  array($this, "setDirty"));
    }
}
