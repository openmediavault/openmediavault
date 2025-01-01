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
namespace Engined\Rpc;

require_once("openmediavault/functions.inc");

class UserMgmt extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "UserMgmt";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("authUser");
		$this->registerMethod("enumerateSystemUsers");
		$this->registerMethod("enumerateUsers");
		$this->registerMethod("enumerateAllUsers");
		$this->registerMethod("enumerateSystemGroups");
		$this->registerMethod("enumerateGroups");
		$this->registerMethod("enumerateAllGroups");
		$this->registerMethod("getUserList");
		$this->registerMethod("getUser");
		$this->registerMethod("getUserByContext");
		$this->registerMethod("setUser");
		$this->registerMethod("setUserByContext");
		$this->registerMethod("deleteUser");
		$this->registerMethod("importUsers");
		$this->registerMethod("getGroupList");
		$this->registerMethod("getGroup");
		$this->registerMethod("setGroup");
		$this->registerMethod("deleteGroup");
		$this->registerMethod("importGroups");
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
		$this->registerMethod("setPasswordByContext");
	}

	/**
	 * Authenticate the given user.
	 * @param params The method parameters containing the following fields:
	 *   \em username The name of the user.
	 *   \em password The password.
	 * @param context The context of the caller.
	 * @return An array containing the fields \em authenticated which is TRUE
	 *   if authentication was successful, otherwise FALSE. The name of the
	 *   user is in \em username.
	 */
	final public function authUser($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.usermgmt.authuser");
		// Authenticate the user.
		$authenticated = FALSE;
		$user = new \OMV\System\User($params['username']);
		$permissions = [];
		if ($user->exists()) {
			if (FALSE === ($authenticated = $user->authenticate(
					$params['password']))) {
				goto finish;
			}
			// Deny access for inactive user accounts.
			if (TRUE === $user->getInactive()) {
				$authenticated = FALSE;
				goto finish;
			}
			// Is the user a WebUI administrator?
			// Note, this check must be carried out before checking
			// whether the user is a system account.
			if (in_array(\OMV\Environment::get("OMV_WEBGUI_ADMINGROUP_NAME"),
					$user->getGroups())) {
				$permissions['role'] = "admin";
				goto finish;
			}
			// Deny access for 'system' user accounts.
			if (TRUE === $user->isSystemAccount()) {
				$authenticated = FALSE;
				goto finish;
			}
			$permissions['role'] = "user";
		}
finish:
		return [
			"authenticated" => $authenticated,
			"username" => $params['username'],
			"permissions" => $permissions
		];
	}

	/**
	 * Return info about an user by their name or UID.
	 * @param id The user identifier; name or UID.
	 * @return Returns an associative array with the user information
	 *   or FALSE if the user does not exist.
	 */
	private function getUserInfo($id) {
		$user = new \OMV\System\User($id);
		if (!$user->exists())
			return FALSE;
		return [
			"name" => $user->getName(),
			"uid" => $user->getUid(),
			"gid" => $user->getGid(),
			"comment" => $user->getGecos(),
			"dir" => $user->getHomeDirectory(),
			"shell" => $user->getShell(),
			"lastchanged" => $user->getLastChanged(),
			"minimum" => $user->getMinimum(),
			"maximum" => $user->getMaximum(),
			"warn" => $user->getWarn(),
			"inactive" => $user->getInactive(),
			"expire" => $user->getExpire(),
			"reserved" => $user->getReserved(),
			"groups" => $user->getGroups(),
			"system" => $user->isSystemAccount()
		];
	}

	/**
	 * Enumerate users.
	 * @param type The user type, e.g. system, normal or all.
	 * @return An array containing user objects with following fields:
	 *   name, UID, GID, comment, home directory, and shell program,
	 *   last changed, minimum, maximum, warn, inactive, expire and
	 *   reserved.
	 */
	private function enumerateUsersByType($type) {
		// Get list of user names.
		$users = \OMV\System\User::getUsers();
		// Iterate through all users to filter the requested ones.
		$result = [];
		foreach ($users as $userk => $userv) {
			// Get the user information about the user.
			if (FALSE === ($userInfo = $this->getUserInfo($userv)))
				continue;
			// Append user 'root'?
			if ((0 == strcasecmp("root", $userInfo['name'])) &&
			  (FALSE == \OMV\Environment::getBoolean(
			  "OMV_USERMGMT_ENUMERATE_USER_ROOT", TRUE)))
				continue;
			// Check if the current user is requested.
			$append = FALSE;
			switch ($type) {
			case "system":
				$append = $userInfo['system'];
				break;
			case "normal":
				$append = !$userInfo['system'];
				break;
			case "all":
				$append = TRUE;
				break;
			}
			if (FALSE === $append)
				continue;
			$result[] = $userInfo;
		}
		return $result;
	}

	/**
	 * Get the list of system users.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A list of user objects.
	 */
	public function enumerateSystemUsers($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get system users.
		return $this->enumerateUsersByType("system");
	}

	/**
	 * Get the list of non-system users.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A list of user objects.
	 */
	public function enumerateUsers($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get non-system users.
		return $this->enumerateUsersByType("normal");
	}

	/**
	 * Get the list of all users.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A list of user objects.
	 */
	public function enumerateAllUsers($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get all users.
		return $this->enumerateUsersByType("all");
	}

	/**
	 * Return info about a group by their name or GID.
	 * @param id The group identifier; name or GID.
	 * @return Returns an associative array with the group information
	 *   or FALSE if the group does not exist.
	 */
	private function getGroupInfo($id) {
		$group = new \OMV\System\Group($id);
		if (!$group->exists())
			return FALSE;
		return [
			"name" => $group->getName(),
			"gid" => $group->getGid(),
			"members" => $group->getMembers(),
			"system" => $group->isSystemAccount()
		];
	}

	/**
	 * Enumerate groups.
	 * @param type The group type, e.g. system, normal or all.
	 * @return An array containing user objects with following fields:
	 *   name, GID and group members (array of user names).
	 */
	private function enumerateGroupsByType($type) {
		// Get list of group names.
		$groups = \OMV\System\Group::getGroups();
		// Get group information.
		$result = [];
		foreach ($groups as $groupk => $groupv) {
			// Get the user information about the group.
			if (FALSE === ($groupInfo = $this->getGroupInfo($groupv)))
				continue;
			// Check if the current group is requested.
			$append = FALSE;
			switch ($type) {
			case "system":
				$append = $groupInfo['system'];
				break;
			case "normal":
				$append = !$groupInfo['system'];
				break;
			case "all":
				$append = TRUE;
				break;
			}
			if (FALSE === $append)
				continue;
			$result[] = $groupInfo;
		}
		return $result;
	}

	/**
	 * Get the list of system groups.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A list of group objects.
	 */
	public function enumerateSystemGroups($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get system groups.
		return $this->enumerateGroupsByType("system");
	}

	/**
	 * Get the list of non-system groups.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A list of group objects.
	 */
	public function enumerateGroups($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get non-system groups.
		return $this->enumerateGroupsByType("normal");
	}

	/**
	 * Get the list of all groups.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A list of group objects.
	 */
	public function enumerateAllGroups($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get all groups.
		return $this->enumerateGroupsByType("all");
	}

	/**
	 * Get list of users (except system users).
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	function getUserList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the list of non-system user.
		$users = $this->enumerateUsersByType("normal");
		// Process users and append additional information stored in
		// the database.
		foreach ($users as $userk => &$userv) {
			// Set the defaults of the additional information.
			$userv = array_merge($userv, [
				"email" => "",
				"disallowusermod" => FALSE,
				"sshpubkeys" => []
			]);
			// Get additional information stored in database.
			$db = \OMV\Config\Database::getInstance();
			$objects = $db->getByFilter("conf.system.usermngmnt.user", [
				"operator" => "stringEquals",
				"arg0" => "name",
				"arg1" => $userv['name']
			]);
			if (0 < count($objects)) {
				// Get the user configuration object. Due the fact that a user
				// name is unique, we can simply use the first found object.
				$object = $objects[0];
				// Append additional information.
				$userv['email'] = $object->get("email");
				$userv['disallowusermod'] = $object->get("disallowusermod");
				if (FALSE === $object->isEmpty("sshpubkeys"))
					$userv['sshpubkeys'] = $object->get("sshpubkeys.sshpubkey");
			}
		}
		// Filter the result.
		return $this->applyFilter($users, $params['start'],
			$params['limit'], $params['sortfield'], $params['sortdir'],
			$params['search']);
	}

	/**
	 * Get an user configuration object.
	 * @param params An array containing the following fields:
	 *   \em name The name of the user.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function getUser($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.usermgmt.getuser");
		// Ensure the user exists.
		$user = new \OMV\System\User($params['name']);
		$user->assertExists();
		// Get the user information.
		$result = $this->getUserInfo($user->getName());
		// Set the defaults of the additional information.
		$result = array_merge($result, [
			"email" => "",
			"disallowusermod" => FALSE,
			"sshpubkeys" => []
		]);
		// Get additional user information stored in database.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.system.usermngmnt.user", [
			"operator" => "stringEquals",
			"arg0" => "name",
			"arg1" => $user->getName()
		]);
		if (0 < count($objects)) {
			// Get the user configuration object. Due the fact that a user
			// name is unique, we can simply use the first found object.
			$object = $objects[0];
			// Append additional information.
			$result['email'] = $object->get("email");
			$result['disallowusermod'] = $object->get("disallowusermod");
			if (FALSE === $object->isEmpty("sshpubkeys"))
				$result['sshpubkeys'] = $object->get("sshpubkeys.sshpubkey");
		}
		return $result;
	}

	/**
	 * Get the user configuration from the current context user. The
	 * returned object contains only a limited number of user information.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The result object contains the fields \em name, \em comment,
	 *   \em email, \em sshpubkeys and the internal field \em _readonly.
	 */
	function getUserByContext($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_EVERYONE
		]);
		// Get the user configuration object.
		$object = $this->callMethod("getUser", [
			"name" => $context['username']
		], $this->getAdminContext());
		// Prepare the result object.
		return [
			"name" => $object['name'],
			"comment" => $object['comment'],
			"email" => $object['email'],
			"sshpubkeys" => $object['sshpubkeys'],
			// Hijack the '_readonly' flag to set the form to read-only.
			"_readonly" => $object['disallowusermod']
		];
	}

	/**
	 * Set (add/update) a non-system user account.
	 * @param params An array containing the following fields:
	 *   \em name The name of the user.
	 *   \em uid The user ID. This field is optional.
	 *   \em groups A list of groups which the user is a member of as an
	 *     array of strings.
	 *   \em shell The name of the users login shell. This field is optional.
	 *   \em password The plain password to use.
	 *   \em email The users email address.
	 *   \em comment Any text string. This field is optional.
	 *   \em disallowusermod Set to TRUE to disallow the user from modifying
	 *     their account from within the web interface.
	 *   \em sshpubkeys The users SSH public keys.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setUser($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.usermgmt.setuser");
		// Check if the given user is a system account. Abort this attempt.
		$user = new \OMV\System\User($params['name']);
		if ($user->exists() && $user->isSystemAccount()) {
			throw new \OMV\Exception(
				"Unauthorized attempt to modify the system user account '%s'",
				$params['name']);
		}
		// Convert all public SSH keys to RFC 4716 format and make sure
		// that they are all unique.
		foreach ($params['sshpubkeys'] as $sshPubKeyk => &$sshPubKeyv) {
			$pubKey = new \OMV\Ssh\PublicKey($sshPubKeyv);
			$sshPubKeyv = $pubKey->toRfc4716();
		}
		$params['sshpubkeys'] = array_values(array_unique(
			$params['sshpubkeys']));
		// Try to get existing configuration object.
		$filter = [
			"operator" => "stringEquals",
			"arg0" => "name",
			"arg1" => $params['name']
		];
		$db = \OMV\Config\Database::getInstance();
		// Does the user already exist in the database?
		$oldObject = NULL;
		if ($db->exists("conf.system.usermngmnt.user", $filter)) {
			$notifyType = OMV_NOTIFY_MODIFY;
			// Get the user configuration object. Since the name of a user
			// is unique, we can simply use the first object found.
			$object = $oldObject = $db->getByFilter(
				"conf.system.usermngmnt.user", $filter, 1);
			$object->set("email", $params['email']);
			$object->set("disallowusermod", $params['disallowusermod']);
			$object->set("email", $params['email']);
			$object->set("sshpubkeys", [
				"sshpubkey" => $params['sshpubkeys']
			]);
		} else {
			$notifyType = OMV_NOTIFY_CREATE;
			// Create a new user configuration object.
			$object = new \OMV\Config\ConfigObject(
				"conf.system.usermngmnt.user");
			$object->setNew();
			$object->set("name", $params['name']);
			$object->set("email", $params['email']);
			$object->set("disallowusermod", $params['disallowusermod']);
			$object->set("email", $params['email']);
			$object->set("sshpubkeys", [
				"sshpubkey" => $params['sshpubkeys']
			]);
		}
		$db->set($object, TRUE);
		// Prepare command arguments. Note, a user is always in the
		// group 'users'.
		$cmdArgs = [];
		$cmdArgs[] = "--gid";
		$cmdArgs[] = escapeshellarg(\OMV\Environment::get(
			"OMV_USERMGMT_DEFAULT_GROUP"));
		if (array_key_exists("shell", $params) &&
				!empty($params['shell'])) {
			// Make sure the shell exists.
			if (!file_exists($params['shell'])) {
				throw new \OMV\Exception("The shell '%s' does not exist.",
					$params['shell']);
			}
			$cmdArgs[] = "--shell";
			$cmdArgs[] = escapeshellarg($params['shell']);
		}
		if (array_key_exists("comment", $params)) {
			$cmdArgs[] = "--comment";
			$cmdArgs[] = escapeshellarg($params['comment']);
		}
		if (array_key_exists("groups", $params) &&
				!empty($params['groups'])) {
			// Make sure the specified groups exists. Note, the groups
			// are handled as group names and NOT as GIDs. This is the
			// only way to support numeric group names.
			// https://manpages.debian.org/buster/passwd/groupadd.8.en.html#CAVEATS
			foreach ($params['groups'] as $groupk => $groupv) {
				$group = new \OMV\System\Group($groupv);
				$group->assertExists();
			}
			$cmdArgs[] = "--groups";
			$cmdArgs[] = escapeshellarg(implode(",", $params['groups']));
		}
		// Does the user already exist?
		if (FALSE === $user->exists()) {
			// Get user management settings.
			$hdsobject = $db->get("conf.system.usermngmnt.homedir");
			// Append additional arguments.
			if (array_key_exists("uid", $params)) {
				$cmdArgs[] = "--uid";
				$cmdArgs[] = $params['uid'];
			}
			if (TRUE === $hdsobject->get("enable")) {
				// Get the absolute shared folder path.
				$sfpath = \OMV\Rpc\Rpc::call("ShareMgmt", "getPath",
					array("uuid" => $hdsobject->get("sharedfolderref")),
					$context);
				$cmdArgs[] = "--create-home";
				$cmdArgs[] = "--home";
				$cmdArgs[] = escapeshellarg(build_path(DIRECTORY_SEPARATOR,
					$sfpath, $params['name']));
			}
			$cmdArgs[] = escapeshellarg($params['name']);
			// Create the new user.
			$cmd = new \OMV\System\Process("useradd", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute();
		} else {
			// Append additional arguments.
			$cmdArgs[] = escapeshellarg($params['name']);
			// Modify the existing user.
			$cmd = new \OMV\System\Process("usermod", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute();
		}
		// Update the password.
		if (array_key_exists("password", $params) &&
				!empty($params['password'])) {
			$user = new \OMV\System\User($params['name']);
			$user->assertExists();
			$user->changePassword($params['password']);
		}
		// Append some more information to the notification object.
		$objectAssoc = array_merge($object->getAssoc(), $params);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify($notifyType,
			"org.openmediavault.conf.system.usermngmnt.user",
			$objectAssoc,
			!is_null($oldObject) ? $oldObject->getAssoc() : NULL);
		// Return the configuration object.
		return $objectAssoc;
	}

	/**
	 * Set an user configuration object of the current context user.
	 * @param params An array containing the following fields:
	 *   \em password The password. Empty passwords are ignored.
	 *   \em email The email address.
	 *   \em comment The comment.
	 * @param context The context of the caller.
	 * @return void
	 */
	function setUserByContext($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_EVERYONE
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.usermgmt.setuserbycontext");
		// Get the user configuration object.
		$object = $this->callMethod("getUser", [
			"name" => $context['username']
		], $this->getAdminContext());
		// Make sure the user is allowed to modify their account.
		if (TRUE === $object['disallowusermod']) {
			throw new \OMV\Exception(
				"Unauthorized attempt to modify the user account '%s'",
				$object['name']);
		}
		// Update the user configuration.
		return $this->callMethod("setUser", [
			"name" => $object['name'],
			"uid" => $object['uid'],
			"groups" => $object['groups'],
			"shell" => $object['shell'],
			"password" => array_value($params, 'password', ''),
			"email" => $params['email'],
			"comment" => $params['comment'],
			"disallowusermod" => $object['disallowusermod'],
			"sshpubkeys" => $object['sshpubkeys']
		], $this->getAdminContext());
	}

	/**
	 * Delete a user.
	 * @param params An array containing the following fields:
	 *   \em name The name of the user to delete.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	function deleteUser($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.usermgmt.deleteuser");
		// Get the user configuration object (required for the notification
		// event).
		$object = $this->callMethod("getUser", array(
			  "name" => $params['name']
		  ), $this->getAdminContext());
		// Delete the user.
		$cmdArgs = [];
		$cmdArgs[] = "--force";
		$cmdArgs[] = escapeshellarg($params['name']);
		$cmd = new \OMV\System\Process("userdel", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Delete additional user information stored in database.
		$db = \OMV\Config\Database::getInstance();
		$db->deleteByFilter("conf.system.usermngmnt.user", [
			"operator" => "stringEquals",
			"arg0" => "name",
			"arg1" => $params['name']
		]);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.conf.system.usermngmnt.user", $object);
		// Return the deleted configuration object.
		return $object;
	}

	/**
	 * Import a list of users.
	 * @param params An array containing the following fields:
	 *   \em csv The user values, e.g.:
	 *   name;uid;comment;email;password;group,group,...;disallowusermod
	 *   test1;;comment;test1@xyz.com;foobarpwd1;adm;1
	 *   test2;1200;comment2;test2@xyz.com;foobarpwd2;adm,audio,www-data;0
	 *   test3;;comment3;test3@xyz.com;foobarpwd3;ssh;1
	 * @param context The context of the caller.
	 * @return void
	 */
	function importUsers($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.usermgmt.importuser");
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params, $context) {
			// Import users.
			$objects = [];
			$csv = explode("\n", $params['csv']);
			foreach ($csv as $linek => $linev) {
				$linev = trim($linev);
				# Skip comments.
				if (empty($linev) || ($linev[0] === "#"))
					continue;
				# Parse line.
				# <name>;<uid>;<comment>;<email>;<password>;<shell>;<group,group,...>;<disallowusermod>
				if (1 !== preg_match("/^([^;]+);(\d*);([^;]*);".
						"([^;]*);([^;]*);([^;]*);([^;]*);".
						"(true|1|yes|y|on|false|0|no|n|off)$/i",
						$linev, $matches)) {
					throw new \OMV\Exception("Invalid line: %s", $linev);
				}
				$object = [
					"name" => $matches[1],
					"comment" => $matches[3],
					"email" => $matches[4],
					"password" => $matches[5],
					"groups" => !empty($matches[7]) ?
						explode(",", $matches[7]) : [],
					"disallowusermod" => boolvalEx($matches[8]),
					"shell" => !empty($matches[6]) ?
						$matches[6] : "/bin/sh",
					"sshpubkeys" => []
				];
				if (!empty($matches[2]) && is_numeric($matches[2]))
					$object['uid'] = intval($matches[2]);
				// Do some checks:
				// - Check if an user with the given name already exists.
				$user = new \OMV\System\User($object['name']);
				$user->assertNotExists();
				// - Check if an user with the given UID already exists.
				if (array_key_exists("uid", $object)) {
					$user = new \OMV\System\User($object['uid']);
					$user->assertNotExists();
				}
				$objects[] = $object;
			}
			// Finally create the users.
			foreach ($objects as $objectk => $objectv) {
				$this->callMethod("setUser", $objectv, $context);
			}
			return "";
		});
	}

	/**
	 * Get list of groups (except system groups).
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	function getGroupList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the list of non-system groups.
		$groups = $this->enumerateGroupsByType("normal");
		foreach ($groups as $groupk => &$groupv) {
			// Get additional information stored in database.
			$db = \OMV\Config\Database::getInstance();
			$objects = $db->getByFilter("conf.system.usermngmnt.group", [
				"operator" => "stringEquals",
				"arg0" => "name",
				"arg1" => $groupv['name']
			]);
			if (0 < count($objects)) {
				// Get the group configuration object. Due the fact that a
				// group name is unique, we can simply use the first found
				// object.
				$object = $objects[0];
				// Append additional information.
				$groupv['comment'] = $object->get("comment");
			}
		}
		// Filter result.
		return $this->applyFilter($groups, $params['start'],
			$params['limit'], $params['sortfield'], $params['sortdir'],
			$params['search']);
	}

	/**
	 * Get a group.
	 * @param params An array containing the following fields:
	 *   \em name The name of the group.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function getGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.usermgmt.getgroup");
		// Get the group information.
		$group = new \OMV\System\Group($params['name']);
		$group->assertExists();
		// Get the group information.
		$result = $this->getGroupInfo($group->getName());
		// Get additional group information stored in database.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.system.usermngmnt.group", [
			"operator" => "stringEquals",
			"arg0" => "name",
			"arg1" => $group->getName()
		]);
		if (0 < count($objects)) {
			// Get the group configuration object. Due the fact that a group
			// name is unique, we can simply use the first found object.
			$object = $objects[0];
			// Append additional information.
			$result['comment'] = $object->get("comment");
		}
		return $result;
	}

	/**
	 * Set (add/update) a non-system group account.
	 * @param params An array containing the following fields:
	 *   \em name The name of the group.
	 *   \em gid The group ID. This field is optional.
	 *   \em comment Any text string.
	 *   \em members The group members as an array of user names.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.usermgmt.setgroup");
		// Check if the given group is a system account. Abort this attempt.
		$group = new \OMV\System\Group($params['name']);
		if ($group->exists() && $group->isSystemAccount()) {
			throw new \OMV\Exception(
			  "Unauthorized attempt to modify the system group account '%s'",
			  $params['name']);
		}
		// Try to get existing configuration object.
		$filter = [
			"operator" => "stringEquals",
			"arg0" => "name",
			"arg1" => $params['name']
		];
		$db = \OMV\Config\Database::getInstance();
		// Does the group already exist in the database?
		$oldObject = NULL;
		if ($db->exists("conf.system.usermngmnt.group", $filter)) {
			$notifyType = OMV_NOTIFY_MODIFY;
			// Get the group configuration object. Since the name of a group
			// is unique, we can simply use the first object found.
			$object = $oldObject = $db->getByFilter(
				"conf.system.usermngmnt.group", $filter, 1);
			$object->set("comment", $params['comment']);
		} else {
			$notifyType = OMV_NOTIFY_CREATE;
			// Create a new group configuration object.
			$object = new \OMV\Config\ConfigObject(
				"conf.system.usermngmnt.group");
			$object->setNew();
			$object->set("name", $params['name']);
			$object->set("comment", $params['comment']);
		}
		$db->set($object);
		// Append some more information to the notification object.
		$objectAssoc = $object->getAssoc();
		// Does the group already exist?
		if (FALSE === $group->exists()) {
			// Prepare command arguments.
			$cmdArgs = [];
			if (array_key_exists("gid", $params)) {
				$cmdArgs[] = "--gid";
				$cmdArgs[] = $params['gid'];
				// Append additional fields to configuration object for
				// the notification event.
				$objectAssoc['gid'] = $params['gid'];
			}
			$cmdArgs[] = escapeshellarg($params['name']);
			// Create the new group.
			$cmd = new \OMV\System\Process("groupadd", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute();
		} else {
			// Nothing to do here.
		}
		// Process members.
		if (array_key_exists("members", $params)) {
			// Append additional fields to configuration object for
			// the notification event.
			$objectAssoc['members'] = $params['members'];
			// Append the given members to the group.
			$cmdArgs = [];
			$cmdArgs[] = "--members";
			$cmdArgs[] = escapeshellarg(implode(",", $params['members']));
			$cmdArgs[] = escapeshellarg($params['name']);

			$cmd = new \OMV\System\Process("gpasswd", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute();
		}
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify($notifyType,
			"org.openmediavault.conf.system.usermngmnt.group",
			$objectAssoc,
			!is_null($oldObject) ? $oldObject->getAssoc() : NULL);
		// Return the configuration object.
		return $objectAssoc;
	}

	/**
	 * Delete a group.
	 * @param params An array containing the following fields:
	 *   \em name The name of the group to delete.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	function deleteGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.usermgmt.deletegroup");
		// Get the group configuration object (required for the notification
		// event).
		$object = $this->callMethod("getGroup", [
			"name" => $params['name']
		], $context);
		// Delete the group.
		$cmdArgs = [];
		$cmdArgs[] = "--only-if-empty";
		$cmdArgs[] = escapeshellarg($params['name']);
		$cmd = new \OMV\System\Process("delgroup", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Delete configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->deleteByFilter("conf.system.usermngmnt.group", [
			"operator" => "stringEquals",
			"arg0" => "name",
			"arg1" => $params['name']
		]);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.conf.system.usermngmnt.group", $object);
		// Return the deleted configuration object.
		return $object;
	}

	/**
	 * Import a list of groups.
	 * @param params An array containing the following fields:
	 *   \em csv The group values, e.g.:
	 *   name;gid;comment
	 *   grp1;;comment1
	 *   grp2;1200;comment2
	 * @param context The context of the caller.
	 * @return void
	 */
	function importGroups($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.usermgmt.importgroup");
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params, $context) {
			// Import groups.
			$objects = [];
			$csv = explode("\n", $params['csv']);
			foreach ($csv as $linek => $linev) {
				$linev = trim($linev);
				# Skip comments.
				if(empty($linev) || ($linev[0] === "#"))
					continue;
				# Parse line.
				if (1 !== preg_match("/^([^;]+);(\d*);([^;]*)$/i", $linev,
				  $matches)) {
					throw new \OMV\Exception("Invalid line: %s", $linev);
				}
				$object = [
					"name" => $matches[1],
					"comment" => $matches[3],
					"members" => []
				];
				if (!empty($matches[2]) && is_numeric($matches[2]))
					$object['gid'] = intval($matches[2]);
				$objects[] = $object;
			}
			// Check if the users does not exist until now.
			foreach ($objects as $objectk => $objectv) {
				// Check if an group with the given name already exists.
				$group = new \OMV\System\Group($objectv['name']);
				$group->assertNotExists();
				// Check if an group with the given GID already exists.
				if (array_key_exists("gid", $object)) {
					$group = new \OMV\System\Group($objectv['gid']);
					$group->assertNotExists();
				}
			}
			// Finally create the groups.
			foreach ($objects as $objectk => $objectv)
				$this->callMethod("setGroup", $objectv, $context);
			return "";
		});
	}

	/**
	 * Get user management settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function getSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		return $db->getAssoc("conf.system.usermngmnt.homedir");
	}

	/**
	 * Set user management settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.usermgmt.setsettings");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject(
		  "conf.system.usermngmnt.homedir");
		$object->setAssoc($params);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Build the home directory path.
		$sfPath = "";
		if (TRUE === $object->get("enable")) {
			// Get the absolute shared folder path.
			$sfPath = \OMV\Rpc\Rpc::call("ShareMgmt", "getPath", [
				"uuid" => $object->get("sharedfolderref")
			], $context);
		}
		// Update the non-system user.
		$users = $this->enumerateUsersByType("normal");
		$ld = \OMV\System\System::getLoginDefs();
		foreach ($users as $userk => $userv) {
			// Only process users that are in group 'users'.
			$defaultGroup = \OMV\Environment::get(
				"OMV_USERMGMT_DEFAULT_GROUP", "users");
			if (!in_array($defaultGroup, $userv['groups'])) {
				continue;
			}
			$cmdArgs = [];
			if (!empty($sfPath)) {
				$homeDirPath = build_path(DIRECTORY_SEPARATOR, $sfPath,
				  $userv['name']);
				// Set new home directory. Move the content from the old to
				// the new home directory if this does not exist.
				// Note, the usermod command throws an error when calling
				// with --move-home and the home directory already exists.
				if (!file_exists($homeDirPath)) {
					$cmdArgs[] = "--move-home";
				}
				$cmdArgs[] = "--home";
				$cmdArgs[] = escapeshellarg($homeDirPath);
			} else {
				// Skip the user if they do not yet have a home directory.
				if (empty($userv['dir'])) {
					continue;
				}
				// Unset home directory.
				// See https://unix.stackexchange.com/a/617552
				//
				// ToDo:
				// Make use of an empty dir path again when the fix at
				// https://github.com/shadow-maint/shadow/pull/1133 is
				// available in Debian.
				$cmdArgs[] = "--home";
				$cmdArgs[] = array_value($ld, "NONEXISTENT", "/nonexistent");
			}
			$cmdArgs[] = escapeshellarg($userv['name']);
			$cmd = new \OMV\System\Process("usermod", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute();
			// Create the home directory if it does not exist until now.
			if (!empty($sfPath)) {
				$homeDirPath = build_path(DIRECTORY_SEPARATOR, $sfPath,
				  $userv['name']);
				if (!file_exists($homeDirPath)) {
					if (!mkdir($homeDirPath, 0700, TRUE)) {
						throw new \OMV\Exception(
						  "Failed to create home directory '%s': %s",
						  $homeDirPath, last_error_msg());
					}
					if (!chown($homeDirPath, $userv['name'])) {
						throw new \OMV\Exception(
						  "Failed to change owner of the directory '%s' ".
						  "to '%s': %s", $homeDirPath, $userv['name'],
						  last_error_msg());
					}
					if (!chgrp($homeDirPath, \OMV\Environment::get(
					  "OMV_USERMGMT_DEFAULT_GROUP"))) {
						throw new \OMV\Exception(
						  "Failed to change group of the directory '%s' ".
						  "to '%s': %s", $homeDirPath,
						  \OMV\Environment::get("OMV_USERMGMT_DEFAULT_GROUP"),
						  last_error_msg());
					}
				}
			}
		}
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.system.usermanagement.homedirectory",
		  $object->getAssoc());
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Set the password of the current context user.
	 * @param params An array containing the following fields:
	 *   \em password The new password.
	 * @param context The context of the caller.
	 * @return void
	 */
	function setPasswordByContext($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_EVERYONE
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
			"rpc.usermgmt.setpasswordbycontext");
		$user = new \OMV\System\User($context['username']);
		$user->assertExists();
		$user->changePassword($params['password']);
		// Notify configuration changes if the given user exists in the
		// database. System users like 'admin' are ignored.
		$db = \OMV\Config\Database::getInstance();
		$filter = [
			"operator" => "stringEquals",
			"arg0" => "name",
			"arg1" => $user->getName()
		];
		if (TRUE === $db->exists("conf.system.usermngmnt.user", $filter)) {
			$object = $db->getByFilter("conf.system.usermngmnt.user",
				$filter, 1);
			// Append some more information to the notification object.
			$objectAssoc = array_merge($object->getAssoc(), [
				"password" => $params['password']
			]);
			// Notify configuration changes.
			$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
			$dispatcher->notify(OMV_NOTIFY_MODIFY,
				"org.openmediavault.conf.system.usermngmnt.user",
				$objectAssoc, $object->getAssoc());
		}
	}
}
