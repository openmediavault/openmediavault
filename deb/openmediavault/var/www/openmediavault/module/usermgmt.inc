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
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Get user management settings
		$xpath = "//system/usermanagement/homedirectory";
		$hdsobject = $xmlConfig->get($xpath);
		if (is_null($hdsobject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Create the command line. Note, a user is always in group 'users'.
		$cmdargs = array(
			sprintf("--gid %s", $GLOBALS['OMV_USERMGMT_DEFAULT_GROUP']),
			sprintf("--password %s", escapeshellarg(crypt(
			  $object['password'])))
		);
		if (array_key_exists("uid", $object)) {
			$cmdargs[] = sprintf("--uid %d", $object['uid']);
		}
		if (array_key_exists("shell", $object)) {
			$cmdargs[] = sprintf("--shell %s", escapeshellarg(
			  $object['shell']));
		}
		if (array_key_exists("comment", $object)) {
			$cmdargs[] = sprintf("--comment %s", escapeshellarg(
			  $object['comment']));
		}
		if (array_key_exists("groups", $object)) {
			$cmdargs[] = sprintf("--groups %s", escapeshellarg(
			  implode(",", $object['groups'])));
		}
		if (TRUE === boolval($hdsobject['enable'])) {
			// Get the absolute shared folder path.
			$sfpath = OMVRpc::exec("ShareMgmt", "getPath", array(
			  "uuid" => $hdsobject['sharedfolderref']));
			// Append command argument
			$cmdargs[] = "--create-home";
			$cmdargs[] = sprintf("--home %s", escapeshellarg(sprintf("%s/%s",
			  $sfpath, $object['name'])));
		}
		$cmd = sprintf("sudo useradd %s %s 2>&1", implode(" ", $cmdargs),
		  escapeshellarg($object['name']));
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
//		$this->debug(var_export(func_get_args(), TRUE));
		// Create the command line. Note, a user is always in group 'users'.
		$cmdargs = array(
			sprintf("--gid %s", $GLOBALS['OMV_USERMGMT_DEFAULT_GROUP'])
		);
		if (array_key_exists("shell", $object)) {
			$cmdargs[] = sprintf("--shell %s", escapeshellarg(
			  $object['shell']));
		}
		if (array_key_exists("comment", $object)) {
			$cmdargs[] = sprintf("--comment %s", escapeshellarg(
			  $object['comment']));
		}
		if (array_key_exists("groups", $object)) {
			$cmdargs[] = sprintf("--groups %s", escapeshellarg(
			  implode(",", $object['groups'])));
		}
		if (array_key_exists("password", $object) && !empty(
		  $object['password'])) {
			$cmdargs[] = sprintf("--password %s", escapeshellarg(crypt(
			  $object['password'])));
		}
		$cmd = sprintf("sudo usermod %s %s 2>&1", implode(" ", $cmdargs),
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
//		$this->debug(var_export(func_get_args(), TRUE));
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
//		$this->debug(var_export(func_get_args(), TRUE));
		$cmdargs = array();
		if (array_key_exists("gid", $object)) {
			$cmdargs[] = sprintf("--gid %d", $object['gid']);
		}
		$cmd = sprintf("sudo groupadd %s %s 2>&1", implode(" ", $cmdargs),
		  escapeshellarg($object['name']));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Process members
		if (array_key_exists("members", $object)) {
			foreach ($object['members'] as $memberk => $memberv) {
				$cmd = sprintf("sudo usermod --append --groups %s %s 2>&1",
				  escapeshellarg($object['name']), escapeshellarg($memberv));
				OMVUtil::exec($cmd, $output, $result);
				if ($result !== 0) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
			}
		}
	}

	/**
	 * Modify an group.
	 */
	public function modifyGroup($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		if (array_key_exists("members", $object)) {
			$cmd = sprintf("sudo gpasswd --members %s %s 2>&1",
			  escapeshellarg(implode(",", $object['members'])),
			  escapeshellarg($object['name']));
			OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
		}
	}

	/**
	 * Delete a group.
	 */
	public function deleteGroup($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$cmd = sprintf("sudo delgroup --only-if-empty %s 2>&1",
		  escapeshellarg($object['name']));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Modify user management settings.
	 */
	public function modifyHomeDirectory($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Build the home directory path
		$homeDirPath = "";
		if (TRUE === boolval($object['enable'])) {
			// Get the absolute shared folder path.
			$homeDirPath = OMVRpc::exec("ShareMgmt", "getPath", array(
			  "uuid" => $object['sharedfolderref']));
		}
		// Update users
		$users = OMVRpc::exec("UserMgmt", "enumerateUsers");
		foreach ($users as $userk => $userv) {
			$cmdargs = array();
			if (!empty($homeDirPath)) {
				// Set new home directory. Move the content from the old to
				// the new directory.
				$cmdargs[] = "--move-home";
				$cmdargs[] = sprintf("--home %s", escapeshellarg(
				  sprintf("%s/%s", $homeDirPath, $userv['name'])));
			} else {
				// Unset home directory.
				$cmdargs[] = sprintf("--home %s", escapeshellarg(
				  $homeDirPath));
			}
			$cmd = sprintf("sudo usermod %s %s 2>&1", implode(" ", $cmdargs),
			  escapeshellarg($userv['name']));
			OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			// Create the home directory if it does not exist until now
			if (!empty($homeDirPath)) {
				$path = sprintf("%s/%s", $homeDirPath, $userv['name']);
				if (!file_exists($path)) {
					$cmd = sprintf("sudo mkdir --mode 755 %s 2>&1",
					  escapeshellarg($path));
					OMVUtil::exec($cmd, $output, $result);
					if ($result !== 0) {
						throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
						  $cmd, implode("\n", $output));
					}
					$cmd = sprintf("sudo chown %s:%s %s", $userv['name'],
					  $GLOBALS['OMV_USERMGMT_DEFAULT_GROUP'],
					  escapeshellarg($path));
					OMVUtil::exec($cmd, $output, $result);
					if ($result !== 0) {
						throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
						  $cmd, implode("\n", $output));
					}
				}
			}
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
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.usermanagement.groups.group",
		  array($this, "modifyGroup"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.usermanagement.groups.group",
		  array($this, "deleteGroup"));
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.usermanagement.homedirectory",
		  array($this, "modifyHomeDirectory"));
    }
}

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new UserMgmtModule());
?>
