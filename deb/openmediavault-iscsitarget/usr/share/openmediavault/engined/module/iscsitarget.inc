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
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");

class OMVModuleIscsiTarget extends OMVModuleServiceAbstract
  implements OMVINotifyListener, OMVIModuleServiceStatus {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "iscsitarget";
	}

	/**
	 * Defines the modules that if present should start before the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	public function shouldStart() { return array("monit"); }

	/**
	 * Defines the modules that if present should be stopped after the
	 * service provided by this module.
	 * @return An array of modules.
	 */
	function shouldStop() { return array("monit"); }

	/**
	 * Generate the configuration.
	 * @throw E_EXEC_FAILED
	 */
	public function applyConfig() {
		$cmd = new \OMV\System\Process("omv-mkconf", "iscsitarget");
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		// Disable monitoring of this service.
		$monit = new \OMV\System\Monit("iscsitarget");
		$monit->unmonitor(TRUE);
		// Stop this service and disable the unit file.
		$systemCtl = new \OMV\System\SystemCtl("iscsitarget");
		$systemCtl->disable(TRUE);
	}

	/**
	 * Start the managed service.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function startService() {
		global $xmlConfig;
		$xpath = "//services/iscsitarget";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if (FALSE === boolvalEx($object['enable']))
			return;
		// Start this service and enable the unit file.
		$systemCtl = new \OMV\System\SystemCtl("iscsitarget");
		$systemCtl->enable(TRUE);
		// Wait to allow the service to start.
		sleep(1);
		// Enable monitoring of this service.
		$monit = new \OMV\System\Monit("iscsitarget");
		$monit->monitor();
	}

	/**
	 * Get the status of the managed service.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getStatus() {
		global $xmlConfig;
		$xpath = "//services/iscsitarget";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$monit = new \OMV\System\Monit("iscsitarget");
		return array(
			"name" => $this->getName(),
			"title" => gettext("iSCSI Target"),
			"enabled" => boolvalEx($object['enable']),
			"running" => $monit->isRunning()
		);
	}

	/**
	 * Helper function that is called whenever the basic service settings
	 * have been modified.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The new configuration object.
	 * @param oldObject The previous configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	final public function onModifySettings($type, $path, $object, $oldObject) {
		// Mark this module as dirty.
		$this->setDirty();
		// Check whether the monit daemon configuration needs to be updated.
		// In this case mark the module as dirty.
		if ($oldObject['enable'] != $object['enable'])
			$this->setDirtyByName("monit");
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.iscsitarget",
		  array($this, "onModifySettings"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.iscsitarget.targets.target",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.hostname",
		  array($this, "setDirty"));
	}
}

// Add config enums (elements that must be handled as arrays).
$xmlConfig->addEnum("lun");
$xmlConfig->addEnum("target");

// Register module.
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleIscsiTarget());
