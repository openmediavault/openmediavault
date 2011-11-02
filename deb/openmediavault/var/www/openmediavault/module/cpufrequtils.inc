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

class CPUFreqUtilsModule extends OMVModuleAbstract
  implements OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "cpufrequtils";
	}

	/**
	 * Create the configuration file and update the service.
	 */
 	public function updateService() {
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//system/powermanagement";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update the cpufreq configuration file and service
		$cmd = "sudo omv-mkconf cpufrequtils 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Note, when disabling the 'cpufrequtils' service the changes
		// will not take effect before the system is rebooted.
		$initd = new OMVSysVInitScript("loadcpufreq", boolval(
		  $object['cpufreq']), FALSE);
		$initd->exec();
		$initd = new OMVSysVInitScript("cpufrequtils", boolval(
		  $object['cpufreq']), FALSE);
		$initd->exec();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.powermanagement",
		  array($this, "updateService"), 1);
	}
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new CPUFreqUtilsModule());
?>
