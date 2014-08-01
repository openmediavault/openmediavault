<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
require_once("openmediavault/log.inc");

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
		if (FALSE === boolvalEx($object['enable'])) {
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
		if (FALSE === boolvalEx($object['enable']))
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
			"enabled" => boolvalEx($object['enable']),
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
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 * @throw E_EXEC_FAILED
	 */
	final public function onAddUser($type, $path, $object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Skip user if it is listed on the ignore list.
		if (in_array($object['name'], $this->invalidUsers))
			return;
		// Add the new user into the database.
		$cmdArgs = array();
		$cmdArgs[] = "--create";
		$cmdArgs[] = "--verbose";
		$cmdArgs[] = "--password-from-stdin";
		$cmdArgs[] = sprintf("--fullname=%s", escapeshellarg($object['name']));
		if (!empty($object['comment']))
			$cmdArgs[] = sprintf("--account-desc=%s", escapeshellarg(
			  $object['comment']));
		$cmdArgs[] = escapeshellarg($object['name']);
		$cmd = sprintf("export LANG=C; (echo %s; echo %s) | pdbedit %s 2>&1",
		  escapeshellarg($object['password']), escapeshellarg(
		  $object['password']), implode(" ", $cmdArgs));
		if (0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Start/Reload service.
		$this->startService();
	}

	/**
	 * Modify an user.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 * @throw E_EXEC_FAILED
	 */
	final public function onModifyUser($type, $path, $object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Skip user if it is listed on the ignore list.
		if (in_array($object['name'], $this->invalidUsers))
			return;
		// Check if user exists in the database. If it does not exist
		// then add it to the database (this should never happen).
		if (FALSE === $this->existsUser($object['name']))
			return $this->onAddUser($type, $path, $object);
		// Check whether the password has been changed (in this case
		// it is not empty).
		$updatePasswd = (array_key_exists("password", $object) &&
		  !empty($object['password'])) ? TRUE : FALSE;
		// Modify the user in the database.
		$cmdArgs = array();
		$cmdArgs[] = "export LANG=C;";
		if (TRUE === $updatePasswd) {
			$cmdArgs[] = sprintf("(echo %s; echo %s)", escapeshellarg(
			  $object['password']), escapeshellarg($object['password']));
			$cmdArgs[] = "|";
		}
		$cmdArgs[] = "pdbedit";
		$cmdArgs[] = "--modify";
		$cmdArgs[] = "--verbose";
		if (TRUE === $updatePasswd)
			$cmdArgs[] = "--password-from-stdin";
		$cmdArgs[] = sprintf("--fullname=%s", escapeshellarg($object['name']));
		if (!empty($object['comment']))
			$cmdArgs[] = sprintf("--account-desc=%s", escapeshellarg(
			  $object['comment']));
		$cmdArgs[] = escapeshellarg($object['name']);
		$cmdArgs[] = "2>&1";
		$cmd = implode(" ", $cmdArgs);
		if (0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Start/Reload service.
		$this->startService();
	}

	/**
	 * Delete an user.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 * @throw E_EXEC_FAILED
	 */
	final public function onDeleteUser($type, $path, $object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Skip user if it is listed on the ignore list.
		if (in_array($object['name'], $this->invalidUsers))
			return;
		// Check if user exists in the database.
		if (FALSE === $this->existsUser($object['name']))
			return;
		// Delete the user from the database.
		$cmdArgs = array();
		$cmdArgs[] = "--delete";
		$cmdArgs[] = "--verbose";
		$cmdArgs[] = escapeshellarg($object['name']);
		$cmd = sprintf("export LANG=C; pdbedit %s 2>&1", implode(" ",
		  $cmdArgs));
		if (0 !== $this->exec($cmd, $output)) {
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
	 * Helper function to find out whether the given shared folder
	 * configuration object is used. If it is used, then mark the
	 * module as dirty.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 */
	final public function onSharedFolder($type, $path, $object) {
		global $xmlConfig;
		$xpath = sprintf("//services/smb/shares/share[sharedfolderref='%s']",
		  $object['uuid']);
		if(TRUE === $xmlConfig->exists($xpath))
			$this->setDirty();
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
		  "org.openmediavault.system.shares.sharedfolder",
		  array($this, "onSharedFolder"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder.privileges",
		  array($this, "onSharedFolder"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.usermanagement.users.user",
		  array($this, "onAddUser"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.usermanagement.users.user",
		  array($this, "onModifyUser"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.usermanagement.users.user",
		  array($this, "onDeleteUser"));
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

// Register new logfile type.
// Format:
// Mar 13 23:50:48 vmomv5 smbd[4192]: <username>|<ipaddr>|<clientnetbiosname>|<servicerootdir>|<servicename>|<operation>|<result>|<filename>
// Mar 13 23:50:48 vmomv5 smbd[4192]: test|192.168.178.30|pc01|/media/1622d31c-7f5d-4add-be13-20e382a9f5cb/test|download|pwrite|ok|xyz.tar.gz
OMVLogFile::registerType("smbdaudit", array(
  "filename" => "smbd_audit.log",
  "filepath" => "/var/log/samba/audit.log",
  "regex" => "/^(\S+\s+\d+\s+\d{2}:\d{2}:\d{2})\s+(\S+)\ssmbd\[\d+\]:\s+([^\|]+)\|([^\|]+)\|([^\|]+)\|([^\|]+)\|([^\|]+)\|([^\|]+)\|([^\|]+)\|([^\|]+)$/",
  "columns" => array(
	  "ts" => array(
		  "index" => 1,
		  "func" => create_function('$v', 'return strpdate($v,"M j G:i:s");')
	  ),
	  "date" => array(
		  "index" => 1,
		  "func" => create_function('$v',
			'return strftime("%c", strpdate($v,"M j G:i:s"));')
	  ),
	  "hostname" => 2,
	  "username" => 3,
	  "clientipaddr" => 4,
	  "clientnetbiosname" => 5,
	  "servicerootdir" => 6,
	  "servicename" => 7,
	  "operation" => 8,
	  "result" => 9,
	  "filename" => 10
  )));
?>
