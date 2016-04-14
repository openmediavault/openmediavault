<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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
require_once("openmediavault/config.inc");
require_once("openmediavault/functions.inc");

class OMVRpcServiceShareMgmt extends \OMV\Rpc\ServiceAbstract {
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
		$this->registerMethod("getPrivilegesByRole");
		$this->registerMethod("setPrivilegesByRole");
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
	 */
	public function getCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get a list of mount points, except bind mounts.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.system.filesystem.mountpoint", [
			  "operator" => "not",
			  "arg0" => [
				  "operator" => "stringContains",
				  "arg0" => "opts",
				  "arg1" => "bind"
			  ]
		  ]);
		// Get the filesystem details for each mount point.
		$result = array();
		foreach ($objects as $objectk => $objectv) {
			$fs = \OMV\System\Filesystem\Filesystem::getImpl(
			  $objectv->get("fsname"));
			if (is_null($fs) || !$fs->exists())
				continue;
			// Skip the filesystems that are not mounted at the moment.
			if (FALSE === $fs->isMounted())
				continue;
			// Get some filesystem details.
			$devicefile = $fs->getDeviceFile();
			$label = $fs->getLabel();
			// Prepare result object.
			$object = array(
				"uuid" => $objectv->get("uuid"),
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
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The list of configured shared folders.
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
			$objectv['volume'] = "";
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
				// Add some mount point information.
				$objectv['mntent'] = array_merge($objectv['mntent'], array(
					"fsname" => $meObject['fsname'],
					"dir" => $meObject['dir'],
					"type" => $meObject['type']
				));
				// Get the filesystem backend.
				$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
				$fsb = $fsbMngr->getBackendByType($meObject['type']);
				if (is_null($fsb)) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf("No filesystem backend exists for '%s'",
					  $meObject['type']));
				}
				// Does the filesystem support POSIX ACL?
				$objectv['mntent'] = array_merge($objectv['mntent'], array(
					"posixacl" => $fsb->hasPosixAclSupport()
				));
				// Get the filesystem implementation.
				$fs = $fsb->getImpl($meObject['fsname']);
				if (!is_null($fs) && $fs->exists()) {
					$devicefile = $fs->getDeviceFile();
					$label = $fs->getLabel();
					// Add some filesystem details.
					$objectv['mntent'] = array_merge($objectv['mntent'], array(
						"devicefile" => $devicefile
					));
					// Override default values.
					$objectv['volume'] = !empty($label) ? $label : $devicefile;
					$objectv['description'] = sprintf(
					  gettext("%s [on %s, %s]"), $objectv['description'],
					  $objectv['volume'], $objectv['reldirpath']);
				}
			}
			// Is the shared folder referenced by any object? Shared folder
			// references are named 'sharedfolderref'.
			$xpath = sprintf("//*[contains(name(),'sharedfolderref')]".
			  "[contains(.,'%s')]", $objectv['uuid']);
			if (TRUE === $xmlConfig->exists($xpath))
				$objectv['_used'] = TRUE;
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
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Enumerate all shared folders.
		$objects = $this->callMethod("enumerateSharedFolders", NULL,
		  $context);
		// Filter the result.
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
	 */
	public function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.sharedfolder", $params['uuid']);
		// Get the mount point configuration object and add the mount point
		// to the returned shared folder configuration object.
		$meObject = $db->get("conf.system.filesystem.mountpoint",
		  $object->get("mntentref"));
		$object->add("mountpoint", "string");
		$object->set("mountpoint", $meObject->get("dir"));
		return $object->getAssoc();
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
		if (1 == preg_match("/\.\./", $params['reldirpath'])) {
			throw new OMVException(
			  OMVErrorMsg::E_RPC_SERVICE_METHOD_INVALID_PARAMS,
			  sprintf(gettext("The field '%s' contains forbidden ".
			  "two-dot symbols"), "reldirpath"));
		}
		// Prepare the configuration object. Use the name of the shared
		// folder as the relative directory name of the share.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  \OMV\Uuid::uuid4() : $params['uuid'],
			"name" => $params['name'],
			"comment" => $params['comment'],
			"mntentref" => $params['mntentref'],
			"reldirpath" => sprintf("%s/", trim($params['reldirpath'], "/"))
		);
		// Set the configuration object.
		$success = FALSE;
		if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness. The share name must be global unique because
			// the name is also used when exporting a shared folder via NFS for
			// example.
			$xpath = sprintf("//system/shares/sharedfolder[name='%s']",
			  $params['name']);
			if (TRUE === $xmlConfig->exists($xpath)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				gettext("A shared folder with the given name already exists"));
			}
