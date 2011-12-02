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
require_once("openmediavault/initscript.inc");
require_once("openmediavault/log.inc");

class SMARTModule extends OMVModuleAbstract implements OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "smart";
	}

	/**
	 * Create the configuration file and update the service.
	 */
	public function updateService() {
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/smart";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Create '/etc/smartd.conf' file
		$cmd = "sudo omv-mkconf smartmontools 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Update service
		$initd = new OMVSysVInitScript("smartmontools", boolval(
		  $object['enable']));
		$initd->exec();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smart",
		  array($this, "updateService"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.hdparm",
		  array($this, "updateService"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smart.scheduledtests.job",
		  array($this, "updateService"));
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new SMARTModule());

// Register new logfile type
OMVLogFile::registerType("smartd", array(
  "filename" => "smartd.log",
  "filepath" => "/var/log/smartd.log",
  "regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\ssmartd\[\d+\]: (.*)$/",
  "columns" => array(
	  "date" => array(
		  "index" => 1,
		  "func" => create_function('$v', 'return strpdate($v,"M j G:i:s");')
	  ),
	  "event" => 3
  )));
?>
