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
require_once("openmediavault/system.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceShareMgmt extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "ShareMgmt";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getCandidates");
		$this->registerMethod("enumerateSharedFolders");
		$this->registerMethod("getList");
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("delete");
		$this->registerMethod("getPrivileges");
		$this->registerMethod("setPrivileges");
		$this->registerMethod("getFileACL");
		$this->registerMethod("setFileACL");
		$this->registerMethod("getPath");
		$this->registerMethod("getReferencers");
	}

	/**
	 * Get list of mount points that can be used for shared folders.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing objects with the following fields:
	 *   \em uuid, \em devicefile and \em description.
	 *   The field \em uuid is the mount point configuration object UUID.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getCandidates($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get a list of mount points, except bind mounts.
		$xpath = "//system/fstab/mntent[not(contains(opts,'bind'))]";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Get the filesystem details for each mount point.
		$result = array();
		foreach($objects as $objectk => $objectv) {
			$fs = new OMVFilesystem($objectv['fsname']);
			if(!$fs->exists()) {
//				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
//				  sprintf(gettext("Filesystem '%s' not found"),
//				  $objectv['fsname']));
				continue;
			}
			// Skip the filesystems that are not mounted at the moment.
			if(FALSE === $fs->isMounted())
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
			if(FALSE !== ($fsStats = $fs->getStatistics())) {
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
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The list of configured shared folders.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function enumerateSharedFolders($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get all configured shared folder configuration objects.
		$xpath = "//system/shares/sharedfolder";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional informations.
		foreach ($objects as $objectk => &$objectv) {
			// Set the default values.
			$objectv['_used'] = false;
			$objectv['description'] = $objectv['name'];
			$objectv['mntent'] = array(
				"devicefile" => "",
				"fsname" => "",
				"dir" => "",
				"type" => "",
				"posixacl" => FALSE
			);
			// Get the mount point configuration object to append additional
			// information to the returned objects, e.g. the devicefile or
			// a modified description.
			$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
			  $objectv['mntentref']);
			$meObject = $xmlConfig->get($xpath);
			if (!is_null($meObject) && !empty($meObject)) {
				// Add mount point information.
				$objectv['mntent'] = array_merge($objectv['mntent'], array(
					"fsname" => $meObject['fsname'],
					"dir" => $meObject['dir'],
					"type" => $meObject['type']
				));
				$fs = new OMVFilesystem($meObject['fsname']);
				if (TRUE === $fs->exists()) {
					$devicefile = $fs->getDeviceFile();
					$label = $fs->getLabel();
					// Add some filesystem details.
					$objectv['mntent'] = array_merge($objectv['mntent'], array(
						"devicefile" => $devicefile,
						"posixacl" => $fs->hasPosixAclSupport()
					));
					// Override default values.
					$objectv['volume'] = !empty($label) ? $label : $devicefile;
					$objectv['description'] = sprintf(
					  gettext("%s [on %s, %s ]"), $objectv['description'],
					  $objectv['volume'], $objectv['reldirpath']);
				}
			}
			// Is the shared folder referenced by any object? Shared folder
			// references are named 'sharedfolderref'.
			$xpath = sprintf("//*[contains(name(),'sharedfolderref')]".
			  "[contains(.,'%s')]", $objectv['uuid']);
			if (TRUE === $xmlConfig->exists($xpath)) {
				$objectv['_used'] = TRUE;
			}
		}
		return $objects;
	}

	/**
	 * Get list of shared folder configuration objects.
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
	public function getList($params, $context) {
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
		// Enumerate all shared folders.
		$objects = $this->callMethod("enumerateSharedFolders", NULL,
		  $context);
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a shared folder configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object. The field \em mountpoint
	 *   is the directory where the corresponding filesystem is mounted.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function get($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Get the configuration object.
		$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Get the mount point configuration object and add the mount point
		// to the returned shared folder configuration object.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $object['mntentref']);
		$meObject = $xmlConfig->get($xpath);
		if(is_null($meObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$object['mountpoint'] = $meObject['dir'];
		return $object;
	}

	/**
	 * Set (add/update) a shared folder config object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder object.
	 *   \em name The name of the shared folder.
	 *   \em reldirpath The relative directory path.
	 *   \em comment The comment.
	 *   \em mntentref The UUID of the mount point configuration object wherein
	 *   the shared folder is located.
	 *   \em mode The file mode of the shared folder directory. This field
	 *   is optional. Defaults to 775.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_RPC_SERVICE_METHOD_INVALID_PARAMS
	 * @throw E_CONFIG_OBJECT_UNIQUENESS
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 * @throw E_MISC_FAILURE
	 */
	public function set($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
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
		  }');
		// The field 'reldirpath' may not contain the characters '..'. This
		// is because of security reasons: the given canonicalized absolute
		// path MUST be below the given mount point.
		if(1 == preg_match("/\.\./", $params['reldirpath'])) {
			throw new OMVException(
			  OMVErrorMsg::E_RPC_SERVICE_METHOD_INVALID_PARAMS,
			  sprintf(gettext("The field '%s' contains forbidden ".
			  "two-dot symbols"), "reldirpath"));
		}
		// Prepare the configuration object. Use the name of the shared
		// folder as the relative directory name of the share.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"name" => $params['name'],
			"comment" => $params['comment'],
			"mntentref" => $params['mntentref'],
			"reldirpath" => $params['reldirpath']
		);
		// Set the configuration object.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness. The share name must be global unique because
			// the name is also used when exporting a shared folder via NFS for
			// example.
			$xpath = sprintf("//system/shares/sharedfolder[name='%s']",
			  $params['name']);
			if(TRUE === $xmlConfig->exists($xpath)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				gettext("A shared folder with the given name already exists"));
			}
			// Add empty list of privileges per default.
			$object['privileges'] = array();
			// Append object to configuration.
			$success = $xmlConfig->set("//system/shares",
			  array("sharedfolder" => $object));
		} else {
			// Get existing configuration object.
			$xpath = "//system/shares/sharedfolder[uuid='{$params['uuid']}']";
			$oldObject = $xmlConfig->get($xpath);
			if(is_null($oldObject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Add existing privileges.
			$object['privileges'] = $oldObject['privileges'];
			// Update existing configuration object.
			$success = $xmlConfig->replace($xpath, $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Append the file mode field to the notification object if set.
		// Defaults to 775.
		$object['mode'] = "775";
		if(array_key_exists("mode", $params)) {
			$object['mode'] = $params['mode'];
		}
		// Get the mount point configuration object to build the absolute
		// shared folder path.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $object['mntentref']);
		$meObject = $xmlConfig->get($xpath);
		if(is_null($meObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Build the absolute shared folder path.
		$pathName = sprintf("%s/%s", $meObject['dir'], $object['reldirpath']);
		// Create the shared folder directory if necessary.
		if(FALSE === file_exists($pathName)) {
			// Create the directory. Note, the function seems to have a bug
			// when using the mask parameter. E.g. octdec("777") does not
			// create the correct permissions as expected, thus change the
			// mode using chmod.
			if(FALSE === mkdir($pathName, 0700, TRUE)) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("Failed to create the directory '%s'", $pathName));
			}
			// Change the directory mode.
			if(FALSE === chmod($pathName, octdec($object['mode']))) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("Failed to set file mode to '%s' for '%s'",
				  $object['mode'], $pathName));
			}
		}
		// Change group owner of directory to configured default group,
		// e.g. 'users'.
		if(FALSE === chgrp($pathName, $GLOBALS['OMV_USERMGMT_DEFAULT_GROUP'])) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to set file group to '%s' for '%s'",
			  $GLOBALS['OMV_USERMGMT_DEFAULT_GROUP'], $pathName));
		}
		// Set the setgid bit. Setting this permission means that all files
		// created in the folder will inherit the group of the folder rather
		// than the primary group of the user who creates the file.
		$mode = fileperms($pathName) | 02000;
		if(FALSE === chmod($pathName, $mode)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to set file mode to '%o' for '%s'",
			  $mode, $pathName));
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a shared folder configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 *   \em recursive Remove the shared folder and its content recursively.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 * @throw E_CONFIG_OBJECT_INUSE
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function delete($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
				  "recursive":{"type":"boolean"}
			  }
		  }');
		// Check if the shared folder is used somewhere.
		$xpath = sprintf("//*[contains(name(),'sharedfolderref')]".
		  "[contains(.,'%s')]", $params['uuid']);
		if(TRUE === $xmlConfig->exists($xpath)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INUSE);
		}
		// Get the configuration object.
		$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Delete shared folder content recursively?
		$recursive = boolval($params['recursive']);
		// Do some more checks before a shared folder is deleted. Do not
		// delete the shared folder content if a sub-directory is part of
		// another shared folder.
		if(TRUE === $recursive) {
			$xpath = sprintf("//system/shares/sharedfolder[mntentref='%s']",
			  $object['mntentref']);
			$sfObjects = $xmlConfig->getList($xpath);
			if(!is_null($sfObjects)) {
				// Get the mount point configuration object to build the
				// absolute shared folder path.
				$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
				  $object['mntentref']);
				$meObject = $xmlConfig->get($xpath);
				if(is_null($meObject)) {
					throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
					  $xpath);
				}
				// Build the absolute shared folder path.
				$absDirPath = OMVUtil::readlink(sprintf("%s/%s",
				  $meObject['dir'], $object['reldirpath']));
				// Ensure the path ends with an slash, otherwise comparison
				// may be false positive.
				// Example:
				// To delete = /media/4ee04694-e849-4b97-b31a-a928cc084e8f/test/
				// To check  = /media/4ee04694-e849-4b97-b31a-a928cc084e8f/test_folder
				$absDirPath = rtrim($absDirPath, "/")."/";
				foreach($sfObjects as $sfObjectk => $sfObjectv) {
					// Skip current processed shared folder configuration
					// object.
					if($sfObjectv['uuid'] === $object['uuid'])
						continue;
					// Check if the shared folder to be deleted is a top
					// directory of another shared folder configuration object.
					// In this case throw an exception because unlinking the
					// content recursively will delete the content of another
					// shared folder.
					//
					// Example:
					// <volume>
					//  |-dir1
					//  |-dir2
					//  |  |-dir2.1
					//  |  '-dir2.x
					//  .
					//
					// Deleting dir2 recursively will be forbidden when dir2.1
					// is used by another shared folder.
					$sfAbsDirPath = OMVUtil::readlink(sprintf("%s/%s",
					  $meObject['dir'], $sfObjectv['reldirpath']));
					if(0 === stripos($sfAbsDirPath, $absDirPath)) {
						throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
						  gettext("Cannot unlink shared folder content because another shared folder is referencing a subdirectory"));
					}
				}
			}
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.system.shares.sharedfolder", $object,
		  $recursive);
		// Remove the shared folder and its content recursively? 
		if(TRUE === $recursive) {
			// Get the mount point configuration object to build the absolute
			// shared folder path.
			$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
			  $object['mntentref']);
			$meObject = $xmlConfig->get($xpath);
			if(is_null($meObject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Build the absolute shared folder path.
			$dirPath = sprintf("%s/%s", $meObject['dir'], $object['reldirpath']);
			// Delete the shared folder directory.
			$cmd = sprintf("rm -fr %s 2>&1", escapeshellarg($dirPath));
			if(0 !== $this->exec($cmd, $output)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
		}
		// Delete the configuration object.
		$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
		  $params['uuid']);
		if(FALSE === ($object = $xmlConfig->delete($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
			  $xpath);
		}
		// Notify configuration changes
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.shares.sharedfolder", $object);
		// Return the deleted configuration object.
		return $object;
	}

	/**
	 * Get the shared folder privileges.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder configuration object.
	 * @param context The context of the caller.
	 * @return An array containing the requested privileges.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getPrivileges($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Get shared folder privileges configuration object.
		$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Prepare result object.
		$result = array();
		// Process non-system users.
		$users = OMVRpc::exec("UserMgmt", "enumerateUsers", NULL, $context);
		foreach($users as $userk => $userv) {
			$privilege = array(
				"type" => "user",
				"name" => $userv['name'],
				"perms" => NULL
			);
			// Check if there are any configured privileges for the
			// given user.
			if(!empty($object['privileges']['privilege'])) {
				foreach($object['privileges']['privilege'] as $objectv) {
					if(($objectv['type'] === $privilege['type']) &&
					  ($objectv['name'] === $privilege['name'])) {
						$privilege['perms'] = intval($objectv['perms']);
						break;
					}
				}
			}
			$result[] = $privilege;
		}
		// Process non-system groups.
		$groups = OMVRpc::exec("UserMgmt", "enumerateGroups", NULL, $context);
		foreach($groups as $groupk => $groupv) {
			$privilege = array(
				"type" => "group",
				"name" => $groupv['name'],
				"perms" => NULL
			);
			// Check if there are any configured privileges for the
			// given group.
			if(!empty($object['privileges']['privilege'])) {
				foreach($object['privileges']['privilege'] as $objectv) {
					if(($objectv['type'] === $privilege['type']) &&
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
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder.
	 *   \em privileges An array containing the privileges to be set.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_MISC_FAILURE
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	public function setPrivileges($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
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
		  }');
		// Get shared folder configuration object.
		$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Reset the shared folders privileges.
		$object['privileges'] = array();
		// Prepare the shared folders privileges.
		foreach($params['privileges'] as $privilegek => $privilegev) {
			// Skip entries with perms = NULL. It seems they are not detected
			// by the JSON schema validation.
			if(is_null($privilegev['perms']))
				continue;
			// Do some checks ...
			switch($privilegev['type']) {
			case "user":
				// Check if user exists
				$user = new OMVUser($privilegev['name']);
				if(!$user->exists()) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf(gettext("The user '%s' does not exist"),
					  $user->getName()));
				}
				// Is user allowed? It does not make sense to give the WebGUI
				// administrator permissions for a shared folder.
				if(in_array($user->getName(), array("admin"))) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf(gettext("The user '%s' is not allowed"),
					  $user->getName()));
				}
				break;
			case "group":
				// Check if group exists.
				$group = new OMVUserGroup($privilegev['name']);
				if(!$group->exists()) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf(gettext("The group '%s' does not exist"),
					  $group->getName()));
				}
				break;
			}
			// Finally add privilege to shared folder privileges.
			$object['privileges']['privilege'][] = array(
				"type" => $privilegev['type'],
				"name" => $privilegev['name'],
				"perms" => $privilegev['perms']
			);
		}
		// Update existing configuration object.
		$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
		  $params['uuid']);
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder.privileges",
		  $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Get the file access control lists.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder object.
	 *   \em file The relative path to the file.
	 * @param context The context of the caller.
	 * @return An object containing the fields \em file, \em owner, \em group
	 *   and the object \em acl with the fields \em user, \em group and
	 *   \em other. The fields \em users and \em groups contain the fields
	 *   \em name, \em uid or \em gid, \em system and \em perms.
	 * @throw E_RPC_SERVICE_METHOD_INVALID_PARAMS
	 * @throw E_EXEC_FAILED
	 */
	public function getFileACL($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
				  "file":{"type":"string"}
			  }
		  }');
		// The field 'file' may not contain the characters '..'.
		if(1 == preg_match("/\.\./", $params['file'])) {
			throw new OMVException(
			  OMVErrorMsg::E_RPC_SERVICE_METHOD_INVALID_PARAMS,
			  sprintf(gettext("The field '%s' contains forbidden ".
			  "two-dot symbols"), "file"));
		}
		// Get the absolute shared folder path.
		$sfpath = OMVRpc::exec("ShareMgmt", "getPath", array(
		  "uuid" => $params['uuid']), $context);
		// Execute command to get the file access control lists.
		$cmd = sprintf("export LANG=C; getfacl --no-effective ".
		  "--access -- %s 2>&1", escapeshellarg(sprintf("%s/%s", $sfpath,
		  $params['file'])));
		if(0 !== $this->exec($cmd, $output)) {
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
		foreach($output as $outputk => $outputv) {
			$regex = '/^# (file|owner|group): (.+)$/';
			if(1 !== preg_match($regex, $outputv, $matches))
				continue;
			$result[$matches[1]] = $matches[2];
		}
		// Get the user, group and other file mode permission bits.
		foreach($output as $outputk => $outputv) {
			$regex = '/^(user|group|other):(.*):(.+)$/';
			if(1 !== preg_match($regex, $outputv, $matches))
				continue;
			// Convert permissions string into a number
			$perms = 0;
			$map = array("r" => 4, "w" => "2", "x" => 1, "-" => 0);
			foreach(str_split($matches[3]) as $permk => $permv) {
				if(!array_key_exists($permv, $map))
					continue;
				$perms += $map[$permv];
			}
			if(!empty($matches[2])) {
				$result['acl']["{$matches[1]}s"][] = array(
					// Convert special characters, e.g.
					// group:Domain\040Computers:---
					"name" => str_replace('\040', ' ', $matches[2]),
					"perms" => $perms
				);
			} else {
				$result['acl'][$matches[1]] = $perms;
			}
		}
		// Add missing users.
		$users = OMVRpc::exec("UserMgmt", "enumerateAllUsers", NULL, $context);
		foreach($users as $userk => $userv) {
			$found = FALSE;
			foreach($result['acl']['users'] as &$resultv) {
				if($resultv['name'] === $userv['name']) {
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
			if(TRUE === $found)
				continue;
			$result['acl']['users'][] = array(
				"name" => $userv['name'],
				"perms" => NULL,
				"uid" => $userv['uid'],
				"system" => $userv['system']
			);
		}
		// Add missing groups.
		$groups = OMVRpc::exec("UserMgmt", "enumerateAllGroups", NULL, $context);
		foreach($groups as $groupk => $groupv) {
			$found = FALSE;
			foreach($result['acl']['groups'] as &$resultv) {
				if($resultv['name'] === $groupv['name']) {
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
			if(TRUE === $found)
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
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder object.
	 *   \em file The relative path to the file.
	 *   \em recursive Apply operations to all files and directories
	 *   \em replace Replace all permissions.
	 *     recursively.
	 *   \em userperms The owner permissions as octal digit. This field is
	 *     optional.
	 *   \em groupperms The group permissions as octal digit. This field is
	 *     optional.
	 *   \em otherperms The other permissions as octal digit. This field is
	 *     optional.
	 *   \em users An array of arrays with the following fiels:
	 *     \em name The user name.
	 *     \em perms The permissions as octal digit.
	 *   \em groups An array of arrays with the following fiels:
	 *     \em name The group name.
	 *     \em perms The permissions as octal digit.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw E_RPC_SERVICE_METHOD_INVALID_PARAMS
	 * @throw E_EXEC_FAILED
	 */
	public function setFileACL($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
				  "file":{"type":"string"},
				  "recursive":{"type":"boolean"},
				  "replace":{"type":"boolean"},
				  "userperms":{"type":"integer","enum":[0,1,2,3,4,5,6,7],'.
					'"optional":true},
				  "groupperms":{"type":"integer","enum":[0,1,2,3,4,5,6,7],'.
					'"optional":true},
				  "otherperms":{"type":"integer","enum":[0,1,2,3,4,5,6,7],'.
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
		  }');
		// The field 'file' may not contain the characters '..'.
		if(1 == preg_match("/\.\./", $params['file'])) {
			throw new OMVException(
			  OMVErrorMsg::E_RPC_SERVICE_METHOD_INVALID_PARAMS,
			  sprintf(gettext("The field '%s' contains forbidden ".
			  "two-dot symbols"), "file"));
		}
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
			// Get the absolute shared folder path.
			$sfpath = OMVRpc::exec("ShareMgmt", "getPath", array(
			  "uuid" => $params['uuid']), $context);
			// Build the ACL specifications.
			$aclspec = array();
			// Set permissions of a named users and groups.
			foreach(array("user", "group") as $typek => $typev) {
				foreach($params["{$typev}s"] as $entryk => $entryv) {
					$aclspec[] = sprintf("default:%s:%s:%d,%s:%s:%d",
					  $typev, $entryv['name'], $entryv['perms'],
					  $typev, $entryv['name'], $entryv['perms']);
				}
			}
			// Set default owner permissions.
			if(array_key_exists("userperms", $params)) {
				$aclspec[] = sprintf("default:user::%d,user::%d",
				  $params['userperms'], $params['userperms']);
			}
			// Set default owning group permissions.
			if(array_key_exists("groupperms", $params)) {
				$aclspec[] = sprintf("default:group::%d,group::%d",
				  $params['groupperms'], $params['groupperms']);
			}
			// Set default permissions of others.
			if(array_key_exists("otherperms", $params)) {
				$aclspec[] = sprintf("default:other::%d,other::%d",
				  $params['otherperms'], $params['otherperms']);
			}
			// Build the command arguments.
			$cmdargs = array();
			if(TRUE === boolval($params['replace'])) {
				$cmdargs[] = "--remove-all";
			}
			if(TRUE === boolval($params['recursive'])) {
				$cmdargs[] = "--recursive";
			}
			if(!empty($aclspec)) {
				$cmdargs[] = sprintf("-m %s", escapeshellarg(
				  implode(",", $aclspec)));
			}
			// Execute command to set the file access control lists.
			$cmd = sprintf("export LANG=C; setfacl %s -- %s 2>&1",
			  implode(" ", $cmdargs), escapeshellarg(sprintf("%s/%s",
			  $sfpath, $params['file'])));
			if(0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			$this->finalizeBgProcStatus($bgStatusFilename, $output);
			exit(0);
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}

	/**
	 * Get the absolute path of a shared folder.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder object.
	 * @param context The context of the caller.
	 * @return The path of the shared folder.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getPath($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Get shared folder configuration object.
		$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
		  $params['uuid']);
		$sfObject = $xmlConfig->get($xpath);
		if(is_null($sfObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Get the mount point configuration object to build the absolute
		// shared folder path.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $sfObject['mntentref']);
		$meObject = $xmlConfig->get($xpath);
		if(is_null($meObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Return the absolute shared folder path.
		return sprintf("%s/%s", $meObject['dir'], $sfObject['reldirpath']);
	}

	/**
	 * Get the XPath location path to the configuration objects referencing
	 * a shared folder configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder configuration object.
	 * @param context The context of the caller.
	 * @return An array containing the XPath location path of the referencing
	 *   configuration objects.
	 */
	public function getReferencers($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Get a list of XPath's to the configuration objects referencing
		// the given shared folder. Shared folder references are named
		// 'sharedfolderref'.
		$xpath = sprintf("//*[contains(name(),'sharedfolderref')]".
		  "[contains(.,'%s')]/..", $params['uuid']);
		$objects = $xmlConfig->getXPathList($xpath);
		return $objects;
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceShareMgmt());
?>
