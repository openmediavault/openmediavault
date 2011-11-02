<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
require_once("openmediavault/system.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class ShareMgmtRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getList" => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}' // sortDir
			),
			"get" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"set" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"name":{'.$GLOBALS['OMV_JSONSCHEMA_SHARENAME'].'},
					"comment":{"type":"string"},
					"mntentref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
					"umask":{"type":"string","enum":["700","755","777"]}
				}
			}'),
			"delete" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}', // uuid
				'{"type":"boolean"}' // recursive
			),
			"getPrivileges" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"setPrivileges" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
					"privileges":{
						"type":"array",
						"items":{
							"type":"object",
							"properties":{
								"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
								"perms":{"type":"integer","enum":[0,5,7]},
								"type":{"type":"string","enum":["user","group"]}
							}
						}
					}
				}
			}')
		);
	}

	/**
	 * Get list of mount points that can be used for shared folders.
	 * @return An array containing objects with the following fields:
	 * \em uuid, \em devicefile and \em description.
	 * The field \em uuid is the mount point configuration object UUID.
	 */
	public function getCandidates() {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get list of mount points, except bind mounts.
		$xpath = "//system/fstab/mntent[not(contains(opts,'bind'))]";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Get the filesystem details for each mount point.
		$result = array();
		foreach ($objects as $objectk => $objectv) {
			$fs = new OMVFilesystem($objectv['fsname']);
			if (!$fs->exists()) {
//				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
//				  sprintf(gettext("Filesystem '%s' not found"),
//				  $objectv['fsname']));
				continue;
			}
			// Skip the filesystems that are not mounted at the moment.
			if (FALSE === $fs->isMounted())
				continue;
			// Get the filesystem details.
			$object = array(
				"uuid" => $objectv['uuid'],
				"devicefile" => $fs->getDeviceFile(),
				"description" => $fs->getDeviceFile() // Default value
			);
			// Get some more filesystem details if possible.
			if (FALSE !== ($fsStats = $fs->getStatistics())) {
				$label = $fs->getLabel();
				$object['description'] = sprintf(
				  gettext("%s (%s available)"), !empty($label) ?
				  $label : $object['devicefile'],
				  binary_format($fsStats['available']));
			}
			$result[] = $object;
		}
		return $result;
	}

	/**
	 * Get list of shared folder configuration objects.
	 */
	function getList($start, $count, $sortField, $sortDir) {
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
		$xpath = "//system/shares/sharedfolder";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional informations.
		foreach ($objects as $objectk => &$objectv) {
			$objectv['_used'] = false;
			$objectv['longdescription'] = $objectv['name'];
			$objectv['volume'] = gettext("n/a");
			// Get the mount point configuration object to append additional
			// informations, e.g. the devicefile or a modified long
			// description, to the returned objects.
			$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
			  $objectv['mntentref']);
			$meobject = $xmlConfig->get($xpath);
			if (!is_null($meobject) && !empty($meobject)) {
				$fs = new OMVFilesystem($meobject['fsname']);
				if (TRUE === $fs->exists()) {
					$objectv['volume'] = $fs->getDeviceFile();
					$objectv['longdescription'] = sprintf(
					  gettext("%s [on %s]"), $objectv['longdescription'],
					  $fs->getDeviceFile());
				}
			}
			// Is the shared folder referenced by any object?
			if (TRUE === $xmlConfig->exists("//sharedfolderref[" .
			  "contains(.,'{$objectv['uuid']}')]")) {
				$objectv['_used'] = true;
			}
		}
		// Filter result
		return $this->applyFilter($objects, $start, $count, $sortField,
		  $sortDir);
	}

	/**
	 * Get a shared folder config object.
	 */
	function get($uuid) {
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
		$xpath = "//system/shares/sharedfolder[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Build the full path of the shared folder to display it in the
		// WebGUI.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $object['mntentref']);
		$meobject = $xmlConfig->get($xpath);
		if (is_null($meobject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$object['dirpath'] = sprintf("%s/%s", $meobject['dir'],
		  $object['reldirpath']);
		return $object;
	}

	/**
	 * Set (add/update) a shared folder config object.
	 */
	function set($data) {
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
		// Prepare configuration data. Use the name of the shared folder
		// as the relative directory name of the share.
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"name" => $data['name'],
			"comment" => $data['comment'],
			"mntentref" => $data['mntentref'],
			"reldirpath" => $data['name'],
			"umask" => $data['umask']
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness
			if (TRUE === $xmlConfig->exists("//system/shares/sharedfolder[" .
			  "name='{$data['name']}']")) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS);
			}
			// Add empty list of privileges per default
			$object['privileges'] = array();
			// Append object to configuration
			$success = $xmlConfig->set("//system/shares",
			  array("sharedfolder" => $object));
		} else {
			// Get existing configuration object
			$xpath = "//system/shares/sharedfolder[uuid='{$data['uuid']}']";
			$oldObject = $xmlConfig->get($xpath);
			if (is_null($oldObject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Add existing privileges
			$object['privileges'] = $oldObject['privileges'];
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
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder", $object);
	}

	/**
	 * Delete a shared folder config object.
	 * @param uuid The UUID of the configuration object to delete.
	 * @param recursive Remove the shared folder and its content recursively.
	 */
	function delete($uuid, $recursive) {
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
		// Check if the shared folder is used somewhere
		if (TRUE === $xmlConfig->exists("//sharedfolderref[" .
		  "contains(.,'{$uuid}')]")) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INUSE);
		}
		// Get the configuration object
		$xpath = "//system/shares/sharedfolder[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.system.shares.sharedfolder", $object,
		  $recursive);
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
		  "org.openmediavault.system.shares.sharedfolder", $object,
		  $recursive);
	}

	/**
	 * Get the shared folder privileges.
	 * @param uuid The UUID of the shared folder.
	 * @return An array containing the requested privileges.
	 */
	function getPrivileges($uuid) {
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
		// Get shared folder privileges configuration object
		$xpath = "//system/shares/sharedfolder[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$result = array();
		// Get all user configuration objects
		$xpath = "//system/usermanagement/users/user";
		$users = $xmlConfig->getList($xpath);
		if (is_null($users)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		foreach ($users as $userk => $userv) {
			//Skip various users.
			if (in_array($userv['name'], array("admin")))
				continue;
			$privilege = array(
				"type" => "user",
				"name" => $userv['name'],
				"uuid" => $userv['uuid'],
				"perms" => NULL
			);
			// Check if there are any configured privileges for the given user
			if (!empty($object['privileges']['privilege'])) {
				foreach ($object['privileges']['privilege'] as $objprivilegev) {
					if (!array_key_exists('userref', $objprivilegev) ||
					  ($userv['uuid'] !== $objprivilegev['userref'])) {
						continue;
					}
					$privilege['perms'] = intval($objprivilegev['perms']);
				}
			}
			$result[] = $privilege;
		}
		// Get all group configuration objects
		$xpath = "//system/usermanagement/groups/group";
		$groups = $xmlConfig->getList($xpath);
		if (is_null($groups)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		foreach ($groups as $groupk => $groupv) {
			$privilege = array(
				"type" => "group",
				"name" => $groupv['name'],
				"uuid" => $groupv['uuid'],
				"perms" => NULL
			);
			// Check if there are any configured privileges for the given user
			if (!empty($object['privileges']['privilege'])) {
				foreach ($object['privileges']['privilege'] as $objprivilegev) {
					if (!array_key_exists('groupref', $objprivilegev) ||
					  ($groupv['uuid'] !== $objprivilegev['groupref'])) {
						continue;
					}
					$privilege['perms'] = intval($objprivilegev['perms']);
				}
			}
			$result[] = $privilege;
		}
		return $result;
	}

	/**
	 * Set the shared folder privileges.
	 * @param data An array containing the fields \em uuid and \em privileges.
	 * \em uuid is the UUID of the shared folder and \em privileges an array
	 * of privileges to set.
	 */
	function setPrivileges($data) {
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
		// Get shared folder configuration object
		$xpath = "//system/shares/sharedfolder[uuid='{$data['uuid']}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Reset the shared folders privileges.
		$object['privileges'] = array();
		// Prepare the shared folders privileges.
		foreach ($data['privileges'] as $datak => $datav) {
			// Skip entries with perms = NULL. It seems they are not detected
			// by the JSON schema validation.
			if (is_null($datav['perms']))
				continue;
			$privilege = array(
				"perms" => $datav['perms']
			);
			// Privilege for an user or a group?
			switch ($datav['type']) {
			case "user":
				// Check if user exists.
				$xpath = sprintf("//system/usermanagement/users/user".
				  "[uuid='%s']", $datav['uuid']);
				$userObj = $xmlConfig->get($xpath);
				if (is_null($userObj)) {
					throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INVALID,
					  sprintf(gettext("The user '%s' does not exist"),
					  $datav['uuid']));
				}
				// Is user allowed? It does not make sense to give the WebGUI
				// administrator permissions for a shared folder.
				if (in_array($userObj['name'], array("admin"))) {
					throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INVALID,
					  sprintf(gettext("The user '%s' is not allowed"),
					  $userObj['name']));
				}
				$privilege['userref'] = $datav['uuid'];
				break;
			case "group":
				// Check if group exists.
				$xpath = sprintf("//system/usermanagement/groups/group".
				  "[uuid='%s']", $datav['uuid']);
				$groupObj = $xmlConfig->get($xpath);
				if (is_null($groupObj)) {
					throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INVALID,
					  sprintf(gettext("The group '%s' does not exist"),
					  $datav['uuid']));
				}
				$privilege['groupref'] = $datav['uuid'];
				break;
			}
			// Finally add privilege to shared folder privileges.
			$object['privileges']['privilege'][] = $privilege;
		}
		// Update existing configuration object.
		$xpath = "//system/shares/sharedfolder[uuid='{$data['uuid']}']";
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration.
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder.privileges", $object);
	}
}
?>
