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
require_once("openmediavault/object.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/system.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class UserMgmtRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getUserList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"getUser" => array('{
				"type":"object",
				"properties":{
					"name":{"type":"string"}
				}
			}'),
			"setUser" => array('{
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
					"shell":{"type":"string"},
					"password":{"type":"string"},
					"email":{'.$GLOBALS['OMV_JSONSCHEMA_EMAIL'].'},
					"comment":{"type":"string"},
					"disallowusermod":{"type":"boolean"}
				}
			}'),
			"setSessionUser" => array('{
				"type":"object",
				"properties":{
					"password":{"type":"string"},
					"email":{'.$GLOBALS['OMV_JSONSCHEMA_EMAIL'].'},
					"comment":{"type":"string"}
				}
			}'),
			"deleteUser" => array('{
				"type":"object",
				"properties":{
					"name":{"type":"string"}
				}
			}'),
			"getGroupList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"getGroup" => array('{
				"type":"object",
				"properties":{
					"name":{"type":"string"}
				}
			}'),
			"setGroup" => array('{
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
			}'),
			"deleteGroup" => array('{
				"type":"object",
				"properties":{
					"name":{"type":"string"}
				}
			}'),
			"importUsers" => array('{
				"type":"object",
				"properties":{
					"csv":{"type":"string"}
				}
			}'),
			"importGroups" => array('{
				"type":"object",
				"properties":{
					"csv":{"type":"string"}
				}
			}'),
			"setSettings" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"sharedfolderref":{'.
					  $GLOBALS['OMV_JSONSCHEMA_UUID_NONE'].'}
				}
			}')
		);
	}

	/**
	 * Enumerate users.
	 * @param type The user type, e.g. system, normal or all.
	 * @return An array containing user objects with following fields:
	 *   name, UID, GID, comment, home directory, and shell program,
	 *   last changed, minimum, maximum, warn, inactive, expire and reserved.
	 */
	private function enumerateUsersByType($type) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Get shadow password suite configuration
		$loginDefs = OMVSystem::getLoginDefs();
		// Get list of user names
		$users = OMVSystemUsers::get();
		// Iterate through all users to filter the requested ones
		$objects = array();
		foreach ($users as $userk => $userv) {
			$user = new OMVSystemUser($userv);
			if (!$user->exists())
				continue;
			// Check if the current user is requested
			$append = FALSE;
			$system = !(($user->getUid() >= $loginDefs['UID_MIN']) &&
			  ($user->getUid() <= $loginDefs['UID_MAX']));
			switch ($type) {
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
			if (TRUE === $append) {
				$objects[] = array(
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
		return $objects;
	}

	public function enumerateSystemUsers() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get system users
		return $this->enumerateUsersByType("system");
	}

	public function enumerateUsers() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get non-system users
		return $this->enumerateUsersByType("normal");
	}

	public function enumerateAllUsers() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get all users
		return $this->enumerateUsersByType("all");
	}

	/**
	 * Enumerate groups.
	 * @param type The group type, e.g. system, normal or all.
	 * @return An array containing user objects with following fields:
	 *   name, GID and group members (array of user names).
	 */
	public function enumerateGroupsByType($type) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Get shadow password suite configuration
		$loginDefs = OMVSystem::getLoginDefs();
		// Get list of group names
		$groups = OMVSystemGroups::get();
		// Get group informations
		$objects = array();
		foreach ($groups as $groupk => $groupv) {
			$group = new OMVSystemGroup($groupv);
			if (!$group->exists())
				continue;
			// Check if the current group is requested
			$append = FALSE;
			$system = !(($group->getGid() >= $loginDefs['GID_MIN']) &&
			  ($group->getGid() <= $loginDefs['GID_MAX']));
			switch ($type) {
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
			if (TRUE === $append) {
				$objects[] = array(
					"name" => $group->getName(),
					"gid" => $group->getGid(),
					"members" => $group->getMembers(),
					"system" => $system
				);
			}
		}
		return $objects;
	}

	public function enumerateSystemGroups() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get system groups
		return $this->enumerateGroupsByType("system");
	}

	public function enumerateGroups() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get non-system groups
		return $this->enumerateGroupsByType("normal");
	}

	public function enumerateAllGroups() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get all groups
		return $this->enumerateGroupsByType("all");
	}

	/**
	 * Get list of users (except system users).
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	function getUserList($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get users
		$users = OMVRpc::exec("UserMgmt", "enumerateUsers");
		// Process users and skip all system users except 'admin'
		$objects = array();
		foreach ($users as $userk => &$userv) {
			// Get additional informations stored in database
			$xpath = sprintf("//system/usermanagement/users/user[name='%s']",
			  $userv['name']);
			$object = $xmlConfig->get($xpath);
			if (!is_null($object)) {
				$userv['email'] = $object['email'];
				$userv['disallowusermod'] = boolval($object['disallowusermod']);
			}
		}
		// Filter result
		return $this->applyFilter($users, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get an user configuration object.
	 * @param data An array containing the following fields:
	 *   \em name The name of the user.
	 * @return The requested configuration object.
	 */
	function getUser($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get the user informations
		$user = new OMVSystemUser($data['name']);
		if (!$user->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("User '%s' does not exist"), $data['name']));
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
			"email" => ""
		);
		// Get additional informations stored in database
		$xpath = sprintf("//system/usermanagement/users/user[name='%s']",
		  $result['name']);
		$object = $xmlConfig->get($xpath);
		if (!is_null($object)) {
			$result['email'] = $object['email'];
			$result['disallowusermod'] = boolval($object['disallowusermod']);
		}
		return $result;
	}

	/**
	 * Get an user configuration object from the current session user
	 * Note, it is not possible to use the UserMgmt::getUser RPC due
	 * the limited permissions.
	 */
	function getSessionUser() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		// Get the user informations
		$result = array(
			"name" => OMVSession::getUsername(),
			"email" => "",
			"_readOnly" => FALSE
		);
		// Get additional informations stored in database
		$xpath = sprintf("//system/usermanagement/users/user[name='%s']",
		  OMVSession::getUsername());
		$object = $xmlConfig->get($xpath);
		if (!is_null($object)) {
			$result['email'] = $object['email'];
			// Hijack the '_readOnly' flag to set the form read-only
			$result['_readOnly'] = boolval($object['disallowusermod']);
		}
		return $result;
	}

	/**
	 * Set (add/update) an user.
	 * @param data An array containing the following fields:
	 *   \em name The name of the user.
	 *   \em uid The user ID. This field is optional.
	 *   \em groups A list of groups which the user is a member of as an
	 *   array of strings.
	 *   \em shell The name of the users login shell.
	 *   \em password The plain password to use.
	 *   \em email The users email address.
	 *   \em comment Any text string.
	 *   \em disallowusermod Set to FALSE to disallow the user to modify his
	 *   account.
	 * @return None
	 */
	function setUser($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get additional informations stored in database
		$xpath = sprintf("//system/usermanagement/users/user[name='%s']",
		  $data['name']);
		$object = $oldObject = $xmlConfig->get($xpath);
		// Set configuration object
		$success = FALSE;
		if (is_null($object)) {
			$object = array(
				"uuid" => OMVUtil::uuid(),
				"name" => $data['name'],
				"email" => $data['email'],
				"disallowusermod" => array_boolval($data, "disallowusermod")
			);
			// Append new configuration object
			$success = $xmlConfig->set("//system/usermanagement/users",
			  array("user" => $object));
		} else {
			$object['email'] = $data['email'];
			$object['disallowusermod'] = array_boolval($data,
			  "disallowusermod");
			// Update existing configuration object
			$success = $xmlConfig->replace($xpath, $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Append additional fields to configuration object for notification.
		$object = array_merge($object, array(
			"shell" => $data['shell'],
			"password" => $data['password'],
			"groups" => $data['groups'],
			"comment" => $data['comment']
		));
		if (isset($data['uid'])) {
			$object['uid'] = $data['uid'];
		}
		// Get the user to find out if it still exists and to get additional
		// informations.
		$user = new OMVSystemUser($data['name']);
		if ($user->exists()) {
			// Append some informations about the user to the old
			// configuration object.
			$oldObject = array_merge($oldObject, array(
				"disallowusermod" => boolval($oldObject['disallowusermod']),
				"shell" => $user->getShell(),
				"password" => $user->getPassword(),
				"groups" => $user->getGroups(),
				"comment" => $user->getGecos(),
				"uid" => $user->getUid()
			));
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		if (!$user->exists()) {
			$dispatcher->notify(OMV_NOTIFY_CREATE,
			  "org.openmediavault.system.usermanagement.users.user", $object);
		} else {
			$dispatcher->notify(OMV_NOTIFY_MODIFY,
			  "org.openmediavault.system.usermanagement.users.user", $object,
			  $oldObject);
		}
	}

	/**
	 * Set an user configuration object from the current session user
	 */
	function setSessionUser($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Try to get existing configuration object
		$xpath = sprintf("//system/usermanagement/users/user[name='%s']",
		  OMVSession::getUsername());
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			$object = array(
				"uuid" => OMVUtil::uuid(),
				"name" => OMVSession::getUsername(),
				"email" => $data['email'],
				"disallowusermod" => FALSE
			);
			// Append new configuration object
			$success = $xmlConfig->set("//system/usermanagement/users",
			  array("user" => $object));
		} else {
			// Check if user is allowed to modify his account
			if (boolval($object['disallowusermod']) === TRUE) {
				throw new OMVException(OMVErrorMsg::E_MISC_OPERATION_DENIED);
			}
			$object['email'] = $data['email'];
			// Update existing configuration object
			$success = $xmlConfig->replace($xpath, $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Append additional fields to configuration object for notification
		$object = array_merge($object, array(
			"password" => $data['password']
		));
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.usermanagement.users.user", $object);
	}

	/**
	 * Delete a user.
	 * @param data An array containing the following fields:
	 *   \em name The name of the user to delete.
	 * @return None
	 */
	function deleteUser($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get the user configuration object
		$object = OMVRpc::exec("UserMgmt", "getUser", array(
		  "name" => $data['name']));
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.system.usermanagement.users.user", $object);
		// Delete additional user informations stored in database
		$xpath = sprintf("//system/usermanagement/users/user[name='%s']",
		  $object['name']);
		$xmlConfig->delete($xpath);
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.usermanagement.users.user", $object);
	}

	/**
	 * Import a list of users.
	 * @param data An array containing the following fields:
	 *   \em csv The user values, e.g.:
	 *   name;uid;comment;email;password;group,group,...;disallowusermod
	 *   test1;;comment;test1@xyz.com;foobarpwd1;adm;1
	 *   test2;1200;comment2;test2@xyz.com;foobarpwd2;adm,audio,www-data;0
	 *   test3;;comment3;test3@xyz.com;foobarpwd3;ssh;1
	 * @return None
	 */
	function importUsers($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Import users
		$objects = array();
		$csv = explode("\n", $data['csv']);
		foreach ($csv as $linek => $linev) {
			$linev = trim($linev);
			# Skip comments.
			if (empty($linev) || ($linev[0] === "#"))
				continue;
			# Parse line.
			if (1 !== preg_match("/^([^;]+);(\d*);([^;]+);([^;]+);([^;]+);".
			  "([^;]+);(true|1|yes|y|on|false|0|no|n|off)$/i",
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
			if (!empty($matches[2]) && is_numeric($matches[2]))
				$object['uid'] = intval($matches[2]);
			$objects[] = $object;
		}
		// Check if the users does not exist until now
		foreach ($objects as $objectk => $objectv) {
			// Check if an user with the given name already exists
			$user = new OMVSystemUser($objectv['name']);
			if ($user->exists()) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf(gettext("User '%s' already exist"),
				  $objectv['name']));
			}
			// Check if an user with the given UID already exists
			if (array_key_exists("uid", $object)) {
				$user = new OMVSystemUser($objectv['uid']);
				if ($user->exists()) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf(gettext("User ID '%d' already exist"),
					  $objectv['uid']));
				}
			}
		}
		// Finally create the users
		foreach ($objects as $objectk => $objectv) {
			OMVRpc::exec("UserMgmt", "setUser", $objectv);
		}
	}

	/**
	 * Get list of groups (except system groups).
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	function getGroupList($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get non-system groups
		$groups = OMVRpc::exec("UserMgmt", "enumerateGroups");
		foreach ($groups as $groupk => &$groupv) {
			// Get additional informations stored in database
			$xpath = sprintf("//system/usermanagement/groups/group[name='%s']",
			  $groupv['name']);
			$object = $xmlConfig->get($xpath);
			if (!is_null($object)) {
				$groupv['comment'] = $object['comment'];
			}
		}
		// Filter result
		return $this->applyFilter($groups, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get a group
	 * @param data An array containing the following fields:
	 *   \em name The name of the group.
	 * @return The requested configuration object.
	 */
	function getGroup($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get the group informations
		$group = new OMVSystemGroup($data['name']);
		if (!$group->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("Group '%s' does not exist"), $data['name']));
		}
		$result = array(
			"name" => $group->getName(),
			"gid" => $group->getGid(),
			"members" => $group->getMembers()
		);
		// Get additional informations stored in database
		$xpath = sprintf("//system/usermanagement/groups/group[name='%s']",
		  $result['name']);
		$object = $xmlConfig->get($xpath);
		if (!is_null($object)) {
			$result['comment'] = $object['comment'];
		}
		return $result;
	}

	/**
	 * Set (add/update) a group.
	 * @param data An array containing the following fields:
	 *   \em name The name of the group.
	 *   \em gid The group ID. This field is optional.
	 *   \em comment Any text string.
	 *   \em members The group members as an array of user names.
	 * @return None
	 */
	function setGroup($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Try to get existing configuration object
		$xpath = sprintf("//system/usermanagement/groups/group[name='%s']",
		  $data['name']);
		$object = $xmlConfig->get($xpath);
		// Set configuration object
		$success = FALSE;
		if (is_null($object)) {
			$object = array(
				"uuid" => OMVUtil::uuid(),
				"name" => $data['name'],
				"comment" => $data['comment']
			);
			// Append object to configuration
			$success = $xmlConfig->set("//system/usermanagement/groups",
			  array("group" => $object));
		} else {
			$object['comment'] = $data['comment'];
			// Update existing configuration object
			$success = $xmlConfig->replace("//system/usermanagement/groups/".
			  "group[name='{$data['name']}']", $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Append additional fields to configuration object for notification
		$object = array_merge($object, array(
			"members" => $data['members']
		));
		if (isset($data['gid']))
			$object['gid'] = $data['gid'];
		// Get the group to find out if it still exists
		$group = new OMVSystemGroup($data['name']);
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(!$group->exists() ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.usermanagement.groups.group", $object);
	}

	/**
	 * Delete a group.
	 * @param data An array containing the following fields:
	 *   \em name The name of the group to delete.
	 * @return None
	 */
	function deleteGroup($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get the group configuration object
		$object = OMVRpc::exec("UserMgmt", "getGroup", array(
		  "name" => $data['name']));
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.system.usermanagement.groups.group", $object);
		// Delete configuration object
		$xpath = sprintf("//system/usermanagement/groups/group[name='%s']",
		  $object['name']);
		$xmlConfig->delete($xpath);
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.usermanagement.groups.group", $object);
	}

	/**
	 * Import a list of groups.
	 * @param data An array containing the following fields:
	 *   \em csv The group values, e.g.:
	 *   name;gid;comment
	 *   grp1;;comment1
	 *   grp2;1200;comment2
	 * @return None
	 */
	function importGroups($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Import users
		$objects = array();
		$csv = explode("\n", $data['csv']);
		foreach ($csv as $linek => $linev) {
			$linev = trim($linev);
			# Skip comments.
			if (empty($linev) || ($linev[0] === "#"))
				continue;
			# Parse line.
			if (1 !== preg_match("/^([^;]+);(\d*);([^;]+)$/i", $linev,
			  $matches)) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf(gettext("Invalid line: %s"), $linev));
			}
			$object = array(
				"name" => $matches[1],
				"comment" => $matches[3],
				"members" => array()
			);
			if (!empty($matches[2]) && is_numeric($matches[2]))
				$object['gid'] = intval($matches[2]);
			$objects[] = $object;
		}
		// Check if the users does not exist until now
		foreach ($objects as $objectk => $objectv) {
			// Check if an group with the given name already exists
			$group = new OMVSystemGroup($objectv['name']);
			if ($group->exists()) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf(gettext("Group '%s' already exist"),
				  $objectv['name']));
			}
			// Check if an group with the given GID already exists
			if (array_key_exists("gid", $object)) {
				$group = new OMVSystemGroup($objectv['gid']);
				if ($group->exists()) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf(gettext("Group ID '%d' already exist"),
					  $objectv['gid']));
				}
			}
		}
		// Finally create the groups
		foreach ($objects as $objectk => $objectv) {
			OMVRpc::exec("UserMgmt", "setGroup", $objectv);
		}
	}

	/**
	 * Get user management settings.
	 */
	function getSettings() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//system/usermanagement/homedirectory";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		return $object;
	}

	/**
	 * Set user management settings.
	 */
	public function setSettings($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Update configuration
		$xpath = "//system/usermanagement/homedirectory";
		$object = array(
			"enable" => array_boolval($data, 'enable'),
			"sharedfolderref" => $data['sharedfolderref']
		);
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.usermanagement.homedirectory", $object);
	}
}
?>
