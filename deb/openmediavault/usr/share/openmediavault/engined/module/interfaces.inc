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

class OMVModuleInterfaces extends OMVModuleServiceAbstract
  implements OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "interfaces";
	}

	/**
	 * Generate the configuration.
	 * @throw E_EXEC_FAILED
	 */
	public function applyConfig() {
		$cmd = "export LANG=C; omv-mkconf interfaces 2>&1";
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		$this->execTasks("delete");
	}

	/**
	 * Helper function to add a new task when a bond interface is deleted.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 * @return None.
	 */
	final public function onDeleteBondInterface($type, $path, $object) {
		$this->addTask("delete", "deleteBondInterface", $object);
	}

	/**
	 * Helper method to remove a bond interface.
	 * @param params The method parameters.
	 */
	final public function deleteBondInterface($params) {
		// See http://backdrift.org/manage-linux-bonding-without-ifenslave-using-sysfs
        $cmd = sprintf("echo '-%s' > /sys/class/net/bonding_masters",
          $params['devicename']);
		$this->exec($cmd);
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.hostname",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.interfaces.iface",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.interfaces.bondiface",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.interfaces.vlaniface",
		  array($this, "setDirty"));
		$moduleMgr = &OMVModuleMgr::getInstance();
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.interfaces.iface",
		  array($moduleMgr->getModule("networking"), "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.interfaces.bondiface",
		  array($moduleMgr->getModule("networking"), "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.interfaces.bondiface",
		  array($this, "onDeleteBondInterface"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.interfaces.vlaniface",
		  array($moduleMgr->getModule("networking"), "setDirty"));
    }
}

// Register module.
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleInterfaces());
?>
