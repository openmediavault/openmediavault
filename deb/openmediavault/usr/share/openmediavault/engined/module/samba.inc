<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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
namespace Engined\Module;

class Samba extends \OMV\Engine\Module\ServiceAbstract implements
		\OMV\Engine\Notify\IListener, \OMV\Engine\Module\IServiceStatus {
	private $invalidUsers = ["admin"];

	public function getName() {
		return "samba";
	}

	public function getDescription() {
		return gettext("SMB/CIFS file, print, and login server for Unix.");
	}

	public function getStatus() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.smb");
		$systemCtl = new \OMV\System\SystemCtl("smbd");
		return [
			"name" => $this->getName(),
			"title" => gettext("SMB/CIFS"),
			"enabled" => $object->get("enable"),
			"running" => $systemCtl->isActive()
		];
	}

	/**
	 * Helper function to check whether the given user exists in the
	 * SMB/CIFS user database.
	 * @param username The name of the user.
	 * @return bool TRUE if the user exists, otherwise FALSE.
	 */
	private function existsUser($username) {
		$cmdLine = sprintf("pdbedit --list | grep '%s:'", $username);
		$cmd = new \OMV\System\Process($cmdLine);
		$cmd->setQuiet(TRUE);
		$cmd->execute($output, $exitStatus);
		return (0 == $exitStatus) ? TRUE : FALSE;
	}

	/**
	 * Add an user.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 */
	final public function onAddUser($type, $path, $object) {
		// Skip this notification if the user is listed in the ignore list.
		if (in_array($object['name'], $this->invalidUsers))
			return;
		// Skip this notification if no password is included.
		if (FALSE === array_key_exists("password", $object))
			return;
		// Add the new user into the database.
		$pwdFile = new \OMV\System\TmpFile();
		$pwdFile->write(sprintf("%s\n%s\n", $object['password'],
			$object['password']));
		$cmdArgs = [];
		$cmdArgs[] = sprintf("--configfile=%s", escapeshellarg(
			"/etc/samba/smb.conf"));
		$cmdArgs[] = "--create";
		$cmdArgs[] = "--verbose";
		$cmdArgs[] = "--password-from-stdin";
		$cmdArgs[] = sprintf("--fullname=%s", escapeshellarg($object['name']));
		if (!empty($object['comment'])) {
			$cmdArgs[] = sprintf("--account-desc=%s", escapeshellarg(
				$object['comment']));
		}
		$cmdArgs[] = escapeshellarg($object['name']);
		$cmd = new \OMV\System\Process("pdbedit", $cmdArgs);
		$cmd->setInputFromFile($pwdFile->getFilename());
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Modify an user.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 */
	final public function onModifyUser($type, $path, $object) {
		// Skip user if it is listed on the ignore list.
		if (in_array($object['name'], $this->invalidUsers))
			return;
		// Check if user exists in the database. If it does not exist
		// then add it to the database (this should never happen).
		if (FALSE === $this->existsUser($object['name']))
			return $this->onAddUser($type, $path, $object);
		// Modify the user in the database.
		$cmdArgs = [];
		$cmdArgs[] = sprintf("--configfile=%s", escapeshellarg(
			"/etc/samba/smb.conf"));
		$cmdArgs[] = "--modify";
		$cmdArgs[] = "--verbose";
		$cmdArgs[] = sprintf("--fullname=%s", escapeshellarg($object['name']));
		if (!empty($object['comment'])) {
			$cmdArgs[] = sprintf("--account-desc=%s", escapeshellarg(
				$object['comment']));
		}
		$cmdArgs[] = escapeshellarg($object['name']);
		$cmd = new \OMV\System\Process("pdbedit", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Update the user password. Check whether the password has been
		// changed (in this case it is not empty).
		if (array_key_exists("password", $object) &&
				!empty($object['password'])) {
			$pwdFile = new \OMV\System\TmpFile();
			$pwdFile->write(sprintf("%s\n%s\n", $object['password'],
				$object['password']));
			$cmdArgs = [];
			$cmdArgs[] = sprintf("-c %s", escapeshellarg(
				"/etc/samba/smb.conf"));
			$cmdArgs[] = "-s";
			$cmdArgs[] = escapeshellarg($object['name']);
			$cmd = new \OMV\System\Process("smbpasswd", $cmdArgs);
			$cmd->setInputFromFile($pwdFile->getFilename());
			$cmd->setRedirect2to1();
			$cmd->execute();
		}
	}

	/**
	 * Delete an user.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 */
	final public function onDeleteUser($type, $path, $object) {
		// Skip user if it is listed on the ignore list.
		if (in_array($object['name'], $this->invalidUsers))
			return;
		// Check if user exists in the database.
		if (FALSE === $this->existsUser($object['name']))
			return;
		// Delete the user from the database.
		$cmdArgs = [];
		$cmdArgs[] = sprintf("-c %s", escapeshellarg(
			"/etc/samba/smb.conf"));
		$cmdArgs[] = "--delete";
		$cmdArgs[] = "--verbose";
		$cmdArgs[] = escapeshellarg($object['name']);
		$cmd = new \OMV\System\Process("pdbedit", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
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
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $db->exists("conf.service.smb.share", [
			"operator" => "stringEquals",
			"arg0" => "sharedfolderref",
			"arg1" => $object['uuid']
		])) {
			$this->setDirty();
		}
	}

	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.smb",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.service.smb.share",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.usermanagement.homedirectory",
			[$this, "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.sharedfolder",
			[$this, "onSharedFolder"]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.sharedfolder.privilege",
			[$this, "onSharedFolder"]);
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE,
			"org.openmediavault.conf.system.usermngmnt.user",
			[$this, "onAddUser"]);
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.usermngmnt.user",
			[$this, "onModifyUser"]);
		$dispatcher->addListener(
			OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.system.usermngmnt.user",
			[$this, "onDeleteUser"]);
		// The smbd daemon must be restarted when groups are modified
		// in order to take these changes into action, otherwise it is
		// possible that a user still works with privileges that are
		// already obsolete.
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.usermngmnt.group",
			[$this, "setDirty"]);
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.service.smb",
			[$moduleMngr->getModule("zeroconf"), "setDirty"]);
		$dispatcher->addListener(
			OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.service.smb.share",
			[$moduleMngr->getModule("zeroconf"), "setDirty"]);
		// If NetBIOS is enabled, changing the hostname requires a restart
		// of the `nmbd.service` systemd unit.
		$dispatcher->addListener(
			OMV_NOTIFY_MODIFY,
			"org.openmediavault.conf.system.network.dns",
			[$this, "setDirty"]);
	}
}
