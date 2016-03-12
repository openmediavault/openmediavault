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

class OMVModuleAcpid extends OMVModuleServiceAbstract
  implements \OMV\Notify\IListener {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "acpid";
	}

	/**
	 * Generate the configuration.
	 */
	public function applyConfig() {
		$cmd = new \OMV\System\Process("omv-mkconf", "acpid");
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		$systemCtl = new \OMV\System\SystemCtl("acpid");
		$systemCtl->stop();
	}

	/**
	 * Start the managed service.
	 */
	public function startService() {
		$db = &\OMV\Config\Database::getInstance();
		$object = $db->get("//system/powermanagement",
		  "conf.system.powermanagement");
		if (TRUE !== $object->get("powerbtn"))
			return;
		$systemCtl = new \OMV\System\SystemCtl("acpid");
		$systemCtl->start();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(\OMV\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.powermanagement",
		  array($this, "setDirty"));
	}
}
