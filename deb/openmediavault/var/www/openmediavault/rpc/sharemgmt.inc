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
require_once("openmediavault/system.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class ShareMgmtRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"get" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"set" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"name":{'.$GLOBALS['OMV_JSONSCHEMA_SHARENAME'].'},
					"reldirpath":{"type":"string"},
					"comment":{"type":"string"},
					"mntentref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
					"mode":{"type":"string","enum":["700","750","755",'.
					  '"770","775","777"],"optional":true}
				}
			}'),
			"delete" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
					"recursive":{"type":"boolean"}
				}
			}'),
			"getPath" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"getPrivileges" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"setPrivileges" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
					"privileges":{
						"type":"array",
						"items":{
							"type":"object",
							"properties":{
								"name":{"type":"string"},
								"perms":{"type":"integer","enum":[0,5,7]},
								"type":{"type":"string","enum":["user","group"]}
							}
						}
					}
				}
			}'),
			"getFileACL" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
					"file":{"type":"string"}
				}
			}'),
			"setFileACL" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
					"file":{"type":"string"},
					"recursive":{"type":"boolean"},
					"replace":{"type":"boolean"},
					"user":{"type":"integer","enum":[0,1,2,3,4,5,6,7],'.
					  '"optional":true},
					"group":{"type":"integer","enum":[0,1,2,3,4,5,6,7],'.
					  '"optional":true},
					"other":{"type":"integer","enum":[0,1,2,3,4,5,6,7],'.
					  '"optional":true},
					"users":{
						"type":"array",
						"items":{
							"type":"object",
							"properties":{
								"name":{"type":"string"},
								"perms":{"type":"integer","enum":'.
								  '[0,1,2,3,4,5,6,7]}
							}
						}
					},
					"groups":{
						"type":"array",
						"items":{
							"type":"object",
							"properties":{
								"name":{"type":"string"},
								"perms":{"type":"integer","enum":'.
								  '[0,1,2,3,4,5,6,7]}
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
	 *   \em uuid, \em devicefile and \em description.
	 *   The field \em uuid is the mount point configuration object UUID.
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
			// Get some filesystem details.
			$devicefile = $fs->getDeviceFile();
			$label = $fs->getLabel();
			// Prepare result object.
			$object = array(
				"uuid" => $objectv['uuid'],
				"devicefile" => $devicefile,
				"description" => !empty($label) ? $label : $devicefile
			);
			// Get some more filesystem details if possible.
			if (FALSE !== ($fsStats = $fs->getStatistics())) {
				$object['description'] = sprintf(
				  gettext("%s (%s available)"), $object['description'],
				  binary_format($fsStats['available']));
			}
			$result[] = $object;
		}
		return $result;
	}

	/**
	 * Enumerate all shared folder configuration objects.
	 */
	public function enumerateSharedFolders() {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
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
			$objectv['description'] = $objectv['name'];
			$objectv['volume'] = gettext("n/a");
			// Get the mount point configuration object to append additional
			// informations to the returned objects, e.g. the devicefile or
			// a modified description.
			$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
			  $objectv['mntentref']);
			$meobject = $xmlConfig->get($xpath);
			if (!is_null($meobject) && !empty($meobject)) {
				$fs = new OMVFilesystem($meobject['fsname']);
				if (TRUE === $fs->exists()) {
					// Get some filesystem details.
					$devicefile = $fs->getDeviceFile();
					$label = $fs->getLabel();
					// Override default values.
					$objectv['volume'] = !empty($label) ? $label : $devicefile;
					$objectv['description'] = sprintf(
					  gettext("%s [on %s]"), $objectv['description'],
					  $objectv['volume']);
				}
			}
			// Is the shared folder referenced by any object?
			if (TRUE === $xmlConfig->exists("//sharedfolderref[" .
			  "contains(.,'{$objectv['uuid']}')]")) {
				$objectv['_used'] = true;
			}
		}
		return $objects;
	}

	/**
	 * Get list of shared folder configuration objects.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getList($data) {
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
		// Enumerate all shared folders.
		$objects = OMVRpc::exec("ShareMgmt", "enumerateSharedFolders");
		// Filter result
		return $this->applyFilter($objects, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get a shared folder configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The requested configuration object. The field \em mountpoint
	 *   is the directory where the corresponding filesystem is mounted.
	 */
	public function get($data) {
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
		$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
		  $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Get the mount point configuration object.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $object['mntentref']);
		$meobject = $xmlConfig->get($xpath);
		if (is_null($meobject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$object['mountpoint'] = $meobject['dir'];
		return $object;
	}

	/**
	 * Set (add/update) a shared folder config object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the shared folder object.
	 *   \em name The name of the shared folder.
	 *   \em reldirpath The relative directory path.
	 *   \em comment The comment.
	 *   \em mntentref The UUID of the mount point configuration object wherein
	 *   the shared folder is located.
	 *   \em mode The file mode of the shared folder directory. This field
	 *   is optional.
	 */
	public function set($data) {
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
		// The field 'reldirpath' may not contain the characters '..'. This
		// is because of security reasons: the given canonicalized absolute
		// path MUST be below the given mount point.
		if (1 == preg_match("/\.\./", $data['reldirpath'])) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PARAM,
			  sprintf(gettext("The field '%s' contains forbidden ".
			  "two-dot symbols"), "reldirpath"));
		}
		// Prepare configuration data. Use the name of the shared folder
		// as the relative directory name of the share.
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"name" => $data['name'],
			"comment" => $data['comment'],
			"mntentref" => $data['mntentref'],
			"reldirpath" => $data['reldirpath']
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness. The share name must be global unique because
			// the name is also used when exporting a shared folder via NFS for
			// example.
			$xpath = sprintf("//system/shares/sharedfolder[name='%s']",
			  $data['name']);
			if (TRUE === $xmlConfig->exists($xpath)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				gettext("A shared folder with the given name already exists"));
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
		// Append the file mode field to the notification object if set.
		if (array_key_exists("mode", $data)) {
			$object['mode'] = $data['mode'];
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder", $object);
	}

	/**
	 * Delete a shared folder configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 *   \em recursive Remove the shared folder and its content recursively.
	 * @return None
	 */
	function delete($data) {
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
		$xpath = sprintf("//sharedfolderref[contains(.,'%s')]", $data['uuid']);
		if (TRUE === $xmlConfig->exists($xpath)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INUSE);
		}
		// Get the configuration object
		$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
		  $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Delete shared folder content recursively?
		$recursive = boolval($data['recursive']);
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
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the shared folder configuration object.
	 * @return An array containing the requested privileges.
	 */
	public function getPrivileges($data) {
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
		$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
		  $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Prepare result object.
		$result = array();
		// Process non-system users.
		$users = OMVRpc::exec("UserMgmt", "enumerateUsers");
		foreach ($users as $userk => $userv) {
			$privilege = array(
				"type" => "user",
				"name" => $userv['name'],
				"perms" => NULL
			);
			// Check if there are any configured privileges for the given user
			if (!empty($object['privileges']['privilege'])) {
				foreach ($object['privileges']['privilege'] as $objectv) {
					if (($objectv['type'] === $privilege['type']) &&
					  ($objectv['name'] === $privilege['name'])) {
						$privilege['perms'] = intval($objectv['perms']);
						break;
					}
				}
			}
			$result[] = $privilege;
		}
		// Process non-system groups.
		$groups = OMVRpc::exec("UserMgmt", "enumerateGroups");
		foreach ($groups as $groupk => $groupv) {
			$privilege = array(
				"type" => "group",
				"name" => $groupv['name'],
				"perms" => NULL
			);
			// Check if there are any configured privileges for the given user
			if (!empty($object['privileges']['privilege'])) {
				foreach ($object['privileges']['privilege'] as $objectv) {
					if (($objectv['type'] === $privilege['type']) &&
					  ($objectv['name'] === $privilege['name'])) {
						$privilege['perms'] = intval($objectv['perms']);
						break;
					}
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
	public function setPrivileges($data) {
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
			// Do some checks ...
			switch ($datav['type']) {
			case "user":
				// Check if user exists
				$user = new OMVSystemUser($datav['name']);
				if (!$user->exists()) {
					throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INVALID,
					  sprintf(gettext("The user '%s' does not exist"),
					  $user->getName()));
				}
				// Is user allowed? It does not make sense to give the WebGUI
				// administrator permissions for a shared folder.
				if (in_array($user->getName(), array("admin"))) {
					throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INVALID,
					  sprintf(gettext("The user '%s' is not allowed"),
					  $user->getName()));
				}
				break;
			case "group":
				// Check if group exists
				$group = new OMVSystemGroup($datav['name']);
				if (!$group->exists()) {
					throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INVALID,
					  sprintf(gettext("The group '%s' does not exist"),
					  $group->getName()));
				}
				break;
			}
			// Finally add privilege to shared folder privileges.
			$object['privileges']['privilege'][] = array(
				"type" => $datav['type'],
				"name" => $datav['name'],
				"perms" => $datav['perms']
			);
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

	/**
	 * Get the file access control lists.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the shared folder object.
	 *   \em file The relative path to the file.
	 * @return An object containing the fields \em file, \em owner, \em group
	 *   and the object \em acl with the fields \em user, \em group and
	 *   \em other. The fields \em users and \em groups contain the fields
	 *   \em name, \em uid or \em gid, \em system and \em perms.
	 */
	public function getFileACL($data) {
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
		// The field 'file' may not contain the characters '..'.
		if (1 == preg_match("/\.\./", $data['file'])) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PARAM,
			  sprintf(gettext("The field '%s' contains forbidden ".
			  "two-dot symbols"), "file"));
		}
		// Get the absolute shared folder path.
		$sfpath = OMVRpc::exec("ShareMgmt", "getPath", array(
		  "uuid" => $data['uuid']));
		// Execute command to get the file access control lists.
		$cmd = sprintf("export LANG=C; sudo getfacl --no-effective ".
		  "--access %s 2>&1", escapeshellarg(sprintf("%s/%s", $sfpath,
		  $data['file'])));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Prepare result object.
		$result = array(
			"file" => NULL,
			"owner" => NULL,
			"group" => NULL,
			"acl" => array(
				"user" => NULL,
				"group" => NULL,
				"other" => NULL,
				"users" => array(),
				"groups" => array()
			)
		);
		// Parse the command output:
		// # file: test
		// # owner: root
		// # group: users
		// user::rwx
		// user:test:rwx
		// user:nobody:rwx
		// group::rwx
		// group:users:rwx
		// mask::rwx
		// other::r--
		$object = array();
		// Get the owner and owning group.
		foreach ($output as $outputk => $outputv) {
			$regex = '/^# (file|owner|group): (.+)$/';
			if (1 !== preg_match($regex, $outputv, $matches))
				continue;
			$result[$matches[1]] = $matches[2];
		}
		// Get the user, group and other file mode permission bits.
		foreach ($output as $outputk => $outputv) {
			$regex = '/^(user|group|other):(.*):(.+)$/';
			if (1 !== preg_match($regex, $outputv, $matches))
				continue;
			// Convert permissions string into a number
			$perms = 0;
			$map = array("r" => 4, "w" => "2", "x" => 1, "-" => 0);
			foreach (str_split($matches[3]) as $permk => $permv) {
				if (!array_key_exists($permv, $map))
					continue;
				$perms += $map[$permv];
			}
			if (!empty($matches[2])) {
				$result['acl']["{$matches[1]}s"][] = array(
					"name" => $matches[2],
					"perms" => $perms
				);
			} else {
				$result['acl'][$matches[1]] = $perms;
			}
		}
		// Add missing users.
		$users = OMVRpc::exec("UserMgmt", "enumerateAllUsers");
		foreach ($users as $userk => $userv) {
			$found = FALSE;
			foreach ($result['acl']['users'] as &$resultv) {
				if ($resultv['name'] === $userv['name']) {
					// Append additional user details, e.g. uid and the
					// information if it is a system user.
					$resultv = array_merge($resultv, array(
						"uid" => $userv['uid'],
						"system" => $userv['system']
					));
					$found = TRUE;
					break;
				}
			}
			if (TRUE === $found)
				continue;
			$result['acl']['users'][] = array(
				"name" => $userv['name'],
				"perms" => NULL,
				"uid" => $userv['uid'],
				"system" => $userv['system']
			);
		}
		// Add missing groups.
		$groups = OMVRpc::exec("UserMgmt", "enumerateAllGroups");
		foreach ($groups as $groupk => $groupv) {
			$found = FALSE;
			foreach ($result['acl']['groups'] as &$resultv) {
				if ($resultv['name'] === $groupv['name']) {
					// Append additional group details, e.g. gid and the
					// information if it is a system group.
					$resultv = array_merge($resultv, array(
						"gid" => $groupv['gid'],
						"system" => $groupv['system']
					));
					$found = TRUE;
					break;
				}
			}
			if (TRUE === $found)
				continue;
			$result['acl']['groups'][] = array(
				"name" => $groupv['name'],
				"perms" => NULL,
				"gid" => $groupv['gid'],
				"system" => $groupv['system']
			);
		}
		return $result;
	}

	/**
	 * Set the local directory access control lists.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the shared folder object.
	 *   \em file The relative path to the file.
	 *   \em recursive Apply operations to all files and directories
	 *   \em replace Replace all permissions.
	 *     recursively.
	 *   \em user The owner permissions as octal digit. This field is
	 *     optional.
	 *   \em group The group permissions as octal digit. This field is
	 *     optional.
	 *   \em other The other permissions as octal digit. This field is
	 *     optional.
	 *   \em users An array of arrays with the following fiels:
	 *     \em name The user name.
	 *     \em perms The permissions as octal digit.
	 *   \em groups An array of arrays with the following fiels:
	 *     \em name The group name.
	 *     \em perms The permissions as octal digit.
	 * @return The process ID of the 'setfacl' command.
	 */
	public function setFileACL($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Note, do not commit the session because the PID of the
		// started process will be stored there.
		//$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// The field 'file' may not contain the characters '..'.
		if (1 == preg_match("/\.\./", $data['file'])) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PARAM,
			  sprintf(gettext("The field '%s' contains forbidden ".
			  "two-dot symbols"), "file"));
		}
		// Get the absolute shared folder path.
		$sfpath = OMVRpc::exec("ShareMgmt", "getPath", array(
		  "uuid" => $data['uuid']));
		// Build the ACL specifications.
		$aclspec = array();
		// Set permissions of a named users and groups.
		foreach (array("user", "group") as $typek => $typev) {
			foreach ($data["{$typev}s"] as $entryk => $entryv) {
				$aclspec[] = sprintf("default:%s:%s:%d,%s:%s:%d", $typev,
				  $entryv['name'], $entryv['perms'], $typev,
				  $entryv['name'], $entryv['perms']);
			}
		}
		// Set owner permissions.
		if (array_key_exists("user", $data)) {
			$aclspec[] = sprintf("default:user::%s,user::%s", $data['user'],
			  $data['user']);
		}
		// Set ownning group permissions.
		if (array_key_exists("group", $data)) {
			$aclspec[] = sprintf("default:group::%s,group::%s", $data['group'],
			  $data['group']);
		}
		// Set permissions of others.
		if (array_key_exists("other", $data)) {
			$aclspec[] = sprintf("default:other::%s,other::%s", $data['other'],
			  $data['other']);
		}
		// Build the command arguments.
		$cmdargs = array();
		if (TRUE === boolval($data['replace'])) {
			$cmdargs[] = "--remove-all";
		}
		if (TRUE === boolval($data['recursive'])) {
			$cmdargs[] = "--recursive";
		}
		if (!empty($aclspec)) {
			$cmdargs[] = sprintf("-m %s", implode(",", $aclspec));
		}
		// Execute command to set the file access control lists.
		$outputFile = tempnam(sys_get_temp_dir(), "setfacl");
		$cmd = sprintf("export LANG=C; sudo setfacl %s %s >%s 2>&1 & echo $!",
		  implode(" ", $cmdargs), escapeshellarg(sprintf("%s/%s", $sfpath,
		  $data['file'])), $outputFile);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Remember pid and content filename in session
		$pid = intval($output[0]);
		$_SESSION['execjob'][$pid] = array(
			"pid" => $pid,
			"outputFile" => $outputFile
		);
		return $pid;
	}

	/**
	 * Get the absolute path of a shared folder.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the shared folder object.
	 * @return The path of the shared folder.
	 */
	public function getPath($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get shared folder configuration object
		$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
		  $data['uuid']);
		$sfobject = $xmlConfig->get($xpath);
		if (is_null($sfobject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Get the mount point configuration object to build the absolute
		// shared folder path.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $sfobject['mntentref']);
		$meobject = $xmlConfig->get($xpath);
		if (is_null($meobject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Return the absolute shared folder path.
		return sprintf("%s/%s", $meobject['dir'], $sfobject['reldirpath']);
	}
}
?>
