<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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

class AFPModule extends OMVModuleAbstract implements OMVINotifyListener,
  OMVIModuleStatus {
	private $invalidUsers = array("admin");

	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "afp";
	}

	/**
	 * Get the module status.
	 */
	public function getStatus() {
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/afp";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$monit = new OMVMonit();
		return array(
			"name" => $this->getName(),
			"title" => gettext("Apple Filing"),
			"enabled" => boolval($object['enable']),
			"running" => ($monit->status("netatalk") === "running")
		);
	}

	/**
	 * Update the configuration.
	 */
	public function mkConf() {
		$cmd = "sudo omv-mkconf netatalk 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Create the configuration file and update the service.
	 */
	public function updateService() {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/afp";
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
			$initd = new OMVSysVInitScript("netatalk", TRUE);
			$initd->exec();
			// Update monit configuration and start monitoring. Delay some
			// time to give monit enough time to reload its configuration.
			$monit->update();
			usleep(2000000);
			$monit->monitor("netatalk");
			break;
		default:
			// Stop monitoring and update monit configuration.
			$monit->unmonitor("netatalk", TRUE);
			$monit->update();
			// Stop the service.
			$initd = new OMVSysVInitScript("netatalk", FALSE);
			$initd->exec();
			break;
		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		// Update service if configuration has been modified
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.afp",
		  array($this, "updateService"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.afp.shares.share",
		  array($this, "updateService"), 1);
		// Modify share permissions
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder.privileges",
		  array($this, "updateService"));
		// Update Zeroconfig if configuration has been modified
		$moduleMgr = &OMVModuleMgr::getInstance();
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.afp",
		  array($moduleMgr->getModule("zeroconf"), "mkConf"));
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new AFPModule());
?>
