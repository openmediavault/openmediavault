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
require_once("openmediavault/log.inc");

class RsyncModule extends OMVModuleAbstract implements OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "rsync";
	}

	/**
	 * Create the configuration file.
	 */
	public function mkConf() {
		$cmd = "sudo omv-mkconf rsync 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.rsync.jobs.job",
		  array($this, "mkConf"), 1);
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new RsyncModule());

// Register new logfile type
OMVLogFile::registerType("rsync", array(
  "filename" => "rsync",
  "filepath" => "/var/log/rsync.log",
  "regex" => "/^(\S+\s+\S+) \[\d+\] (.*)$/",
  "columns" => array(
	  "date" => array(
		  "index" => 1,
		  "func" => create_function('$v', 'return strpdate($v,"Y/m/d G:i:s");')
	  ),
	  "event" => 2
  )));
?>