/*
			// Check if there already exists a shared folder on the same
			// volume with the same relative directory path.
			$xpath = sprintf("//system/shares/sharedfolder[mntentref='%s' ".
			  "and reldirpath='%s']", $object['mntentref'],
			  $object['reldirpath']);
			if (TRUE === $xmlConfig->exists($xpath)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				gettext("A shared folder with the given path already exists"));
			}
*/
			// Add empty list of privileges per default.
			$object['privileges'] = array();
			// Append object to configuration.
			$success = $xmlConfig->set("//system/shares",
			  array("sharedfolder" => $object));
		} else {
/*
			// Check if there already exists a shared folder on the same
			// volume with the same relative directory path.
			$xpath = sprintf("//system/shares/sharedfolder[uuid!='%s' and ".
			  "mntentref='%s' and reldirpath='%s']", $object['uuid'],
			  $object['mntentref'], $object['reldirpath']);
			if (TRUE === $xmlConfig->exists($xpath)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				gettext("A shared folder with the given path already exists"));
			}
*/
			// Get the existing configuration object.
			$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
			  $params['uuid']);
			$oldObject = $xmlConfig->get($xpath);
			if (is_null($oldObject)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Add existing privileges.
			$object['privileges'] = $oldObject['privileges'];
			// Update existing configuration object.
			$success = $xmlConfig->replace($xpath, $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Append the file mode field to the notification object if set.
		// Defaults to 775.
		$object['mode'] = "775";
		if (array_key_exists("mode", $params)) {
			$object['mode'] = $params['mode'];
		}
		// Get the mount point configuration object to build the absolute
		// shared folder path.
		$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
		  $object['mntentref']);
		$meObject = $xmlConfig->get($xpath);
		if (is_null($meObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Build the absolute shared folder path.
		$pathName = build_path(DIRECTORY_SEPARATOR, $meObject['dir'],
		  $object['reldirpath']);
		// Create the shared folder directory if necessary.
		if (FALSE === file_exists($pathName)) {
			// Create the directory. Note, the function seems to have a bug
			// when using the mask parameter. E.g. octdec("777") does not
			// create the correct permissions as expected, thus change the
			// mode using chmod.
			if (FALSE === mkdir($pathName, 0700, TRUE)) {
				$error = error_get_last();
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("Failed to create the directory '%s': %s",
				  $pathName, $error['message']));
			}
			// Change the directory mode.
			if (FALSE === chmod($pathName, octdec($object['mode']))) {
				$error = error_get_last();
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("Failed to set file mode to '%s' for '%s': %s",
				  $object['mode'], $pathName, $error['message']));
			}
		}
		// Change group owner of directory to configured default group,
		// e.g. 'users'.
		if (FALSE === chgrp($pathName, $GLOBALS['OMV_USERMGMT_DEFAULT_GROUP'])) {
			$error = error_get_last();
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to set file group to '%s' for '%s': %s",
			  $GLOBALS['OMV_USERMGMT_DEFAULT_GROUP'], $pathName,
			  $error['message']));
		}
		// Set the setgid bit. Setting this permission means that all files
		// created in the folder will inherit the group of the folder rather
		// than the primary group of the user who creates the file.
		$mode = fileperms($pathName) | 02000;
		if (FALSE === chmod($pathName, $mode)) {
			$error = error_get_last();
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to set file mode to '%o' for '%s': %s",
			  $mode, $pathName, $error['message']));
		}
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
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
		if (TRUE === $xmlConfig->exists($xpath)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INUSE);
		}
		// Get the configuration object.
		$xpath = sprintf("//system/shares/sharedfolder[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Delete shared folder content recursively?
		$recursive = boolvalEx($params['recursive']);
		// Do some more checks before a shared folder is deleted.
		if (TRUE === $recursive) {
			// Do not delete the shared folder content if another shared
			// folder points to the same directory.
			$xpath = sprintf("//system/shares/sharedfolder[uuid!='%s' and ".
			  "mntentref='%s' and reldirpath='%s']", $object['uuid'],
			  $object['mntentref'], $object['reldirpath']);
			if (TRUE === $xmlConfig->exists($xpath)) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  gettext("Cannot unlink the shared folder content because another shared folder also refers to it"));
			}
			// Do not delete the shared folder content if a sub-directory
			// is part of another shared folder.
			$xpath = sprintf("//system/shares/sharedfolder[mntentref='%s']",
			  $object['mntentref']);
			$sfObjects = $xmlConfig->getList($xpath);
			if (!is_null($sfObjects)) {
				// Get the mount point configuration object to build the
				// absolute shared folder path.
				$xpath = sprintf("//system/fstab/mntent[uuid='%s']",
				  $object['mntentref']);
				$meObject = $xmlConfig->get($xpath);
				if (is_null($meObject)) {
					throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
					  $xpath);
				}
				// Build the absolute shared folder path.
				$absDirPath = \OMV\System\Os::readlink(build_path(
				  DIRECTORY_SEPARATOR, $meObject['dir'],
				  $object['reldirpath']));
				// Ensure the path ends with an slash, otherwise comparison
				// may be false positive.
				// Example:
				// To delete = /media/4ee04694-e849-4b97-b31a-a928cc084e8f/test/
				// To check  = /media/4ee04694-e849-4b97-b31a-a928cc084e8f/test_folder
				$absDirPath = sprintf("%s/", rtrim($absDirPath, "/"));
				foreach ($sfObjects as $sfObjectk => $sfObjectv) {
					// Skip current processed shared folder configuration
					// object.
					if ($sfObjectv['uuid'] === $object['uuid'])
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
					$sfAbsDirPath = \OMV\System\Os::readlink(build_path(
					  DIRECTORY_SEPARATOR, $meObject['dir'],
					  $sfObjectv['reldirpath']));
					if (0 === stripos($sfAbsDirPath, $absDirPath)) {
						throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
						  gettext("Cannot unlink the shared folder content because another shared folder is referencing a subdirectory"));
					}
				}
			}
		}
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
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
			$dirPath = build_path(DIRECTORY_SEPARATOR, $meObject['dir'],
			  $object['reldirpath']);
			// Delete the shared folder directory.
			$cmd = sprintf("rm -fr %s 2>&1", escapeshellarg($dirPath));
			if (0 !== $this->exec($cmd, $output))
				throw new \OMV\ExecException($cmd, $output);
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
	 */
	public function getPrivileges($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
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
		$users = \OMV\Rpc\Rpc::call("UserMgmt", "enumerateUsers", NULL, $context);
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
		$groups = \OMV\Rpc\Rpc::call("UserMgmt", "enumerateGroups", NULL, $context);
		foreach ($groups as $groupk => $groupv) {
			// Set default values.
			$privilege = array(
				"type" => "group",
				"name" => $groupv['name'],
				"perms" => NULL
			);
			// Check if there are any configured privileges for the
			// given group.
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
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder.
	 *   \em privileges An array containing the privileges to be set.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
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
				$user = new \OMV\System\User($privilegev['name']);
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
				$group = new \OMV\System\Group($privilegev['name']);
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
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.shares.sharedfolder.privileges",
		  $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Get the shared folder privileges for the given role.
	 * @param params An array containing the following fields:
	 *   \em role The role type, e.g. 'user' or 'group'.
	 *   \em name The name of the user or group.
	 * @param context The context of the caller.
	 * @return An array of objects with the following fields:
	 *   uuid, name, perms.
	 */
	public function getPrivilegesByRole($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "role":{"type":"string","enum":["user","group"]},
				  "name":{"type":"string"}
			  }
		  }');
		// Get all configured shared folder configuration objects.
		$xpath = "//system/shares/sharedfolder";
		$sfObjects = $xmlConfig->getList($xpath);
		if (is_null($sfObjects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		$result = array();
		foreach ($sfObjects as $sfk => $sfv) {
			// Set default values.
			$privilege = array(
				"uuid" => $sfv['uuid'],
				"name" => $sfv['name'],
				"perms" => NULL
			);
			// Check if there are any configured privileges for the
			// given user/group.
			foreach ($sfv['privileges']['privilege'] as $privk => $privv) {
				if ($params['role'] !== $privv['type'])
					continue;
				if ($params['name'] !== $privv['name'])
					continue;
				$privilege['perms'] = intval($privv['perms']);
				break;
			}
			$result[] = $privilege;
		}
		return $result;
	}

	/**
	 * Set the shared folder privileges for the given role.
	 * @param params An array containing the following fields:
	 *   \em role The role type, e.g. 'user' or 'group'.
	 *   \em name The name of the user or group.
	 *   \em privileges An array of privileges with the fields:
	 *     \em uuid The shared folder UUID.
	 *     \em perms The privileges: -1 (undef), 0 (none), 5 (r) or 7 (r/w).
	 * @return An array of the stored configuration objects.
	 */
	public function setPrivilegesByRole($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "role":{"type":"string","enum":["user","group"]},
				  "name":{"type":"string"},
				  "privileges":{
					  "type":"array",
					  "items":{
						  "type":"object",
						  "properties":{
							  "uuid":{"type":"string"},
							  "perms":{"type":"integer","enum":[-1,0,5,7]}
						  }
					  }
				  }
			  }
		  }');
		$result = array();
		foreach ($params['privileges'] as $privk => $privv) {
			// Get the privilege configuration for the given shared folder.
			$sfPriv = $this->callMethod("getPrivileges", array(
			  "uuid" => $privv['uuid']), $context);
			// Rebuild the shared folder privileges using the given
			// configuration.
			$objects = array();
			foreach ($sfPriv as $sfPrivk => $sfPrivv) {
				// Do we have to update or delete the processing privilege?
				if (($sfPrivv['type'] == $params['role']) &&
				  ($sfPrivv['name'] == $params['name'])) {
					if (-1 == $privv['perms']) // Delete the privilege.
						continue;
					// Update the privilege.
					$sfPrivv['perms'] = $privv['perms'];
				}
				$objects[] = $sfPrivv;
			}
			// Update the shared folder privileges.
			$result[] = $this->callMethod("setPrivileges", array(
			  "uuid" => $privv['uuid'], "privileges" => $objects),
			  $context);
		}
		return $result;
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
		$sfpath = \OMV\Rpc\Rpc::call("ShareMgmt", "getPath", array(
		  "uuid" => $params['uuid']), $context);
		// Execute command to get the file access control lists.
		$cmd = sprintf("export LANG=C; getfacl --no-effective ".
		  "--access -- %s 2>&1", escapeshellarg(build_path(
		  DIRECTORY_SEPARATOR, $sfpath, $params['file'])));
		if (0 !== $this->exec($cmd, $output))
			throw new \OMV\ExecException($cmd, $output);
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
		$users = \OMV\Rpc\Rpc::call("UserMgmt", "enumerateAllUsers", NULL, $context);
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
		$groups = \OMV\Rpc\Rpc::call("UserMgmt", "enumerateAllGroups", NULL, $context);
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
	 *     recursively.
	 *   \em replace Replace all permissions.
	 *   \em owner The directory/file owner. This field is optional.
	 *   \em group The directory/file group. This field is optional.
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
				  "owner":{"type":"string","optional":true},
				  "group":{"type":"string","optional":true},
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
		$aclspecFile = tempnam(sys_get_temp_dir(), "setfacl");
		try {
			$bgOutputFilename = $this->createBgProcOutput();
			$this->updateBgProcStatus($bgStatusFilename, "outputfilename",
			  $bgOutputFilename);
			// Get the absolute shared folder path.
			$sfpath = \OMV\Rpc\Rpc::call("ShareMgmt", "getPath", array(
			  "uuid" => $params['uuid']), $context);
			////////////////////////////////////////////////////////////////////
			// 1. Update the directory/file owner and group
			////////////////////////////////////////////////////////////////////
			// Set the directory owner and group.
			if (array_key_exists("owner", $params) || array_key_exists(
			  "group", $params)) {
				$ownerGroupArg = "";
				if (array_key_exists("owner", $params))
					$ownerGroupArg = str_replace(' ', '\ ', $params['owner']);
				if (array_key_exists("group", $params))
					$ownerGroupArg = sprintf("%s:%s", $ownerGroupArg,
					  str_replace(' ', '\ ', $params['group']));
				// Build the command arguments list.
				$cmdargs = array();
				if (TRUE === boolvalEx($params['recursive']))
					$cmdargs[] = "--recursive";
				$cmdargs[] = $ownerGroupArg;
				// Execute command to set the file owner/group.
				$cmd = sprintf("export LANG=C; chown %s %s 2>&1", implode(
				  " ", $cmdargs), escapeshellarg(build_path(
				  DIRECTORY_SEPARATOR, $sfpath, $params['file'])));
				if (0 !== $this->exec($cmd, $output, $bgOutputFilename))
					throw new \OMV\ExecException($cmd, $output);
			}
			////////////////////////////////////////////////////////////////////
			// 2. Update the directory/file access control lists
			////////////////////////////////////////////////////////////////////
			// Build the ACL specifications.
			$aclspec = array();
			// Set permissions of a named users and groups.
			foreach (array("user", "group") as $typek => $typev) {
				foreach($params["{$typev}s"] as $entryk => $entryv) {
					$aclspec[] = sprintf("default:%s:%s:%d", $typev,
					  $entryv['name'], $entryv['perms']);
					$aclspec[] = sprintf("%s:%s:%d", $typev,
					  $entryv['name'], $entryv['perms']);
				}
			}
			// Set default owner permissions.
			if (array_key_exists("userperms", $params)) {
				$aclspec[] = sprintf("default:user::%d", $params['userperms']);
				$aclspec[] = sprintf("user::%d", $params['userperms']);
			}
			// Set default owning group permissions.
			if (array_key_exists("groupperms", $params)) {
				$aclspec[] = sprintf("default:group::%d", $params['groupperms']);
				$aclspec[] = sprintf("group::%d", $params['groupperms']);
			}
			// Set default permissions of others.
			if (array_key_exists("otherperms", $params)) {
				$aclspec[] = sprintf("default:other::%d", $params['otherperms']);
				$aclspec[] = sprintf("other::%d", $params['otherperms']);
			}
			// Build the command arguments list.
			$cmdargs = array();
			if (TRUE === boolvalEx($params['replace']))
				$cmdargs[] = "--remove-all";
			if (TRUE === boolvalEx($params['recursive']))
				$cmdargs[] = "--recursive";
			if (!empty($aclspec)) {
				if (FALSE === file_put_contents($aclspecFile, implode("\n",
				  $aclspec))) {
					throw new \OMV\Exception(
					  "Failed to write ACL entries to file");
				}
				// Read the ACL entries from file. The CLI argument list
				// is limited which will cause an error if the user/group
				// list is too large.
				$cmdargs[] = sprintf("-M %s", escapeshellarg($aclspecFile));
			}
			// Execute command to set the file access control lists.
			$cmd = sprintf("export LANG=C; setfacl %s -- %s 2>&1", implode(
			  " ", $cmdargs), escapeshellarg(build_path(DIRECTORY_SEPARATOR,
			  $sfpath, $params['file'])));
			if (0 !== $this->exec($cmd, $output, $bgOutputFilename))
				throw new \OMV\ExecException($cmd, $output);
			$this->finalizeBgProcStatus($bgStatusFilename, $output);
			// Cleanup ACL entries file.
			unlink($aclspecFile);
			exit(0);
		} catch(\Exception $e) {
			// Cleanup ACL entries file.
			unlink($aclspecFile);
			// Update the process status.
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
	 */
	public function getPath($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get shared folder configuration object.
		$db = \OMV\Config\Database::getInstance();
		$sfObject = $db->get("conf.system.sharedfolder", $params['uuid']);
		// Get the mount point configuration object to build the absolute
		// shared folder path.
		$meObject = $db->get("conf.system.filesystem.mountpoint",
		  $sfObject->get("mntentref"));
		// Return the absolute shared folder path.
		return build_path(DIRECTORY_SEPARATOR, $meObject->get("dir"),
		  $sfObject->get("reldirpath"));
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
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get a list of XPath's to the configuration objects referencing
		// the given shared folder. Shared folder references are named
		// 'sharedfolderref'.
		$xpath = sprintf("//*[contains(name(),'sharedfolderref')]".
		  "[contains(.,'%s')]/..", $params['uuid']);
		$objects = $xmlConfig->getXPathList($xpath);
		return $objects;
	}
}
