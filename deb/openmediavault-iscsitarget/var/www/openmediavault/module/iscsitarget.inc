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
require_once("openmediavault/monit.inc");
require_once("openmediavault/initscript.inc");

class OMViSCSITargetModule extends OMVModuleAbstract
  implements OMVINotifyListener, OMVIModuleStatus {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "iscsitarget";
	}

	/**
	 * Get the module status.
	 */
	public function getStatus() {
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/iscsitarget";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$initd = new OMVSysVInitScript("iscsitarget");
		return array(
			"name" => $this->getName(),
			"title" => gettext("iSCSI Target"),
			"enabled" => boolval($object['enable']),
			"running" => ($initd->status() == 0)
		);
	}

	/**
	 * Update the configuration.
	 */
	public function mkConf() {
		$cmd = "sudo omv-mkconf iscsitarget 2>&1";
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
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/iscsitarget";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Create configuration
		$this->mkConf();
		// Update service:
		// - Start/Stop service
		// - Activate/deactivate monitoring
		$monit = new OMVMonit();
		switch($object['enable']) {
		case TRUE:
			// Start the service.
			$initd = new OMVSysVInitScript("iscsitarget", TRUE);
			$initd->exec();
			// Update monit configuration and start monitoring. Delay some
			// time to give monit enough time to reload its configuration.
			$monit->update();
			usleep(2000000);
			$monit->monitor("iscsitarget");
			break;
		default:
			// Stop monitoring and update monit configuration.
			$monit->unmonitor("iscsitarget", TRUE);
			$monit->update();
			// Stop the service.
			$initd = new OMVSysVInitScript("iscsitarget", FALSE);
			$initd->exec();
			break;
		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.iscsitarget",
		  array($this, "updateService"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.iscsitarget.targets.target",
		  array($this, "updateService"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.hostname",
		  array($this, "updateService"));
	}
}

// Add config enums (elements that must be handled as arrays)
$xmlConfig->addEnum("lun");
$xmlConfig->addEnum("target");

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMViSCSITargetModule());
?>
