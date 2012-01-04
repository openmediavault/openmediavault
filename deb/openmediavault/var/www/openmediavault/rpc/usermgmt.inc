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
			"getUserList" => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}' // sortDir
			),
			"getUser" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"setUser" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"uid":{"type":["integer","string"]},
					"useuid":{"type":"boolean"},
					"name":{'.$GLOBALS['OMV_JSONSCHEMA_USERNAME'].'},
					"groups":{"type":"string"},
					"shell":{"type":"string"},
					"password":{"type":"string"},
					"email":{'.$GLOBALS['OMV_JSONSCHEMA_EMAIL'].'},
					"comment":{"type":"string"},
					"disallowusermod":{"type":"boolean"},
					"sharedfolderref":{'.
					  $GLOBALS['OMV_JSONSCHEMA_UUID_NONE'].'}
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
			"deleteUser" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"getGroupList" => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}' // sortDir
			),
			"getGroup" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"setGroup" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"gid":{"type":["integer","string"]},
					"usegid":{"type":"boolean"},
					"name":{"type":"string"},
					"comment":{"type":"string"}
				}
			}'),
			"deleteGroup" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"setUserPassword" => array('{
				"type":"object",
				"properties":{
					"name":{"type":"string"},
					"password":{"type":"string"}
				}
			}')
		);
	}

	/**
	 * Enumerate all users.
	 * @return An array containing user objects with following fields:
	 * name, UID, GID, comment, home directory, and shell program,
	 * last changed, minimum, maximum, warn, inactive, expire and reserved.
	 */
	public function enumerateUsers() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get list of user names
		$users = OMVSystemUsers::get();
		// Get user informations
		$objects = array();
		foreach ($users as $userk => $userv) {
			$user = new OMVSystemUser($userv);
			if (!$user->exists())
				continue;
			$objects[] = array(
				"name" => $user->getName(),
				"uid" => $user->getUid(),
				"gid" => $user->getGid(),
				"gecos" => $user->getGecos(),
				"dir" => $user->getHomeDirectory(),
				"shell" => $user->getShell(),
				"lastchanged" => $user->getLastChanged(),
				"minimum" => $user->getMinimum(),
				"maximum" => $user->getMaximum(),
				"warn" => $user->getWarn(),
				"inactive" => $user->getInactive(),
				"expire" => $user->getExpire(),
				"reserved" => $user->getReserved()
			);
		}
		return $objects;
	}

	/**
	 * Enumerate all groups.
	 * @return An array containing user objects with following fields:
	 * name, GID and group members (array of user names).
	 */
	public function enumerateGroups() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get list of group names
		$groups = OMVSystemGroups::get();
		// Get group informations
		$objects = array();
		foreach ($groups as $groupk => $groupv) {
			$group = new OMVSystemGroup($groupv);
			if (!$group->exists())
				continue;
			$objects[] = array(
				"name" => $group->getName(),
				"gid" => $group->getGid(),
				"members" => $group->getMembers()
			);
		}
		return $objects;
	}

	/**
	 * Get list of users
	 */
	function getUserList($start, $count, $sortField, $sortDir) {
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
		// Get configuration object
		$xpath = "//system/usermanagement/users/user";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional informations
		$loginDefs = OMVSystem::getLoginDefs();
		foreach ($objects as $objectk => &$objectv) {
			// Modify result data
			$objectv['disallowusermod'] = boolval($objectv['disallowusermod']);
			// Get user details
			$user = new OMVSystemUser($objectv['name']);
			$objectv['uid'] = $user->getUid();
			$objectv['_readOnly'] = false;
			$objectv['_used'] = false;
			// Mark users as read-only if their uid is not in the range of
			// [ UID_MIN ... UID_MAX ] (which is the case for system users
			// for example).
			if (($objectv['uid'] < $loginDefs['UID_MIN']) ||
			  ($objectv['uid'] > $loginDefs['UID_MAX'])) {
				$objectv['_readOnly'] = true;
			}
			// Protect user 'admin' from being deleted
			if ($objectv['name'] === "admin") {
				$objectv['_readOnly'] = false;
				// Hijack the '_used' flag to mark this user as not deletable
				$objectv['_used'] = true;
			}
		}
		// Filter result
		return $this->applyFilter($objects, $start, $count, $sortField,
		  $sortDir);
	}

	/**
	 * Get an user configuration object.
	 * @param uuid The UUID of the user configuration object.
	 */
	function getUser($uuid) {
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
		// Get configuration object
		$xpath = "//system/usermanagement/users/user[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Prepare response data
		if (empty($object['uid'])) {
			$object['useuid'] = 0;
			$user = new OMVSystemUser($object['name']);
			$object['uid'] = $user->getUid();
		} else {
			$object['useuid'] = 1;
		}
		// Modify result data
		$object['disallowusermod'] = boolval($object['disallowusermod']);
		return $object;
	}

	/**
	 * Get an user configuration object from the current session user
	 */
	function getSessionUser() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		// Get configuration object
		$xpath = sprintf("//system/usermanagement/users/user[name='%s']",
		  OMVSession::getUsername());
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return array(
			"name" => $object['name'],
			"password" => $object['password'],
			"email" => $object['email'],
			"comment" => $object['comment'],
			"_readOnly" => array_boolval($object, 'disallowusermod')
		);
	}

	/**
	 * Set (add/update) an user configuration object
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
		// Prepare configuration data
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"name" => $data['name'],
			"shell" => $data['shell'],
			"password" => $data['password'],
			"uid" => (isset($data['uid']) && array_boolval($data, 'useuid')) ?
			  $data['uid'] : "",
			"groups" => $data['groups'],
			"email" => $data['email'],
			"comment" => $data['comment'],
			"disallowusermod" => array_boolval($data, 'disallowusermod'),
			"sharedfolderref" => $data['sharedfolderref'] // Home directory
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness
			$user = new OMVSystemUser($data['name']);
			if ($user->exists() || (TRUE === $xmlConfig->exists(
			  "//system/usermanagement/users/user[name='{$data['name']}']"))) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				  gettext("An user with the given name already exists"));
			}
			// Append object to configuration
			$success = $xmlConfig->set("//system/usermanagement/users",
			  array("user" => $object));
		} else {
			// Update existing configuration object
			$success = $xmlConfig->replace("//system/usermanagement/users/" .
			  "user[uuid='{$data['uuid']}']", $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.usermanagement.users.user", $object);
		// If a shared folder is selected to be used as home directory then
		// ensure that the user has read/write permissions. If not, then set
		// them.
		if (!empty($object['sharedfolderref'])) {
			$privileges = OMVRpc::exec("ShareMgmt", "getPrivileges",
			  array($object['sharedfolderref']));
			$update = FALSE;
			foreach ($privileges as $privk => &$privv) {
				// Do we process the privilege for the current user?
				if (($privv['type'] === "user") &&
				  ($privv['name'] === $object['name'])) {
					// Check if the user has write or read-only access to its
					// own home directory.
					if (!in_array($privv['perms'], array(_PERMISSION_RWX,
					  _PERMISSION_RX))) {
						// The user doesn't have write or read-only permissions
						// to the shared folder used as home directory, so
						// give him write access permission to his own home
						// directory per default.
						$privv['perms'] = _PERMISSION_RWX;
						$update = TRUE;
					}
					break;
				}
			}
			if (TRUE === $update) {
				// Remove useless fields from the privileges structure.
				foreach ($privileges as $privk => &$privv)
					unset($privv['name']);
				// Update the shared folder privileges.
				OMVRpc::exec("ShareMgmt", "setPrivileges", array(
					array(
						"uuid" => $object['sharedfolderref'],
						"privileges" => $privileges
					)
				));
			}
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
		// Get configuration object
		$xpath = sprintf("//system/usermanagement/users/user[name='%s']",
		  OMVSession::getUsername());
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if (array_boolval($object, 'disallowusermod') === true) {
			throw new OMVException(OMVErrorMsg::E_MISC_OPERATION_DENIED);
		}
		// Update configuration data
		$object = array_merge($object, array(
			"password" => $data['password'],
			"email" => $data['email'],
			"comment" => $data['comment']
		));
		// Set configuration data
		if (FALSE === $xmlConfig->replace("//system/usermanagement/users/" .
		  "user[uuid='{$object['uuid']}']", $object)) {
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
		  "org.openmediavault.system.usermanagement.users.user", $object);
	}

	/**
	 * Delete a user.
	 * @param uuid The UUID of the user configuration object to delete.
	 */
	function deleteUser($uuid) {
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
		// Get the object
		$xpath = "//system/usermanagement/users/user[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.system.usermanagement.users.user", $object);
		// Delete configuration object
		if (FALSE === ($object = $xmlConfig->delete($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
			  $xpath);
		}
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
	 * Get list of groups
	 */
	function getGroupList($start, $count, $sortField, $sortDir) {
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
		// Get configuration object
		$xpath = "//system/usermanagement/groups/group";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional informations
		$loginDefs = OMVSystem::getLoginDefs();
		foreach ($objects as $objectk => &$objectv) {
			$group = new OMVSystemGroup($objectv['name']);
			$objectv['gid'] = $group->getGid();
			$objectv['_used'] = false;
			$objectv['_readOnly'] = false;
			// Mark groups as read-only if their gid is not in the range of
			// [ GID_MIN ... GID_MAX ] (which is the case for system groups
			// for example).
			if (($objectv['gid'] < $loginDefs['GID_MIN']) ||
			  ($objectv['gid'] > $loginDefs['GID_MAX'])) {
				$objectv['_readOnly'] = true;
			}
			// Set '_used' flag if group is in usage
			if (TRUE === $xmlConfig->exists("//system/usermanagement/" .
			  "users/user[contains(groups,'{$objectv['name']}')]")) {
				$objectv['_used'] = true;
			}
		}
		// Filter result
		return $this->applyFilter($objects, $start, $count, $sortField,
		  $sortDir);
	}

	/**
	 * Get a group
	 * @param uuid The UUID of the group configuration object.
	 */
	function getGroup($uuid) {
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
		// Get configuration object
		$xpath = "//system/usermanagement/groups/group[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Prepare response data
		if (empty($object['gid'])) {
			$object['usegid'] = 0;
			$group = new OMVSystemGroup($object['name']);
			$object['gid'] = $group->getGid();
		} else {
			$object['usegid'] = 1;
		}
		return $object;
	}

	/**
	 * Set (add/update) a group
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
		// Prepare configuration data
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"name" => $data['name'],
			"gid" => (isset($data['gid']) && array_boolval($data, 'usegid')) ?
			  $data['gid'] : "",
			"comment" => $data['comment']
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness
			if (TRUE === $xmlConfig->exists("//system/usermanagement/groups/" .
			  "group[name='{$data['name']}']")) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				  gettext("A group with the given name already exists"));
			}
			// Append object to configuration
			$success = $xmlConfig->set("//system/usermanagement/groups",
			  array("group" => $object));
		} else {
			// Update existing configuration object
			$success = $xmlConfig->replace("//system/usermanagement/groups/" .
			  "group[uuid='{$data['uuid']}']", $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.usermanagement.groups.group", $object);
	}

	/**
	 * Delete a group.
	 * @param uuid The UUID of the group configuration object to delete.
	 */
	function deleteGroup($uuid) {
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
		// Get the object
		$xpath = "//system/usermanagement/groups/group[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Check if the group is used somewhere
		if (TRUE === $xmlConfig->exists("//system/usermanagement/" .
		  "users/user[contains(groups,'{$object['name']}')]")) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INUSE);
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.system.usermanagement.groups.group", $object);
		// Delete configuration object
		if (FALSE === ($object = $xmlConfig->delete($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
			  $xpath);
		}
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
	 * Set the password for the given user.
	 */
	function setUserPassword($data) {
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
		// Get user configuration object
		$xpath = sprintf("//system/usermanagement/users/user[name='%s']",
		  $data['name']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Set new password
		$object['password'] = $data['password'];
		// Update configuration object
		if (FALSE === $xmlConfig->replace("//system/usermanagement/users/" .
		  "user[uuid='{$object['uuid']}']", $object)) {
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
		  "org.openmediavault.system.usermanagement.users.user", $object);
	}
}
?>
