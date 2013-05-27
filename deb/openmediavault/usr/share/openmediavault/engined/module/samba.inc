<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
require_once("openmediavault/initscript.inc");

class OMVModuleSamba extends OMVModuleServiceAbstract
  implements OMVINotifyListener, OMVIModuleServiceStatus, OMVIModuleZeroconf {
	private $invalidUsers = array("admin");

	/**
	 * Get the module name.
	 */
	public function getName() {
		return "samba";
	}

	/**
	 * Generate the configuration.
	 * @throw E_EXEC_FAILED
	 */
	public function applyConfig() {
		$cmd = "omv-mkconf samba 2>&1";
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		global $xmlConfig;
		$xpath = "//services/smb";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if(FALSE === boolval($object['enable'])) {
			$initd = new OMVSysVInitScript("samba", FALSE);
			$initd->exec();
		}
	}

	/**
	 * Start the managed service.
	 */
	public function startService() {
		global $xmlConfig;
		$xpath = "//services/smb";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if(FALSE === boolval($object['enable']))
			return;
		$initd = new OMVSysVInitScript("samba", TRUE);
		switch($initd->status()) {
		case 0: // running -> reload config
			$initd->invoke("reload");
			break;
		default: // start service
			$initd->exec();
		}
	}

	/**
	 * Get the status of the managed service.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getStatus() {
		global $xmlConfig;
		$xpath = "//services/smb";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
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
	 * Helper functio to check whether the given user exists in the
	 * SMB/CIFS user database.
	 * @param username The name of the user.
	 * @return TRUE if the user exists, otherwise FALSE.
	 * @throw E_EXEC_FAILED
	 */
	final private function existsUser($username) {
		$found = FALSE;
		$cmd = sprintf("pdbedit --list | grep '%s:'", $username);
		return (0 !== $this->exec($cmd, $output)) ? FALSE : TRUE;
	}

	/**
	 * Add an user.
	 * @throw E_EXEC_FAILED
	 */
	final public function addUser($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Skip user if it is listed on the ignore list.
		if(in_array($object['name'], $this->invalidUsers))
			return;
		$cmd = sprintf("(echo '%s'; echo '%s') | pdbedit -vat %s 2>&1",
		  $object['password'], $object['password'], $object['name']);
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Start/Reload service.
		$this->startService();
	}

	/**
	 * Modify an user.
	 * @throw E_EXEC_FAILED
	 */
	final public function modifyUser($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Skip user if it is listed on the ignore list.
		if(in_array($object['name'], $this->invalidUsers))
			return;
		// Check whether the password has been changed (in this case it is
		// not empty) and update it if necessary, otherwise no action is
		// required.
		if(!array_key_exists("password", $object) ||
		  empty($object['password'])) {
			return;
		}
		// Check if user exists in the database. If it does not exist then
		// add it to the database (this should never happen).
		if(FALSE === $this->existsUser($object['name'])) {
			return $this->addUser($object);
		}
		// Modify the user.
		$cmd = sprintf("(echo '%s'; echo '%s') | smbpasswd -s %s 2>&1",
		  $object['password'], $object['password'], $object['name']);
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Start/Reload service.
		$this->startService();
	}

	/**
	 * Delete an user.
	 * @throw E_EXEC_FAILED
	 */
	final public function deleteUser($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Skip user if it is listed on the ignore list.
		if(in_array($object['name'], $this->invalidUsers))
			return;
		// Check if user exists in the database.
		if(FALSE === $this->existsUser($object['name']))
			return;
		// Delete the user.
		$cmd = sprintf("pdbedit -vx %s 2>&1", $object['name']);
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Start/Reload service.
		$this->startService();
	}

	/**
	 * Get the Zeroconf service configuration.
	 */
	public function getZeroconfConfig() {
		return array(
			  "id" => "smb",
			  "title" => gettext("SMB/CIFS")
		  );
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smb",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.smb.shares.share",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder.privileges",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.usermanagement.users.user",
		  array($this, "addUser"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.usermanagement.users.user",
		  array($this, "modifyUser"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.usermanagement.users.user",
		  array($this, "deleteUser"));
		$moduleMgr = &OMVModuleMgr::getInstance();
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smb",
		  array($moduleMgr->getModule("zeroconf"), "setDirty"));
    }
}

// Register module.
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVModuleSamba());
?>
