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

class UserMgmtModule extends OMVModuleAbstract implements OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "usermgmt";
	}

	/**
	 * Add an user.
	 */
	public function addUser($object) {
		global $xmlConfig;
		// Prepare command arguments
		$cmdparam = sprintf("--no-user-group --gid %s --password %s ".
		  "--shell %s", $GLOBALS['OMV_USERMGMT_DEFAULT_GROUP'],
		  escapeshellarg(crypt($object['password'])),
		  escapeshellarg($object['shell']));
		if (!empty($object['comment'])) {
			$cmdparam .= sprintf(" --comment %s", escapeshellarg(
			  $object['comment']));
		}
		if (!empty($object['uid'])) {
			$cmdparam .= sprintf(" --uid %d", intval($object['uid']));
		}
		if (!empty($object['groups'])) {
			$cmdparam .= sprintf(" --groups %s", $object['groups']);
		}
		// If a shared folder is set then use this as home directory,
		// otherwise let the system create a home directory as configured
		// (see 'useradd' man page).
		if (!empty($object['sharedfolderref'])) {
			// Get the shared folder object
			$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
			  $object['sharedfolderref']);
			$sfobject = $xmlConfig->get($xpath);
			if (is_null($sfobject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Get the mount point configuration object
			$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
			  $sfobject['mntentref']);
			$meobject = $xmlConfig->get($xpath);
			if (is_null($meobject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Build the shared folder (home directory) path.
			$dirpath = sprintf("%s/%s", $meobject['dir'],
			  $sfobject['reldirpath']);
			$cmdparam .= sprintf(" --home %s", escapeshellarg($dirpath));
		} else {
			$cmdparam .= " --create-home";
		}
		$cmd = sprintf("sudo useradd %s %s 2>&1", $cmdparam, escapeshellarg(
		  $object['name']));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Modify an user.
	 */
	public function modifyUser($object) {
		global $xmlConfig;
		// Prepare command arguments
		$cmdparam = sprintf("--password %s --shell %s", escapeshellarg(crypt(
		  $object['password'])), escapeshellarg($object['shell']));
		if (!empty($object['groups'])) {
			$cmdparam .= sprintf(" --groups %s", $object['groups']);
		}
		if (!empty($object['comment'])) {
			$cmdparam .= sprintf(" --comment %s", escapeshellarg(
			  $object['comment']));
		}
		if (!empty($object['sharedfolderref'])) {
			// Get the shared folder object
			$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
			  $object['sharedfolderref']);
			$sfobject = $xmlConfig->get($xpath);
			if (is_null($sfobject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Get the mount point configuration object
			$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
			  $sfobject['mntentref']);
			$meobject = $xmlConfig->get($xpath);
			if (is_null($meobject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Build the shared folder (home directory) path.
			$dirpath = sprintf("%s/%s", $meobject['dir'],
			  $sfobject['reldirpath']);
			$cmdparam .= sprintf(" --home %s", escapeshellarg($dirpath));
		} else {
			// Set home directory to default value /home/<name>, the
			// directory must not exist in this case (see 'useradd' man page).
			$cmdparam .= sprintf(" --home %s", escapeshellarg(sprintf(
			  "/home/%s", $object['name'])));
		}
		// Modify user
		$cmd = sprintf("sudo usermod %s %s 2>&1", $cmdparam,
		  escapeshellarg($object['name']));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Delete an user.
	 */
	public function deleteUser($object) {
		$cmd = sprintf("sudo userdel --force %s 2>&1", escapeshellarg(
		  $object['name']));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Add a group.
	 */
	public function addGroup($object) {
		$cmdparam = "";
		if (!empty($object['gid'])) {
			$cmdparam .= sprintf(" -g %d", intval($object['gid']));
		}
		$cmd = sprintf("sudo groupadd %s %s 2>&1", $cmdparam,
		  escapeshellarg($object['name']));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Delete a group.
	 */
	public function deleteGroup($object) {
		$cmd = sprintf("sudo delgroup --only-if-empty %s 2>&1",
		  escapeshellarg($object['name']));
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
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.usermanagement.users.user",
		  array($this, "addUser"), 1);
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.usermanagement.users.user",
		  array($this, "modifyUser"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.usermanagement.users.user",
		  array($this, "deleteUser"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.usermanagement.groups.group",
		  array($this, "addGroup"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.usermanagement.groups.group",
		  array($this, "deleteGroup"));
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new UserMgmtModule());
?>
