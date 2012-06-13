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
require_once("openmediavault/log.inc");

class ClamAVModule extends OMVModuleAbstract implements OMVINotifyListener,
  OMVIModuleStatus {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "clamav";
	}

	/**
	 * Get the module status.
	 */
	public function getStatus() {
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/clamav";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$initd = new OMVSysVInitScript("clamav-daemon");
		return array(
			"name" => $this->getName(),
			"title" => gettext("Antivirus"),
			"enabled" => boolval($object['enable']),
			"running" => ($initd->status() == 0)
		);
	}

	/**
	 * Update the configuration.
	 */
	public function mkConf() {
		$cmd = "sudo omv-mkconf clamav 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Create the configuration file and update the service.
	 */
	public function updateService($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Create configuration
		$this->mkConf();
		// Update services.
		$initd = new OMVSysVInitScript("clamav-daemon", boolval(
		  $object['enable']));
		$initd->exec();
		// Enable freshclam daemon only if checks > 0.
		$initd = new OMVSysVInitScript("clamav-freshclam", boolval(
		  $object['enable'] && $object['freshclam']['enable']));
		$initd->exec();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.clamav",
		  array($this, "updateService"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.clamav.jobs.job",
		  array($this, "mkConf"));
	}
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new ClamAVModule());

// Register new logfile type
OMVLogFile::registerType("clamav", array(
  "filename" => "clamav",
  "filepath" => "/var/log/clamav/clamav.log",
  "regex" => "/^(\S+\s+\S+\s+\d+\s+\S+\s+\d+)\s+->\s+(.*)$/",
  "columns" => array(
	  "ts" => array(
		  "index" => 1,
		  "func" => create_function('$v',
			'return strpdate($v,"D M j H:i:s Y");')
	  ),
	  "date" => array(
		  "index" => 1,
		  "func" => create_function('$v',
			'return strftime("%c", strpdate($v,"D M j H:i:s Y"));')
	  ),
	  "event" => 2
  )));
?>
