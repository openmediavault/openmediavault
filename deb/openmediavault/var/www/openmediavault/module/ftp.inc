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
require_once("openmediavault/initscript.inc");
require_once("openmediavault/log.inc");
require_once("openmediavault/system.inc");

class FTPModule extends OMVModuleAbstract implements OMVINotifyListener,
  OMVIModuleStatus {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "ftp";
	}

	/**
	 * Get the module status.
	 */
	public function getStatus() {
		global $xmlConfig;
		// Get the configuration object.
		$xpath = "//services/ftp";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$initd = new OMVSysVInitScript("proftpd");
		return array(
			"name" => $this->getName(),
			"title" => gettext("FTP"),
			"enabled" => boolval($object['enable']),
			"running" => ($initd->status() == 0)
		);
	}

	/**
	 * Set/Update share.
	 */
	public function createShare($object) {
		global $xmlConfig;
		// Get the associated mount point configuration object.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $object['mntentref']);
		$meObject = $xmlConfig->get($xpath);
		if (is_null($meObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Mount the subtree.
		$cmd = sprintf("sudo mount %s", $meObject['dir']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Update the service
		self::updateService();
	}

	/**
	 * Delete share.
	 */
	public static function deleteShare($object) {
		global $xmlConfig;
		// Get the associated mount point configuration object.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $object['mntentref']);
		$meObject = $xmlConfig->get($xpath);
		if (is_null($meObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Umount the subtree (if still mounted)
		if (OMVFilesystem::isMountedEx("mountpoint", $meObject['dir'])) {
			$cmd = sprintf("sudo umount -fl %s 2>&1", $meObject['dir']);
			OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
		}
	}

	/**
	 * Create the configuration file and update the service.
	 */
	public static function updateService() {
//		self::debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/ftp";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Create '/etc/proftpd/proftpd.conf' file
		$cmd = "sudo omv-mkconf proftpd 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Update service
		$initd = new OMVSysVInitScript("proftpd", boolval($object['enable']));
		$initd->exec();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		// Update service if configuration has been modified
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp",
		  array($this, "updateService"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.services.ftp.shares.share",
		  array($this, "createShare"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.services.ftp.shares.share",
		  array($this, "deleteShare"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.ftp.shares.share",
		  array($this, "updateService"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp.modules.mod_ban.rule",
		  array($this, "updateService"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.ftp.modules.mod_ban.rule",
		  array($this, "updateService"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp.modules.mod_tls",
		  array($this, "updateService"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.hostname",
		  array($this, "updateService"));
		// Modify share permissions
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder.privileges",
		  array($this, "updateService"));
		// Update Zeroconfig if configuration has been modified
		$moduleMgr = &OMVModuleMgr::getInstance();
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ftp",
		  array($moduleMgr->getModule("zeroconf"), "mkConf"));
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new FTPModule());

// Register new logfile type
OMVLogFile::registerType("proftpd", array(
  "filename" => "proftpd.log",
  "filepath" => "/var/log/proftpd/proftpd.log",
  "regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\sproftpd\[\d+\]: (.*)$/",
  "columns" => array(
	  "date" => array(
		  "index" => 1,
		  "func" => create_function('$v', 'return strpdate($v,"M j G:i:s");')
	  ),
	  "event" => 3
  )));
?>
