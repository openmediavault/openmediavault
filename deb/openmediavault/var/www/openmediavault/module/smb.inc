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

class SMBModule extends OMVModuleAbstract implements OMVINotifyListener,
  OMVIModuleStatus {
	private $invalidUsers = array("admin");

	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "smb";
	}

	/**
	 * Get the module status.
	 */
	public function getStatus() {
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/smb";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$initd = new OMVSysVInitScript("samba");
		return array(
			"name" => $this->getName(),
			"title" => gettext("SMB/CIFS"),
			"enabled" => boolval($object['enable']),
			"running" => ($initd->status() == 0)
		);
	}

	/**
	 * Create the configuration file and update the service.
	 */
	public function updateService() {
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/smb";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Create '/etc/samba/smb.conf' file
		$cmd = "sudo omv-mkconf samba 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Update service
		$initd = new OMVSysVInitScript("samba", boolval($object['enable']));
		$initd->exec();
	}

	/**
	 * Force service to reload its configuration when it is enabled.
	 */
	private function reloadConfig() {
		global $xmlConfig;
		// Get the configuration object
		$xpath = "//services/smb";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if (boolval($object['enable']) === TRUE) {
			// Force daemon to reload the configuration
			$initd = new OMVSysVInitScript("samba", TRUE);
			$initd->invoke("force-reload");
		}
	}

	/**
	 * Add an user.
	 */
	public function addUser($object) {
		if (in_array($object['name'], $this->invalidUsers))
			return;
		$cmd = sprintf("(echo '%s'; echo '%s') | sudo pdbedit -at %s 2>&1",
		  $object['password'], $object['password'], $object['name']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		$this->reloadConfig();
	}

	/**
	 * Modify an user.
	 */
	public function modifyUser($object) {
		if (in_array($object['name'], $this->invalidUsers))
			return;
		$cmd = sprintf("(echo '%s'; echo '%s') | sudo smbpasswd -a -s ".
		  "%s 2>&1", $object['password'], $object['password'],
		  $object['name']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		$this->reloadConfig();
	}

	/**
	 * Delete an user.
	 */
	public function deleteUser($object) {
		if (in_array($object['name'], $this->invalidUsers))
			return;
		$cmd = sprintf("sudo pdbedit -x %s 2>&1", $object['name']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		$this->reloadConfig();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		// Update service if configuration has been modified
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smb",
		  array($this, "updateService"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.smb.shares.share",
		  array($this, "updateService"), 1);
		// Add/modify/delete users
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.usermanagement.users.user",
		  array($this, "addUser"), 10); // Unix user must already exist
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.usermanagement.users.user",
		  array($this, "modifyUser"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.usermanagement.users.user",
		  array($this, "deleteUser"), 1); // Must be deleted before unix user
		// Modify share permissions
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder.privileges",
		  array($this, "updateService"));
		// Update Zeroconf if configuration has been modified
		$moduleMgr = &OMVModuleMgr::getInstance();
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smb",
		  array($moduleMgr->getModule("zeroconf"), "mkConf"));
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new SMBModule());
?>
