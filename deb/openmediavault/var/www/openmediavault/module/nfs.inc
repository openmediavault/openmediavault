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
require_once("openmediavault/system.inc");

class NFSModule extends OMVModuleAbstract implements OMVINotifyListener,
  OMVIModuleStatus {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "nfs";
	}

	/**
	 * Get the module status.
	 */
	public function getStatus() {
		global $xmlConfig;
		// Get the configuration object.
		$xpath = "//services/nfs";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$initd = new OMVSysVInitScript("nfs-kernel-server");
		return array(
			"name" => $this->getName(),
			"title" => gettext("NFS"),
			"enabled" => boolval($object['enable']),
			"running" => ($initd->status() == 0)
		);
	}

	/**
	 * Set/Update share.
	 */
	public function createShare($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
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
	public function deleteShare($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
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
	public function updateService() {
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/nfs";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Create '/etc/exports' file
		$cmd = "sudo omv-mkconf nfsd 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Update service
		$initd = new OMVSysVInitScript("nfs-kernel-server", boolval(
		  $object['enable']));
		$initd->exec();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		// Update service if configuration has been modified
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.nfs",
		  array($this, "updateService"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.services.nfs.shares.share",
		  array($this, "createShare"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.services.nfs.shares.share",
		  array($this, "deleteShare"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.nfs.shares.share",
		  array($this, "updateService"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.nfs.shares.share",
		  array($this, "updateService"), 1);
		// Update Zeroconfig if configuration has been modified
		$moduleMgr = &OMVModuleMgr::getInstance();
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.nfs",
		  array($moduleMgr->getModule("zeroconf"), "mkConf"));
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new NFSModule());
?>
