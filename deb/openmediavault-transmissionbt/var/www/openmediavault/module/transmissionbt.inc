<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @author    Marcel Beck <marcel.beck@mbeck.org>
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
require_once("openmediavault/initscript.inc");

class TransmissionBTModule extends OMVModuleAbstract implements
  OMVINotifyListener, OMVIModuleStatus {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "transmissionbt";
	}

	/**
	 * Get the module status.
	 */
	public function getStatus() {
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/transmissionbt";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Check if process is running. Init script does not support 'status'.
		$cmd = "ps --no-heading -C transmission-daemon";
		OMVUtil::exec($cmd, $output, $result);
		return array(
			"name" => $this->getName(),
			"title" => gettext("BitTorrent"),
			"enabled" => boolval($object['enable']),
			"running" => !empty($output[0])
		);
	}

	/**
	 * Update the service.
	 */
	public function updateService() {
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/transmissionbt";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Currently transmission overwrites the settings.json configuration
		// file on exit. This means transmission must be terminated before
		// writing the new settings.json file.
		$initd = new OMVSysVInitScript("transmission-daemon", boolval($object['enable']));
		if (boolval($object['enable']) === false) {
			$initd->stop();
		}
		// Create '/etc/transmission-daemon/settings.json' file
		$cmd = "sudo omv-mkconf transmissionbt 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Update service
		$cmd = "sudo pkill -HUP transmission-da";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			// Start the Service, it is not running
			if (boolval($object['enable']) === true) {
				$initd->exec();
			}
			//throw new OMVException(OMVErrorMsg::E_EXEC_FAILED, $cmd, 'Result: '.$result. implode("\n", $output));
		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		// Update service if configuration has been modified
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.transmissionbt",
		  array($this, "updateService"), 1);
		// Update Zeroconfig if configuration has been modified
		$moduleMgr = &OMVModuleMgr::getInstance();
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.transmissionbt",
		  array($moduleMgr->getModule("zeroconf"), "mkConf"));
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new TransmissionBTModule());
?>
