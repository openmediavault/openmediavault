<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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
require_once("openmediavault/config.inc");

class OMVModuleSamba extends \OMV\Engine\Module\ServiceAbstract
  implements \OMV\Engine\Notify\IListener, \OMV\Engine\Module\IServiceStatus, \OMV\Engine\Module\IZeroconf {
	private $invalidUsers = array("admin");

	/**
	 * Get the module name.
	 */
	public function getName() {
		return "samba";
	}

	/**
	 * Generate the configuration.
	 */
	public function applyConfig() {
		$cmd = new \OMV\System\Process("omv-mkconf", "samba");
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		$db = &\OMV\Config\Database::getInstance();
		$object = $db->get("//services/smb", "conf.service.smb");
		if (TRUE !== $object->get("enable"))
			return;
		$systemCtl = new \OMV\System\SystemCtl("smbd");
		$systemCtl->disable(TRUE);
	}

	/**
	 * Start the managed service.
	 */
	public function startService() {
        $db = &\OMV\Config\Database::getInstance();
		$object = $db->get("//services/smb", "conf.service.smb");
		if (TRUE !== $object->get("enable"))
			return;
		$systemCtl = new \OMV\System\SystemCtl("smbd");
		if ($systemCtl->isEnabled() && $systemCtl->isActive())
			$systemCtl->reload(); // running -> reload config
		else
			$systemCtl->enable(TRUE);
	}

	/**
	 * Get the status of the managed service.
	 */
	public function getStatus() {
		$db = &\OMV\Config\Database::getInstance();
		$object = $db->get("//services/smb", "conf.service.smb");
		$systemCtl = new \OMV\System\SystemCtl("smbd");
		return array(
			"name" => $this->getName(),
			"title" => gettext("SMB/CIFS"),
			"enabled" => $object->get("enable"),
			"running" => $systemCtl->isActive()
		);
	}

	/**
	 * Helper function to check whether the given user exists in the
	 * SMB/CIFS user database.
	 * @param username The name of the user.
	 * @return TRUE if the user exists, otherwise FALSE.
	 * @throw E_EXEC_FAILED
	 */
	final private function existsUser($username) {
		$cmd = sprintf("export LANG=C; pdbedit --list | grep '%s:'", $username);
		return (0 !== $this->exec($cmd)) ? FALSE : TRUE;
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
		if (0 !== $this->exec($cmd, $output))
			throw new \OMV\ExecException($cmd, $output);
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
		// Modify the user in the database.
		$cmdArgs = array();
		$cmdArgs[] = "--modify";
		$cmdArgs[] = "--verbose";
		$cmdArgs[] = sprintf("--fullname=%s", escapeshellarg($object['name']));
		if (!empty($object['comment']))
			$cmdArgs[] = sprintf("--account-desc=%s", escapeshellarg(
			  $object['comment']));
		$cmdArgs[] = escapeshellarg($object['name']);
		$cmd = new \OMV\System\Process("pdbedit", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
		// Update the user password. Check whether the password has been
		// changed (in this case it is not empty).
		if (array_key_exists("password", $object) &&
		  !empty($object['password'])) {
			$cmdArgs = array();
			$cmdArgs[] = "export LANG=C;";
			$cmdArgs[] = sprintf("(echo %s; echo %s)", escapeshellarg(
			  $object['password']), escapeshellarg($object['password']));
			$cmdArgs[] = "|";
			$cmdArgs[] = "smbpasswd";
			$cmdArgs[] = "-s";
			$cmdArgs[] = escapeshellarg($object['name']);
			$cmdArgs[] = "2>&1";
			$cmd = implode(" ", $cmdArgs);
			if (0 !== $this->exec($cmd, $output))
				throw new \OMV\ExecException($cmd, $output);
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
		$cmd = new \OMV\System\Process("pdbedit", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
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
		$db = &\OMV\Config\Database::getInstance();
		if (TRUE === $db->getBackend()->exists(sprintf(
		  "//services/smb/shares/share[sharedfolderref='%s']",
		  $object['uuid'])))
			$this->setDirty();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
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
		$moduleMngr = &\OMV\Engine\Module\Manager::getInstance();
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smb",
		  array($moduleMngr->getModule("zeroconf"), "setDirty"));
    }
}
