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
require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/system.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceUserMgmt extends OMVRpcServiceAbstract {
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
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 */
	final public function authUser($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, array(
			  "type" => "object",
			  "properties" => array(
				  "username" => array("type" => "string"),
				  "password" => array("type" => "string")
			  )
		  ));
		// Get shadow password suite configuration.
		if(FALSE === ($loginDefs = OMVSystem::getLoginDefs())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get shadow password suite configuration."));
		}
		// Authenticate the user.
		$authenticated = FALSE;
		$user = new OMVUser($params['username']);
		if($user->exists()) {
			if(FALSE === ($authenticated = $user->authenticate(
			  $params['password'])))
				goto finish;
			if("admin" === $params['username'])
				goto finish;
			if(TRUE === $user->getInactive()) {
				$authenticated = FALSE;
				goto finish;
			}
			if(!(($user->getUid() >= $loginDefs['UID_MIN']) &&
			  ($user->getUid() <= $loginDefs['UID_MAX']))) {
				$authenticated = FALSE;
				goto finish;
			}
		}
finish:
		return array(
			"authenticated" => $authenticated,
			"username" => $params['username']
		);
	}

	/**
	 * Enumerate users.
	 * @param type The user type, e.g. system, normal or all.
	 * @return An array containing user objects with following fields:
	 *   name, UID, GID, comment, home directory, and shell program,
	 *   last changed, minimum, maximum, warn, inactive, expire and
	 *   reserved.
	 * @throw E_MISC_FAILURE
	 */
	private function enumerateUsersByType($type) {
		// Get shadow password suite configuration.
		if(FALSE === ($loginDefs = OMVSystem::getLoginDefs())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get shadow password suite configuration."));
		}
		// Get list of user names.
		if(FALSE === ($users = OMVUsers::get())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get list of users."));
		}
		// Iterate through all users to filter the requested ones.
		$result = array();
		foreach($users as $userk => $userv) {
			$user = new OMVUser($userv);
			if(!$user->exists())
				continue;
			// Append user 'root'?
			if(0 == strcasecmp("root", $user->getName()) && FALSE == boolval(
			  $GLOBALS['OMV_USERMGMT_ENUMERATE_USER_ROOT']))
				continue;
			// Check if the current user is requested.
			$append = FALSE;
			$system = !(($user->getUid() >= $loginDefs['UID_MIN']) &&
			  ($user->getUid() <= $loginDefs['UID_MAX']));
			switch($type) {
			case "system":
				$append = $system;
				break;
			case "normal":
				$append = !$system;
				break;
			case "all":
				$append = TRUE;
				break;
			}
			if(TRUE === $append) {
				$result[] = array(
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
					"system" => $system
				);
			}
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get all users.
		return $this->enumerateUsersByType("all");
	}

	/**
	 * Enumerate groups.
	 * @param type The group type, e.g. system, normal or all.
	 * @return An array containing user objects with following fields:
	 *   name, GID and group members (array of user names).
	 * @throw E_MISC_FAILURE
	 */
	private function enumerateGroupsByType($type) {
		// Get shadow password suite configuration.
		if(FALSE === ($loginDefs = OMVSystem::getLoginDefs())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get shadow password suite configuration."));
		}
		// Get list of group names.
		if(FALSE === ($groups = OMVUserGroups::get())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get list of groups."));
		}
		// Get group informations.
		$result = array();
		foreach($groups as $groupk => $groupv) {
			$group = new OMVUserGroup($groupv);
			if(!$group->exists())
				continue;
			// Check if the current group is requested
			$append = FALSE;
			$system = !(($group->getGid() >= $loginDefs['GID_MIN']) &&
			  ($group->getGid() <= $loginDefs['GID_MAX']));
			switch($type) {
			case "system":
				$append = $system;
				break;
			case "normal":
				$append = !$system;
				break;
			case "all":
				$append = TRUE;
				break;
			}
			if(TRUE === $append) {
				$result[] = array(
					"name" => $group->getName(),
					"gid" => $group->getGid(),
					"members" => $group->getMembers(),
					"system" => $system
				);
			}
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
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
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "start":{"type":"integer"},
				  "limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
				  "sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
				  "sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
			  }
		  }');
		// Get the list of non-system user.
		$users = $this->enumerateUsersByType("normal");
		// Process users and append additional informations stored in
		// the database.
		foreach($users as $userk => &$userv) {
			// Get additional informations stored in database.
			$xpath = sprintf("//system/usermanagement/users/user[name='%s']",
			  $userv['name']);
			$object = $xmlConfig->get($xpath);
			if(!is_null($object)) {
				$userv['email'] = $object['email'];
				$userv['disallowusermod'] = boolval(
				  $object['disallowusermod']);
			}
		}
		// Filter result.
		return $this->applyFilter($users, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get an user configuration object.
	 * @param params An array containing the following fields:
	 *   \em name The name of the user.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_MISC_FAILURE
	 */
	function getUser($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "name":{"type":"string"}
			  }
		  }');
		// Get the user informations.
		$user = new OMVUser($params['name']);
		if(!$user->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("User '%s' does not exist"),
			  $params['name']));
		}
		$result = array(
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
			"email" => "",
			"disallowusermod" => FALSE
		);
		// Get additional informations stored in database.
		$xpath = sprintf("//system/usermanagement/users/user[name='%s']",
		  $result['name']);
		$object = $xmlConfig->get($xpath);
		if(!is_null($object)) {
			$result['email'] = $object['email'];
			$result['disallowusermod'] = boolval(
			  $object['disallowusermod']);
		}
		return $result;
	}

	/**
	 * Get the user configuration from the current context user. The
	 * returned object contains only a limited number of user informations.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The result object contains the fields \em name, \em comment,
	 *   \em email and the internal field \em _readOnly.
	 */
	function getUserByContext($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_EVERYONE
		  ));
		// Get the user configuration object.
		$object = $this->callMethod("getUser", array(
		  "name" => $context['username']), $this->getAdminContext());
		// Prepare the result object.
		return array(
			  "name" => $object['name'],
			  "comment" => $object['comment'],
			  "email" => $object['email'],
			  // Hijack the '_readOnly' flag to set the form to read-only.
			  "_readOnly" => $object['disallowusermod']
		  );
	}

	/**
	 * Set (add/update) an user.
	 * @param params An array containing the following fields:
	 *   \em name The name of the user.
	 *   \em uid The user ID. This field is optional.
	 *   \em groups A list of groups which the user is a member of as an
	 *   array of strings.
	 *   \em shell The name of the users login shell. This field is optional.
	 *   \em password The plain password to use.
	 *   \em email The users email address.
	 *   \em comment Any text string. This field is optional.
	 *   \em disallowusermod Set to FALSE to disallow the user to modify his
	 *   account.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_EXEC_FAILED
	 * @throw E_MISC_FAILURE
	 */
	function setUser($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "name":{'.$GLOBALS['OMV_JSONSCHEMA_USERNAME'].'},
				  "uid":{"type":"integer","optional":true},
				  "groups":{
					  "type":"array",
					  "items":{
						  "type":"string"
					  }
				  },
				  "shell":{"type":"string","optional":true},
				  "password":{"type":"string"},
				  "email":{'.$GLOBALS['OMV_JSONSCHEMA_EMAIL'].'},
				  "comment":{"type":"string","optional":true},
				  "disallowusermod":{"type":"boolean"}
			  }
		  }');
		// Do not allow to modify user 'root' via RPC.
		if(0 == strcasecmp("root", $params['name'])) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  gettext("Operation not allowed for user '%s'."),
			  $params['name']));
		}
		// Get additional informations stored in database.
		$xpath = sprintf("//system/usermanagement/users/user[name='%s']",
		  $params['name']);
		$object = $xmlConfig->get($xpath);
		// Set configuration object.
		$success = FALSE;
		if(is_null($object)) {
			$object = array(
				"uuid" => OMVUtil::uuid(),
				"name" => $params['name'],
				"email" => $params['email'],
				"disallowusermod" => array_boolval($params,
				  "disallowusermod")
			);
			// Append new configuration object.
			$success = $xmlConfig->set("//system/usermanagement/users",
			  array("user" => $object));
		} else {
			$object['email'] = $params['email'];
			$object['disallowusermod'] = array_boolval($params,
			  "disallowusermod");
			// Update existing configuration object.
			$success = $xmlConfig->replace($xpath, $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		$cmdArgs = array();
		if(array_key_exists("shell", $params)) {
			$cmdArgs[] = "--shell";
			$cmdArgs[] = escapeshellarg($params['shell']);
		}
		if(array_key_exists("comment", $params)) {
			$cmdArgs[] = "--comment";
			$cmdArgs[] = escapeshellarg($params['comment']);
		}
		if(array_key_exists("groups", $params)) {
			$cmdArgs[] = "--groups";
			$cmdArgs[] = escapeshellarg(implode(",", $params['groups']));
		}
		// Does the user already exist?
		$user = new OMVUser($params['name']);
		if(!$user->exists()) {
			// Get user management settings.
			$xpath = "//system/usermanagement/homedirectory";
			$hdsobject = $xmlConfig->get($xpath);
			if(is_null($hdsobject)) {
				throw new OMVException(
				  OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED, $xpath);
			}
			// Append additional arguments.
			$cmdArgs[] = "--user-group";
			$cmdArgs[] = "--password";
			$cmdArgs[] = escapeshellarg(crypt($params['password']));
			if(array_key_exists("uid", $params)) {
				$cmdArgs[] = "--uid";
				$cmdArgs[] = $params['uid'];
			}
			if(TRUE === boolval($hdsobject['enable'])) {
				// Get the absolute shared folder path.
				$sfpath = OMVRpc::exec("ShareMgmt", "getPath", array(
				  "uuid" => $hdsobject['sharedfolderref']), $context);
				$cmdArgs[] = "--create-home";
				$cmdArgs[] = "--home";
				$cmdArgs[] = escapeshellarg(sprintf("%s/%s", $sfpath,
				  $params['name']));
			}
			// Create a new user.
			$cmd = sprintf("export LANG=C; useradd %s %s 2>&1",
			  implode(" ", $cmdArgs), escapeshellarg($params['name']));
			if(0 !== $this->exec($cmd, $output)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
		} else {
			// Append additional arguments.
			if(array_key_exists("password", $params) &&
			  !empty($params['password'])) {
				$cmdArgs[] = "--password";
				$cmdArgs[] = escapeshellarg(crypt($params['password']));
			}
			// Modify existing user.
			$cmd = sprintf("export LANG=C; usermod %s %s 2>&1",
			  implode(" ", $cmdArgs), escapeshellarg($params['name']));
			if(0 !== $this->exec($cmd, $output)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
		}
		// Append some more informations to the notification object.
		$object = array_merge($object, $params);
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify((!$user->exists()) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.usermanagement.users.user", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Set an user configuration object from the current context user.
	 * @param params An array containing the following fields:
	 *   \em password The password. Empty passwords are ignored.
	 *   \em email The email address.
	 *   \em comment The comment.
	 * @param context The context of the caller.
	 * @return None.
	 */
	function setUserByContext($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_EVERYONE
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
				"type":"object",
				"properties":{
					"password":{"type":"string"},
					"email":{'.$GLOBALS['OMV_JSONSCHEMA_EMAIL'].'},
					"comment":{"type":"string"}
				}
			}');
		// Get the user configuration object.
		$object = $this->callMethod("getUser", array(
		  "name" => $context['username']), $this->getAdminContext());
		// Update the user configuration.
		return $this->callMethod("setUser", array(
		  "name" => $object['name'],
		  "disallowusermod" => $object['disallowusermod'],
		  "groups" => $object['groups'],
		  "comment" => $params['comment'],
		  "email" => $params['email'],
		  "password" => $params['password']), $this->getAdminContext());
	}

	/**
	 * Delete a user.
	 * @param params An array containing the following fields:
	 *   \em name The name of the user to delete.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 * @throw E_EXEC_FAILED
	 */
	function deleteUser($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "name":{"type":"string"}
			  }
		  }');
		// Get the user configuration object (required for the notification
		// event).
		$object = $this->callMethod("getUser", array(
		  "name" => $params['name']), $this->getAdminContext());
		// Delete the user.
		$cmd = sprintf("export LANG=C; userdel --force %s 2>&1",
		  escapeshellarg($params['name']));
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Delete additional user informations stored in database.
		$xpath = sprintf("//system/usermanagement/users/user[name='%s']",
		  $params['name']);
		$xmlConfig->delete($xpath);
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.usermanagement.users.user", $object);
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
	 * @return None.
	 */
	function importUsers($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "csv":{"type":"string"}
			  }
		  }');
		// Create a background process.
		$bgStatusFilename = $this->createBgProcStatus();
		$pid = $this->fork();
		if($pid > 0) { // Parent process.
			$this->initializeBgProcStatus($bgStatusFilename, $pid);
			return $bgStatusFilename;
		}
		// Child process.
		try {
			$bgOutputFilename = $this->createBgProcOutput();
			$this->updateBgProcStatus($bgStatusFilename, "outputfilename",
			  $bgOutputFilename);
			// Import users.
			$objects = array();
			$csv = explode("\n", $params['csv']);
			foreach($csv as $linek => $linev) {
				$linev = trim($linev);
				# Skip comments.
				if(empty($linev) || ($linev[0] === "#"))
					continue;
				# Parse line.
				if(1 !== preg_match("/^([^;]+);(\d*);([^;]*);([^;]*);([^;]+);".
				  "([^;]*);(true|1|yes|y|on|false|0|no|n|off)$/i",
				  $linev, $matches)) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf(gettext("Invalid line: %s"), $linev));
				}
				$object = array(
					"name" => $matches[1],
					"comment" => $matches[3],
					"email" => $matches[4],
					"password" => $matches[5],
					"groups" => explode(",", $matches[6]),
					"disallowusermod" => boolval($matches[7]),
					"shell" => "/bin/dash"
				);
				if(!empty($matches[2]) && is_numeric($matches[2]))
					$object['uid'] = intval($matches[2]);
				$objects[] = $object;
			}
			// Check if the users does not exist until now.
			foreach($objects as $objectk => $objectv) {
				// Check if an user with the given name already exists.
				$user = new OMVUser($objectv['name']);
				if($user->exists()) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf(gettext("User '%s' already exist"),
					  $objectv['name']));
				}
				// Check if an user with the given UID already exists.
				if(array_key_exists("uid", $object)) {
					$user = new OMVUser($objectv['uid']);
					if($user->exists()) {
						throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
						  sprintf(gettext("User ID '%d' already exist"),
						  $objectv['uid']));
					}
				}
			}
			// Finally create the users.
			foreach($objects as $objectk => $objectv) {
				$this->callMethod("setUser", $objectv, $context);
			}
			$this->finalizeBgProcStatus($bgStatusFilename, $output);
			exit(0);
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
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
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "start":{"type":"integer"},
				  "limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
				  "sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
				  "sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
			  }
		  }');
		// Get the list of non-system groups.
		$groups = $this->enumerateGroupsByType("normal");
		foreach($groups as $groupk => &$groupv) {
			// Get additional informations stored in database.
			$xpath = sprintf("//system/usermanagement/groups/group[name='%s']",
			  $groupv['name']);
			$object = $xmlConfig->get($xpath);
			if(!is_null($object)) {
				$groupv['comment'] = $object['comment'];
			}
		}
		// Filter result.
		return $this->applyFilter($groups, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a group.
	 * @param params An array containing the following fields:
	 *   \em name The name of the group.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_MISC_FAILURE
	 */
	function getGroup($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "name":{"type":"string"}
			  }
		  }');
		// Get the group informations.
		$group = new OMVUserGroup($params['name']);
		if(!$group->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("Group '%s' does not exist"),
			  $params['name']));
		}
		$result = array(
			"name" => $group->getName(),
			"gid" => $group->getGid(),
			"members" => $group->getMembers()
		);
		// Get additional informations stored in database.
		$xpath = sprintf("//system/usermanagement/groups/group[name='%s']",
		  $result['name']);
		$object = $xmlConfig->get($xpath);
		if(!is_null($object)) {
			$result['comment'] = $object['comment'];
		}
		return $result;
	}

	/**
	 * Set (add/update) a group.
	 * @param params An array containing the following fields:
	 *   \em name The name of the group.
	 *   \em gid The group ID. This field is optional.
	 *   \em comment Any text string.
	 *   \em members The group members as an array of user names.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setGroup($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "name":{"type":"string"},
				  "gid":{"type":"integer","optional":true},
				  "comment":{"type":"string"},
				  "members":{
					  "type":"array",
					  "items":{
						  "type":"string"
					  }
				  }
			  }
		  }');
		// Try to get existing configuration object.
		$xpath = sprintf("//system/usermanagement/groups/group[name='%s']",
		  $params['name']);
		$object = $xmlConfig->get($xpath);
		// Set configuration object
		$success = FALSE;
		if(is_null($object)) {
			$object = array(
				"uuid" => OMVUtil::uuid(),
				"name" => $params['name'],
				"comment" => $params['comment']
			);
			// Append object to configuration.
			$success = $xmlConfig->set("//system/usermanagement/groups",
			  array("group" => $object));
		} else {
			$object['comment'] = $params['comment'];
			// Update existing configuration object.
			$success = $xmlConfig->replace("//system/usermanagement/groups/".
			  "group[name='{$params['name']}']", $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Does the group already exist?
		$group = new OMVUserGroup($params['name']);
		if(!$group->exists()) {
			// Prepare command arguments.
			$cmdArgs = array();
			if(array_key_exists("gid", $params)) {
				$cmdArgs[] = "--gid";
				$cmdArgs[] = $params['gid'];
				// Append additional fields to configuration object for
				// the notification event.
				$object['gid'] = $params['gid'];
			}
			// Create new group.
			$cmd = sprintf("export LANG=C; groupadd %s %s 2>&1",
			  implode(" ", $cmdArgs), escapeshellarg($params['name']));
			if(0 !== $this->exec($cmd, $output)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
		} else {
			// Nothing to do here.
		}
		// Process members.
		if(array_key_exists("members", $params)) {
			// Append additional fields to configuration object for
			// the notification event.
			$object = array_merge($object, array(
				"members" => $params['members']
			));
			// Append the given members to the group.
			$cmdArgs = array();
			$cmdArgs[] = "--members";
			$cmdArgs[] = escapeshellarg(implode(",", $params['members']));
			$cmd = sprintf("export LANG=C; gpasswd %s %s 2>&1",
			  implode(" ", $cmdArgs), escapeshellarg($params['name']));
			if(0 !== $this->exec($cmd, $output)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(!$group->exists() ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.usermanagement.groups.group", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a group.
	 * @param params An array containing the following fields:
	 *   \em name The name of the group to delete.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 * @throw E_EXEC_FAILED
	 */
	function deleteGroup($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "name":{"type":"string"}
			  }
		  }');
		// Get the group configuration object (required for the notification
		// event).
		$object = $this->callMethod("getGroup", array(
		  "name" => $params['name']), $context);
		// Delete the group.
		$cmd = sprintf("export LANG=C; delgroup --only-if-empty %s 2>&1",
		  escapeshellarg($params['name']));
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Delete configuration object.
		$xpath = sprintf("//system/usermanagement/groups/group[name='%s']",
		  $params['name']);
		$xmlConfig->delete($xpath);
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.usermanagement.groups.group", $object);
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
	 * @return None.
	 */
	function importGroups($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "csv":{"type":"string"}
			  }
		  }');
		// Create a background process.
		$bgStatusFilename = $this->createBgProcStatus();
		$pid = $this->fork();
		if($pid > 0) { // Parent process.
			$this->initializeBgProcStatus($bgStatusFilename, $pid);
			return $bgStatusFilename;
		}
		// Child process.
		try {
			$bgOutputFilename = $this->createBgProcOutput();
			$this->updateBgProcStatus($bgStatusFilename, "outputfilename",
			  $bgOutputFilename);
			// Import groups.
			$objects = array();
			$csv = explode("\n", $params['csv']);
			foreach($csv as $linek => $linev) {
				$linev = trim($linev);
				# Skip comments.
				if(empty($linev) || ($linev[0] === "#"))
					continue;
				# Parse line.
				if(1 !== preg_match("/^([^;]+);(\d*);([^;]*)$/i", $linev,
				  $matches)) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf(gettext("Invalid line: %s"), $linev));
				}
				$object = array(
					"name" => $matches[1],
					"comment" => $matches[3],
					"members" => array()
				);
				if(!empty($matches[2]) && is_numeric($matches[2]))
					$object['gid'] = intval($matches[2]);
				$objects[] = $object;
			}
			// Check if the users does not exist until now.
			foreach($objects as $objectk => $objectv) {
				// Check if an group with the given name already exists.
				$group = new OMVUserGroup($objectv['name']);
				if($group->exists()) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf(gettext("Group '%s' already exist"),
					  $objectv['name']));
				}
				// Check if an group with the given GID already exists.
				if(array_key_exists("gid", $object)) {
					$group = new OMVUserGroup($objectv['gid']);
					if($group->exists()) {
						throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
						  sprintf(gettext("Group ID '%d' already exist"),
						  $objectv['gid']));
					}
				}
			}
			// Finally create the groups.
			foreach($objects as $objectk => $objectv) {
				$this->callMethod("setGroup", $objectv, $context);
			}
			$this->finalizeBgProcStatus($bgStatusFilename, $output);
			exit(0);
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}

	/**
	 * Get user management settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$xpath = "//system/usermanagement/homedirectory";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data.
		$object['enable'] = boolval($object['enable']);
		return $object;
	}

	/**
	 * Set user management settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 * @throw E_EXEC_FAILED
	 */
	public function setSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "enable":{"type":"boolean"},
				  "sharedfolderref":{'.
					$GLOBALS['OMV_JSONSCHEMA_UUID_NONE'].'}
			  }
		  }');
		// Update the configuration object.
		$xpath = "//system/usermanagement/homedirectory";
		$object = array(
			"enable" => array_boolval($params, 'enable'),
			"sharedfolderref" => $params['sharedfolderref']
		);
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Build the home directory path.
		$homeDirPath = "";
		if(TRUE === boolval($params['enable'])) {
			// Get the absolute shared folder path.
			$homeDirPath = OMVRpc::exec("ShareMgmt", "getPath", array(
			  "uuid" => $object['sharedfolderref']), $context);
		}
		// Update the non-system user.
		$users = $this->enumerateUsersByType("normal");
		foreach($users as $userk => $userv) {
			$cmdArgs = array();
			if(!empty($homeDirPath)) {
				// Set new home directory. Move the content from the old to
				// the new directory.
				$cmdArgs[] = "--move-home";
				$cmdArgs[] = "--home";
				$cmdArgs[] = sprintf("%s/%s", $homeDirPath, $userv['name']);
			} else {
				// Unset home directory.
				$cmdArgs[] = "--home";
				$cmdArgs[] = $homeDirPath;
			}
			$cmd = sprintf("usermod %s %s 2>&1", implode(" ", $cmdArgs),
			  escapeshellarg($userv['name']));
			if(0 !== $this->exec($cmd, $output)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			// Create the home directory if it does not exist until now.
			if(!empty($homeDirPath)) {
				$path = sprintf("%s/%s", $homeDirPath, $userv['name']);
				if(!file_exists($path)) {
					if(!mkdir($path, 0755, TRUE)) {
						throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
						  sprintf("Failed to create home directory '%s'."),
						  $path);
					}
					if(!chown($path, $userv['name'])) {
						throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
						  sprintf("Failed to change owner of the ".
						  "directory '%s' to '%s'."), $path, $userv['name']);
					}
					if(!chgrp($path, $GLOBALS['OMV_USERMGMT_DEFAULT_GROUP'])) {
						throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
						  sprintf("Failed to change group of the ".
						  "directory '%s' to '%s'."), $path,
						  $GLOBALS['OMV_USERMGMT_DEFAULT_GROUP']);
					}
				}
			}
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.usermanagement.homedirectory", $object);
		// Return the configuration object.
		return $object;
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceUserMgmt());
?>
