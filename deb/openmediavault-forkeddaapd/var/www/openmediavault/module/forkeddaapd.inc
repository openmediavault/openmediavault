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

class ForkedDaapdModule extends OMVModuleAbstract implements OMVINotifyListener,
  OMVIModuleStatus {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "daap";
	}

	/**
	 * Get the module status.
	 */
	public function getStatus() {
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/daap";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$initd = new OMVSysVInitScript("forked-daapd");
		return array(
			"name" => $this->getName(),
			"title" => gettext("iTunes/DAAP"),
			"enabled" => boolval($object['enable']),
			"running" => ($initd->status() == 0)
		);
	}

	/**
	 * Update the service.
	 */
	public function updateService($object) {
		$cmd = "sudo omv-mkconf forked-daapd 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Update service:
		// - Start/Stop service
		// - Activate/deactivate monitoring
		$monit = new OMVMonit();
		switch($object['enable']) {
		case TRUE:
			// Start the service.
			$initd = new OMVSysVInitScript("forked-daapd", TRUE);
			$initd->exec();
			// Update monit configuration and start monitoring. Delay some
			// time to give monit enough time to reload its configuration.
			$monit->update();
			usleep(2000000);
			$monit->monitor("forked-daapd");
			break;
		default:
			// Stop monitoring and update monit configuration.
			$monit->unmonitor("forked-daapd", TRUE);
			$monit->update();
			// Stop the service.
			$initd = new OMVSysVInitScript("forked-daapd", FALSE);
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
		  "org.openmediavault.services.daap",
		  array($this, "updateService"), 1);
	}
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new ForkedDaapdModule());
?>
