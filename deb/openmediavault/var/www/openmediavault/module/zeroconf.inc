<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/initscript.inc");

class OMVZeroconfModule extends OMVModuleAbstract
  implements OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "zeroconf";
	}

	/**
	 * Create the configuration file.
	 */
	public function mkConf() {
		// Create '/etc/avahi/avahi-daemon.conf' and
		// '/etc/avahi/services/*.service' files
		$cmd = "sudo omv-mkconf avahi-daemon 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Update the service.
	 */
	public function updateService() {
		// Create configuration files
		$this->mkConf();
		// Restart service
		$initd = new OMVSysVInitScript("avahi-daemon");
		$initd->restart();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.webadmin",
		  array($this, "updateService"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.hostname",
		  array($this, "updateService"));
	}
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVZeroconfModule());
?>
