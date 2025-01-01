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

class ShareMgmt extends \OMV\Rpc\ServiceAbstract {
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
		$this->registerMethod("copyPrivileges");
		$this->registerMethod("getFileACL");
		$this->registerMethod("setFileACL");
		$this->registerMethod("getPath");
		$this->registerMethod("enumerateSnapshots");
		$this->registerMethod("enumerateAllSnapshots");
		$this->registerMethod("createSnapshot");
		$this->registerMethod("createScheduledSnapshotTask");
		$this->registerMethod("enumerateScheduledSnapshotTasks");
		$this->registerMethod("enumerateAllScheduledSnapshotTasks");
		$this->registerMethod("deleteSnapshot");
		$this->registerMethod("restoreSnapshot");
		$this->registerMethod("fromSnapshot");
		$this->registerMethod("getSnapshotLifecycle");
		$this->registerMethod("setSnapshotLifecycle");
	}

	/**
	 * Get list of candidates (file systems) that can be used for shared
	 * folders.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing objects with the following fields:
	 *   \em uuid, and \em description. The field \em uuid is the UUID
	 *   from the mount point configuration object.
	 */
	public function getCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
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
		// Get the shared folder candidates.
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			// Skip file systems that are not mounted at the moment. To find
			// out, just check if the given directory is a mount point.
			$mp = new \OMV\System\MountPoint($objectv->get("dir"));
			if (FALSE === $mp->isMountPoint())
				continue;
			// Get the file system implementation for the specified mount
			// point. Skip the mount point if no file system implementation
			// exists.
			$fs = \OMV\System\Filesystem\Filesystem::getImplByMountPoint(
				$objectv->get("dir"));
			if (TRUE === is_null($fs))
				continue;
			// Skip file systems that do not implement the required interface
			// to be handled as shared folder candidate.
			if (FALSE === ($fs instanceof
				\OMV\System\Filesystem\SharedFolderCandidateInterface))
				continue;
			// Populate the description property. Use the 'comment' property
			// of the mount point database configuration object, otherwise
			// the description delivered by the file system object.
			$description = $fs->getDescription();
			if (!empty($objectv->get("comment"))) {
				$description = sprintf("%s [%s]",
					$fs->getCanonicalDeviceFile(),
					$objectv->get("comment"));
			}
			// Append the shared folder candidate.
			$result[] = [
				"uuid" => $objectv->get("uuid"),
				"description" => $description
			];
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
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get all configured shared folder configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.system.sharedfolder");
		// Add additional informations.
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			$objectDict = new \OMV\Dictionary($objectv->getAssoc());
			// Set the default values.
			$objectDict->set("_used", FALSE);
			$objectDict->set("device", "");
			$objectDict->copy("name", "description");
			$objectDict->set("mntent", [
				"devicefile" => "",
				"fsname" => "",
				"dir" => "",
				"type" => "",
				"posixacl" => FALSE
			]);
			$objectDict->set("snapshots", FALSE);
			// Get the mount point configuration object to append additional
			// information to the returned objects, e.g. the devicefile or
			// a modified description.
			if ($db->exists("conf.system.filesystem.mountpoint", [
				"operator" => "stringEquals",
				"arg0" => "uuid",
				"arg1" => $objectDict->get("mntentref")
			])) {
				$meObject = $db->get("conf.system.filesystem.mountpoint",
					$objectDict->get("mntentref"));
				$objectDict->set("device", $meObject->get("fsname"));
				// Get the file system backend to get various information
				// about the file system features, e.g. if it supports
				// POSIX ACL.
				$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
				$fsbMngr->assertBackendExistsByType($meObject->get("type"));
				$fsb = $fsbMngr->getBackendByType($meObject->get("type"));
				$objectDict->set("mntent.fsname", $meObject->get("fsname"));
				$objectDict->set("mntent.dir", $meObject->get("dir"));
				$objectDict->set("mntent.type", $meObject->get("type"));
				$objectDict->set("mntent.posixacl", $fsb->hasPosixAclSupport());
				// Try to get additional information from the file system if
				// it is mounted right at the moment.
				$fs = \OMV\System\Filesystem\Filesystem::getImplByMountPoint(
					$meObject->get("dir"));
				if ((FALSE === is_null($fs)) && $fs->exists()) {
					// Get the device file of the file system.
					$deviceFile = $fs->getDeviceFile();
					$objectDict->set("mntent.devicefile", $deviceFile);
					// Use the file system label for the 'device' property,
					// otherwise use the device file.
					$objectDict->set("device", $deviceFile);
					if (TRUE === $fs->hasLabel())
						$objectDict->set("device", $fs->getLabel());
				}
				// Modify the description field.
				if (!empty($objectDict->get("comment"))) {
					$objectDict->set("description", sprintf(
						gettext("%s [%s]"),
						$objectDict->get("name"),
						$objectDict->get("comment")));
				} else {
					$objectDict->set("description", sprintf(
						gettext("%s [on %s, %s]"),
						$objectDict->get("name"),
						$objectDict->get("device"),
						$objectDict->get("reldirpath")));
				}
				// Are snapshots supported?
				$sfAbsPath = build_path(DIRECTORY_SEPARATOR,
					$meObject->get("dir"), $objectv->get("reldirpath"));
				if (("btrfs" == $meObject->get("type")) && !is_null($fs)) {
					// Try to get the subvolume information.
					$sfInfo = $fs->getSubvolumeInfo($sfAbsPath);
					// Block the creation of snapshots if this is not a
					// subvolume or if it is a snapshot itself.
					$snapshotsSupported = !(!$sfInfo || is_uuid(
						$sfInfo['parent_uuid']));
					$objectDict->set("snapshots", $snapshotsSupported);
				}
			}
			// Is the shared folder referenced by any object? Shared
			// folder references are named 'sharedfolderref'.
			$objectDict->set("_used", $db->isReferenced($objectv));
			// Append the configuration object to the result list.
			$result[] = $objectDict->getData();
		}
		return $result;
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
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
	 *   \em mntentref The UUID of the mount point configuration object
	 *     wherein the shared folder is located.
	 *   \em mode The file mode of the shared folder directory. This field
	 *     is optional. Defaults to 775.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function set($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.sharemgmt.set");
		// The field 'reldirpath' may not contain the characters '..'. This
		// is because of security reasons: the given canonicalized absolute
		// path MUST be below the given mount point.
		if (1 == preg_match("/\.\./", $params['reldirpath'])) {
			throw new \InvalidArgumentException(sprintf(
				"The field '%s' contains forbidden two-dot symbols.",
				"reldirpath"));
		}
		// Prepare the configuration object. Use the name of the shared
		// folder as the relative directory name of the share.
		$object = new \OMV\Config\ConfigObject("conf.system.sharedfolder");
		$object->setAssoc($params, TRUE, TRUE);
		$object->set("reldirpath", build_path(DIRECTORY_SEPARATOR,
			...explode(DIRECTORY_SEPARATOR, $params['reldirpath'])).
			DIRECTORY_SEPARATOR);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		// Check uniqueness:
		// - The share name must be global unique because the name
		//   is also used when exporting a shared folder via NFS
		//   for example.
		$db->assertIsUnique($object, "name", sprintf(
			"A shared folder with the name '%s' already exists.",
			$object->get("name")));
		// - The mount directory must be unique on the same volume, too.
		$db->assertIsUniqueByFilter(
			$object,
			[
				"operator" => "and",
				"arg0" => [
					"operator" => "stringEquals",
					"arg0" => "mntentref",
					"arg1" => $object->get("mntentref")
				],
				"arg1" => [
					"operator" => "stringEquals",
					"arg0" => "reldirpath",
					"arg1" => $object->get("reldirpath")
				]
			],
			sprintf("A shared folder with the path '%s' already exists ".
				"for the specified file system.",
				$object->get("reldirpath"))
		);
		if (FALSE === $object->isNew()) {
			// Get the existing configuration object.
			$oldObject = $db->get("conf.system.sharedfolder",
				$object->getIdentifier());
			// Copy the privileges.
			if (FALSE === $oldObject->isEmpty("privileges")) {
				$object->set("privileges", $oldObject->get("privileges"));
			}
		}
		$db->set($object);
		// Append the file mode field to the notification object if set.
		// Defaults to 775.
		$object->add("mode", "string", "775");
		if (array_key_exists("mode", $params)) {
			$object->set("mode", $params['mode']);
		}
		// Get the mount point configuration object to build the absolute
		// shared folder path.
		$meObject = $db->get("conf.system.filesystem.mountpoint",
			$object->get("mntentref"));
		// Build the absolute shared folder path.
		$pathName = build_path(DIRECTORY_SEPARATOR, $meObject->get("dir"),
			$object->get("reldirpath"));
		// Create the shared folder directory if necessary.
		if (FALSE === file_exists($pathName)) {
			// Create the directory.
			switch ($meObject->get("type")) {
			case "btrfs":
				$parentDir = dirname($pathName);
				// Ensure the whole path exists, otherwise the Btrfs
				// subvolume can't be created.
				if (FALSE === is_dir($parentDir)) {
					if (FALSE === mkdir($parentDir, 0700, TRUE)) {
						throw new \OMV\Exception(
							"Failed to create the directory '%s': %s",
							$parentDir, last_error_msg());
					}
				}
				// Create the Btrfs subvolume.
				\OMV\System\Filesystem\Btrfs::createSubvolume($pathName);
				break;
			default:
				// Note, the `mkdir` function seems to have a bug when
				// using the mask parameter, e.g. `octdec("777")` does
				// not create the correct permissions as expected, thus
				// change the mode using chmod.
				if (FALSE === mkdir($pathName, 0700, TRUE)) {
					throw new \OMV\Exception(
						"Failed to create the directory '%s': %s",
						$pathName, last_error_msg());
				}
				break;
			}
			// Adapt the directory mode.
			if (FALSE === chmod($pathName, octdec($object->get("mode")))) {
				throw new \OMV\Exception(
					"Failed to set file mode to '%s' for '%s': %s",
					$object->get("mode"), $pathName, last_error_msg());
			}
		}
		// Change group owner of directory to the configured default
		// users group, e.g. 'users'.
		$defaultGroup = \OMV\Environment::get("OMV_USERMGMT_DEFAULT_GROUP",
			"users");
		if (FALSE === chgrp($pathName, $defaultGroup)) {
			throw new \OMV\Exception(
				"Failed to set file group to '%s' for '%s': %s",
				$defaultGroup, $pathName, last_error_msg());
		}
		// Set the setgid bit. Setting this permission means that all files
		// created in the folder will inherit the group of the folder rather
		// than the primary group of the user who creates the file.
		$mode = fileperms($pathName) | 02000;
		if (FALSE === chmod($pathName, $mode)) {
			throw new \OMV\Exception(
				"Failed to set file mode to '%o' for '%s': %s",
				$mode, $pathName, last_error_msg());
		}
		// Walk through the directory path to make sure group 'users' can
		// access all subdirectories.
		// Example:
		// <sf_mount_dir>/<reldirpath>
		// <sf_mount_dir>
		//  |- dir1
		//  |  |- dir1.1
		//  |  '- dir1.2
		//  |     '- dir1.2.1
		//  '
		$parts = explode(DIRECTORY_SEPARATOR, trim($object->get("reldirpath"),
			DIRECTORY_SEPARATOR));
		array_pop($parts); // Remove last directory, this is already processed.
		if (count($parts) > 0) {
			$absPath = $meObject->get("dir");
			// To prevent reaching the command line length limit (we can
			// not expect how long the absolute paths of all subdirectory
			// will be nor how many subdirectories need to be processed)
			// when processing all subdirectories with one 'setfacl' call,
			// each subdirectory will be processed separately.
			foreach ($parts as $partk => $partv) {
				$absPath = build_path(DIRECTORY_SEPARATOR, $absPath, $partv);
				$cmdArgs = [];
				$cmdArgs[] = '--modify';
				$cmdArgs[] = sprintf("group:%s:--x", $defaultGroup);
				$cmdArgs[] = escapeshellarg($absPath);
				$cmd = new \OMV\System\Process("setfacl", $cmdArgs);
				$cmd->setRedirect2to1();
				$cmd->execute();
			}
		}
		// Return the configuration object.
		return $object->getAssoc();
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
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.sharemgmt.delete");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.sharedfolder", $params['uuid']);
		// Make sure the shared folder is not referenced.
		$db->assertIsNotReferenced($object);
		// Delete shared folder content recursively?
		$recursive = boolvalEx($params['recursive']);
		// Do some more checks before a shared folder is deleted.
		if (TRUE === $recursive) {
			// Do not delete the shared folder content if another shared
			// folder points to the same directory.
			// Predicate: [uuid!='%s' and mntentref='%s' and reldirpath='%s']
			if ($db->exists("conf.system.sharedfolder", [
				"operator" => "and",
				"arg0" => [
					"operator" => "stringNotEquals",
					"arg0" => "uuid",
					"arg1" => $object->get("uuid")
				],
				"arg1" => [
					"operator" => "and",
					"arg0" => [
						"operator" => "stringEquals",
						"arg0" => "mntentref",
						"arg1" => $object->get("mntentref")
					],
					"arg1" => [
						"operator" => "stringEquals",
						"arg0" => "reldirpath",
						"arg1" => $object->get("reldirpath")
					]
				]
			])) {
				throw new \OMV\Exception("Cannot unlink the shared folder ".
				  "content because another shared folder also refers to it");
			}
			// Do not delete the shared folder content if a sub-directory
			// is part of another shared folder.
			$sfObjects = $db->getByFilter("conf.system.sharedfolder", [
				"operator" => "stringEquals",
				"arg0" => "mntentref",
				"arg1" => $object->get("mntentref")
			]);
			if (0 < count($sfObjects)) {
				// Get the mount point configuration object to build the
				// absolute shared folder path.
				$meObject = $db->get("conf.system.filesystem.mountpoint",
				  $object->get("mntentref"));
				// Build the absolute shared folder path.
				$absDirPath = \OMV\System\Os::readlink(build_path(
				  DIRECTORY_SEPARATOR, $meObject->get("dir"),
				  $object->get("reldirpath")));
				// Ensure the path ends with an slash, otherwise comparison
				// may be false positive.
				// Example:
				// To delete = /media/4ee04694-e849-4b97-b31a-a928cc084e8f/test/
				// To check  = /media/4ee04694-e849-4b97-b31a-a928cc084e8f/test_folder
				$absDirPath = sprintf("%s/", rtrim($absDirPath, "/"));
				foreach ($sfObjects as $sfObjectk => $sfObjectv) {
					// Skip current processed shared folder configuration
					// object.
					if ($sfObjectv->get("uuid") === $object->get("uuid"))
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
					  DIRECTORY_SEPARATOR, $meObject->get("dir"),
					  $sfObjectv->get("reldirpath")));
					if (0 === stripos($sfAbsDirPath, $absDirPath)) {
						throw new \OMV\Exception("Cannot unlink the shared ".
						  "folder content because another shared folder is ".
						  "referencing a subdirectory");
					}
				}
			}
		}
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.conf.system.sharedfolder",
		  $object->getAssoc(), $recursive);
		// Remove the shared folder and its content recursively?
		if (TRUE === $recursive) {
			// Get the mount point configuration object to build the absolute
			// shared folder path.
			$meObject = $db->get("conf.system.filesystem.mountpoint",
			  $object->get("mntentref"));
			// Build the absolute shared folder path.
			$dirPath = build_path(DIRECTORY_SEPARATOR, $meObject->get("dir"),
			  $object->get("reldirpath"));
			// Delete the shared folder directory.
			$cmdArgs = [];
			$cmdArgs[] = "-f";
			$cmdArgs[] = "-r";
			$cmdArgs[] = escapeshellarg($dirPath);
			$cmd = new \OMV\System\Process("rm", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute();
		}
		// Delete the configuration object.
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get the shared folder privileges.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder configuration object.
	 * @param context The context of the caller.
	 * @return An array containing the requested privileges.
	 */
	public function getPrivileges($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the shared folder configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.sharedfolder", $params['uuid']);
		// Prepare result object.
		$result = [];
		// Process non-system users.
		$users = \OMV\Rpc\Rpc::call("UserMgmt", "enumerateUsers", NULL,
		  $context);
		foreach ($users as $userk => $userv) {
			// Set default values.
			$privilege = new \OMV\Dictionary([
				"type" => "user",
				"name" => $userv['name'],
				"perms" => NULL
			]);
			// Check if there are any configured privileges for the
			// given user.
			if (FALSE === $object->isEmpty("privileges")) {
				foreach ($object->get("privileges.privilege") as $objectv) {
					if (($objectv['type'] === $privilege->get("type")) &&
					  ($objectv['name'] === $privilege->get("name"))) {
						$privilege->set("perms", intval($objectv['perms']));
						break;
					}
				}
			}
			$result[] = $privilege->getData();
		}
		// Process non-system groups.
		$groups = \OMV\Rpc\Rpc::call("UserMgmt", "enumerateGroups", NULL,
		  $context);
		foreach ($groups as $groupk => $groupv) {
			// Set default values.
			$privilege = new \OMV\Dictionary([
				"type" => "group",
				"name" => $groupv['name'],
				"perms" => NULL
			]);
			// Check if there are any configured privileges for the
			// given group.
			if (FALSE === $object->isEmpty("privileges")) {
				foreach ($object->get("privileges.privilege") as $objectv) {
					if (($objectv['type'] === $privilege->get("type")) &&
					  ($objectv['name'] === $privilege->get("name"))) {
						$privilege->set("perms", intval($objectv['perms']));
						break;
					}
				}
			}
			$result[] = $privilege->getData();
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
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.sharemgmt.setprivileges");
		// Get the shared folder configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.sharedfolder", $params['uuid']);
		// Update the shared folders privileges.
		$privilegeAssoc = [];
		foreach ($params['privileges'] as $privilegek => $privilegev) {
			// Do some checks ...
			switch ($privilegev['type']) {
			case "user":
				// Check whether the user exists.
				$user = new \OMV\System\User($privilegev['name']);
				$user->assertExists();
				// Is user allowed? It does not make sense to give the WebGUI
				// administrator permissions for a shared folder.
				if (in_array($user->getName(), [ "admin" ])) {
					throw new \OMV\Exception("The user '%s' is not allowed.",
					  $user->getName());
				}
				break;
			case "group":
				// Check whether the group exists.
				$group = new \OMV\System\Group($privilegev['name']);
				$group->assertExists();
				break;
			}
			// Finally add privilege to shared folder privileges.
			$privilegeAssoc[] = [
				"type" => $privilegev['type'],
				"name" => $privilegev['name'],
				"perms" => $privilegev['perms']
			];
		}
		$object->set("privileges", [
			"privilege" => $privilegeAssoc
		]);
		// Set the configuration object.
		$db->set($object);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.system.sharedfolder.privilege",
		  $object->getAssoc());
		// Return the configuration object.
		return $object->getAssoc();
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
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.sharemgmt.getprivilegesbyrole");
		// Get all configured shared folder configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$sfObjects = $db->get("conf.system.sharedfolder");
		// Append additional information.
		$result = [];
		foreach ($sfObjects as $sfObjectk => $sfObjectv) {
			// Set default values.
			$privDict = new \OMV\Dictionary([
				"uuid" => $sfObjectv->get("uuid"),
				"name" => $sfObjectv->get("name"),
				"perms" => NULL
			]);
			// Check if there are any configured privileges for the
			// given user/group.
			if (FALSE === $sfObjectv->isEmpty("privileges")) {
				foreach ($sfObjectv->get("privileges.privilege") as $privv) {
					if ($params['role'] !== $privv['type'])
						continue;
					if ($params['name'] !== $privv['name'])
						continue;
					$privDict->set("perms", intval($privv['perms']));
					break;
				}
			}
			$result[] = $privDict->getData();
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
	 *     \em perms The privileges: 0 (none), 5 (r) or 7 (r/w).
	 * @return An array of the stored configuration objects.
	 */
	public function setPrivilegesByRole($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.sharemgmt.setprivilegesbyrole");
		// Get all configured shared folder configuration objects and
		// iterate over them.
		$db = \OMV\Config\Database::getInstance();
		$sfObjects = $db->get("conf.system.sharedfolder");
		$result = [];
		foreach ($sfObjects as $sfObjectk => $sfObjectv) {
			// Get the privilege configuration of the current processed
			// shared folder.
			$privObjects = $this->callMethod("getPrivileges", [
				"uuid" => $sfObjectv->getIdentifier()
			], $context);
			$privilege = current(array_filter_ex($params['privileges'],
				"uuid", $sfObjectv->getIdentifier()));
			$newPrivObjects = [];
			foreach ($privObjects as $privObjectk => $privObjectv) {
				if (($privObjectv['type'] === $params['role']) &&
						($privObjectv['name'] === $params['name'])) {
				  	// Remove or update the privilege?
					if (FALSE === $privilege) {
						continue;
					}
					$privObjectv['perms'] = $privilege['perms'];
				}
				// Purge entries with invalid perms.
				if (FALSE === is_int($privObjectv['perms'])) {
					continue;
				}
				$newPrivObjects[] = $privObjectv;
			}
			// Update the shared folder privileges.
			$result[] = $this->callMethod("setPrivileges", [
				"uuid" => $sfObjectv->getIdentifier(),
				"privileges" => $newPrivObjects
			], $context);
		}
		return $result;
	}

	/**
	 * Copy the shared folder privileges.
	 * @param params An array containing the following fields:
	 *   \em src The shared folder UUID that is used as source.
	 *   \em dst The shared folder UUID that is used as destination.
	 * @return An array of the stored configuration objects.
	 */
	public function copyPrivileges($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.sharemgmt.copyprivileges");
		if ($params['src'] == $params['dst']) {
			throw new \OMV\Exception(
				"Source and destination shared folder must not be equal.");
		}
		// Copy the privileges from the source to the destination
		// shared folder.
		$privileges = $this->callMethod("getPrivileges", [
			"uuid" => $params['src']
		], $context);
		$privileges = array_values(
			array_filter($privileges, function($v) {
	   			return !is_null($v['perms']);
			})
		);
		return $this->callMethod("setPrivileges", [
			"uuid" => $params['dst'],
			"privileges" => $privileges
		], $context);
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
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.sharemgmt.getfileacl");
		// The field 'file' may not contain the characters '..'.
		if (1 == preg_match("/\.\./", $params['file'])) {
			throw new \OMV\Exception("The field 'file' contains forbidden ".
			  "two-dot symbols");
		}
		// Get the absolute shared folder path.
		$sfpath = \OMV\Rpc\Rpc::call("ShareMgmt", "getPath", [
			"uuid" => $params['uuid']
		], $context);
		// Execute command to get the file access control lists.
		$cmdArgs = [];
		$cmdArgs[] = "--no-effective";
		$cmdArgs[] = "--access";
		$cmdArgs[] = "--";
		$cmdArgs[] = escapeshellarg(build_path(DIRECTORY_SEPARATOR,
		  $sfpath, $params['file']));
		$cmd = new \OMV\System\Process("getfacl", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		// Prepare result object.
		$result = [
			"file" => NULL,
			"owner" => NULL,
			"group" => NULL,
			"acl" => [
				"user" => NULL,
				"group" => NULL,
				"other" => NULL,
				"users" => [],
				"groups" => []
			]
		];
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
		$object = [];
		// Get the owner and owning group.
		foreach ($output as $outputk => $outputv) {
			$regex = '/^# (file|owner|group): (.+)$/';
			if (1 !== preg_match($regex, $outputv, $matches))
				continue;
			// Convert special characters, e.g.
			// # file: foo
			// # owner: root
			// # group: domain\040users
			// user::rwx
			// ...
			$result[$matches[1]] = unescape_blank($matches[2], TRUE);
		}
		// Get the user, group and other file mode permission bits.
		foreach ($output as $outputk => $outputv) {
			$regex = '/^(user|group|other):(.*):(.+)$/';
			if (1 !== preg_match($regex, $outputv, $matches))
				continue;
			// Convert permissions string into a number
			$perms = 0;
			$map = [ "r" => 4, "w" => "2", "x" => 1, "-" => 0 ];
			foreach (str_split($matches[3]) as $permk => $permv) {
				if (!array_key_exists($permv, $map))
					continue;
				$perms += $map[$permv];
			}
			if (!empty($matches[2])) {
				$result['acl']["{$matches[1]}s"][] = [
					// Convert special characters, e.g.
					// group:Domain\040Computers:---
					"name" => unescape_blank($matches[2], TRUE),
					"perms" => $perms
				];
			} else {
				$result['acl'][$matches[1]] = $perms;
			}
		}
		// Add missing users.
		$users = \OMV\Rpc\Rpc::call("UserMgmt", "enumerateAllUsers",
		  NULL, $context);
		foreach ($users as $userk => $userv) {
			$found = FALSE;
			foreach ($result['acl']['users'] as &$resultv) {
				if ($resultv['name'] === $userv['name']) {
					// Append additional user details, e.g. uid and the
					// information if it is a system user.
					$resultv = array_merge($resultv, [
						"uid" => $userv['uid'],
						"system" => $userv['system']
					]);
					$found = TRUE;
					break;
				}
			}
			if (TRUE === $found)
				continue;
			$result['acl']['users'][] = [
				"name" => $userv['name'],
				"perms" => NULL,
				"uid" => $userv['uid'],
				"system" => $userv['system']
			];
		}
		// Add missing groups.
		$groups = \OMV\Rpc\Rpc::call("UserMgmt", "enumerateAllGroups",
		  NULL, $context);
		foreach ($groups as $groupk => $groupv) {
			$found = FALSE;
			foreach ($result['acl']['groups'] as &$resultv) {
				if ($resultv['name'] === $groupv['name']) {
					// Append additional group details, e.g. gid and the
					// information if it is a system group.
					$resultv = array_merge($resultv, [
						"gid" => $groupv['gid'],
						"system" => $groupv['system']
					]);
					$found = TRUE;
					break;
				}
			}
			if (TRUE === $found)
				continue;
			$result['acl']['groups'][] = [
				"name" => $groupv['name'],
				"perms" => NULL,
				"gid" => $groupv['gid'],
				"system" => $groupv['system']
			];
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
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.sharemgmt.setfileacl");
		// The field 'file' may not contain the characters '..'.
		if (1 == preg_match("/\.\./", $params['file'])) {
			throw new \OMV\Exception("The field 'file' contains forbidden ".
			  "two-dot symbols");
		}
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params, $context) {
			// Get the absolute shared folder path.
			$sfpath = \OMV\Rpc\Rpc::call("ShareMgmt", "getPath", [
				"uuid" => $params['uuid']
			], $context);
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
				$cmdArgs = [];
				if (TRUE === boolvalEx($params['recursive']))
					$cmdArgs[] = "--recursive";
				$cmdArgs[] = $ownerGroupArg;
				$cmdArgs[] = escapeshellarg(build_path(DIRECTORY_SEPARATOR,
				  $sfpath, $params['file']));
				// Execute command to set the file owner/group.
				$cmd = new \OMV\System\Process("chown", $cmdArgs);
				$cmd->setRedirect2to1();
				if (0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
					throw new \OMV\ExecException($cmd, $output);
				}
			}
			////////////////////////////////////////////////////////////////////
			// 2. Update the directory/file access control lists
			////////////////////////////////////////////////////////////////////
			// Build the ACL specifications.
			$aclSpec = [];
			$aclSpecFile = new \OMV\System\TmpFile();
			// Set permissions of a named users and groups.
			foreach ([ "user", "group" ] as $typek => $typev) {
				foreach ($params["{$typev}s"] as $entryk => $entryv) {
					$aclSpec[] = sprintf("default:%s:%s:%d", $typev,
						$entryv['name'], $entryv['perms']);
					$aclSpec[] = sprintf("%s:%s:%d", $typev,
						$entryv['name'], $entryv['perms']);
				}
			}
			// Set default owner permissions.
			if (array_key_exists("userperms", $params)) {
				$aclSpec[] = sprintf("default:user::%d", $params['userperms']);
				$aclSpec[] = sprintf("user::%d", $params['userperms']);
			}
			// Set default owning group permissions.
			if (array_key_exists("groupperms", $params)) {
				$aclSpec[] = sprintf("default:group::%d", $params['groupperms']);
				$aclSpec[] = sprintf("group::%d", $params['groupperms']);
			}
			// Set default permissions of others.
			if (array_key_exists("otherperms", $params)) {
				$aclSpec[] = sprintf("default:other::%d", $params['otherperms']);
				$aclSpec[] = sprintf("other::%d", $params['otherperms']);
			}
			// Build the command arguments list.
			$cmdArgs = [];
			if (TRUE === boolvalEx($params['replace']))
				$cmdArgs[] = "--remove-all";
			if (TRUE === boolvalEx($params['recursive']))
				$cmdArgs[] = "--recursive";
			if (!empty($aclSpec)) {
				if (FALSE === $aclSpecFile->write(implode("\n", $aclSpec))) {
					throw new \OMV\Exception(
						"Failed to write ACL entries to file.");
				}
				// Read the ACL entries from file. The CLI argument list
				// is limited which will cause an error if the user/group
				// list is too large.
				$cmdArgs[] = sprintf("-M %s", escapeshellarg(
					$aclSpecFile->getFileName()));
			}
			$cmdArgs[] = "--";
			$cmdArgs[] = escapeshellarg(build_path(DIRECTORY_SEPARATOR,
				$sfpath, $params['file']));
			// Execute command to set the file access control lists.
			$cmd = new \OMV\System\Process("setfacl", $cmdArgs);
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
			return $output;
		});
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the shared folder configuration object.
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
	 * Enumerate the snapshots of the specified shared folder.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder configuration object.
	 * @param context The context of the caller.
	 * @return An array containing the snapshot information.
	 */
	public function enumerateSnapshots($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the shared folder and mount point configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$sfObject = $db->get("conf.system.sharedfolder", $params['uuid']);
		$meObject = $db->get("conf.system.filesystem.mountpoint",
			$sfObject->get("mntentref"));
		// Get the list of snapshots of this shared folder.
		if ("btrfs" !== $meObject->get("type")) {
			return [];
		}
		$fs = \OMV\System\Filesystem\Filesystem::getImplByMountPoint(
			$meObject->get("dir"));
		$sfAbsPath = build_path(DIRECTORY_SEPARATOR, $meObject->get("dir"),
			$sfObject->get("reldirpath"));
		if (FALSE === $fs->isSubvolume($sfAbsPath)) {
			return [];
		}
		$result = [];
		$svInfo = $fs->getSubvolumeInfo($sfAbsPath);
		// The snapshots of the subvolumes are stored in the root pool of
		// the mounted file system.
		$allSnapshots = $fs->listSnapshots($meObject->get("dir"));
		$result = array_filter_ex($allSnapshots, "parent_uuid",
			$svInfo['uuid']);
		// Check if the snapshot is referenced by a shared folder.
		$db = \OMV\Config\Database::getInstance();
		foreach ($result as $snapshotk => &$snapshotv) {
			$snapshotv['mntentref'] = $meObject->getIdentifier();
			$snapshotv['sharedfolderref'] = $sfObject->getIdentifier();
			$snapshotv['otimets'] = strpdate($snapshotv['otime'],
				"Y-m-d H:i:s");
			$snapshotv['abspath'] = build_path(DIRECTORY_SEPARATOR,
				$meObject->get("dir"), $snapshotv['path']);
			$snapshotv['_used'] = $db->exists(
				"conf.system.sharedfolder", [
					"operator" => "stringEquals",
					"arg0" => "reldirpath",
					"arg1" => build_path(DIRECTORY_SEPARATOR,
						$snapshotv['path'], DIRECTORY_SEPARATOR)
				]);
		}
		return $result;
	}

	/**
	 * Enumerate the snapshots of all existing shared folders.
	 * @param params Not used.
	 * @param context The context of the caller.
	 * @return An array containing the information of all snapshots.
	 */
	public function enumerateAllSnapshots($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.system.sharedfolder");
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			$result = array_merge($result, $this->callMethod(
				"enumerateSnapshots", [
					"uuid" => $objectv->getIdentifier()
				], $context));
		}
		return $result;
	}

	/**
	 * Create a snapshot of the specified shared folder.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder configuration object.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function createSnapshot($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the shared folder and mount point configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$sfObject = $db->get("conf.system.sharedfolder", $params['uuid']);
		$meObject = $db->get("conf.system.filesystem.mountpoint",
			$sfObject->get("mntentref"));
		// Does the shared folder support snapshots?
		if ("btrfs" !== $meObject->get("type")) {
			throw new \OMV\Exception(
				"The shared folder '%s' does not support snapshots.",
				$sfObject->get("name"));
		}
		$fs = \OMV\System\Filesystem\Filesystem::getImplByMountPoint(
			$meObject->get("dir"));
		$sfAbsPath = build_path(DIRECTORY_SEPARATOR, $meObject->get("dir"),
			$sfObject->get("reldirpath"));
		if (FALSE === $fs->isSubvolume($sfAbsPath)) {
			throw new \OMV\Exception(
				"The shared folder '%s' does not support snapshots.",
				$sfObject->get("name"));
		}
		// Build the target of the snapshot. This includes the name of
		// the shared folder + the current time in ISO 8601 basic format
		// (without time zone designator).
		// Make sure the `.snapshots` directory/subvolume exists.
		$snapshotName = sprintf("%s_%s", $sfObject->get("name"),
			date('Ymd\THis'));
		$snapshotsDir = build_path(DIRECTORY_SEPARATOR,
			$meObject->get("dir"), ".snapshots");
		$snapshotPath = build_path(DIRECTORY_SEPARATOR, $snapshotsDir,
			$snapshotName);
		if (FALSE === is_dir($snapshotsDir)) {
			$fs->createSubvolume($snapshotsDir);
		}
		// Make sure that the `.snapshots` directory is accessible by
		// the configured default users group, e.g. 'users'.
		$defaultGroup = \OMV\Environment::get(
			"OMV_USERMGMT_DEFAULT_GROUP", "users");
		if (FALSE === chgrp($snapshotsDir, $defaultGroup)) {
			throw new \OMV\Exception(
				"Failed to set file group to '%s' for '%s': %s",
				$defaultGroup, $snapshotsDir, last_error_msg());
		}
		$fs->createSnapshot($sfAbsPath, $snapshotPath);
		return [
			"sharedfolder" => $sfObject->get("name"),
			"sharedfolderref" => $sfObject->getIdentifier(),
			"name" => $snapshotName,
			"path" => build_path(DIRECTORY_SEPARATOR, ".snapshots",
				$snapshotName),
			"abspath" => $snapshotPath
		];
	}

	/**
	 * Create a scheduled task that will do a snapshot of the specified
	 * shared folder.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder configuration object.
	 *   \em execution The time when the snapshot should be created
	 *     periodically, e.g. `hourly`, `daily`, `weekly`, `monthly`
	 *     or `yearly`.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function createScheduledSnapshotTask($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.sharemgmt.createscheduledsnapshot");
		// Get the shared folder and mount point configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$sfObject = $db->get("conf.system.sharedfolder", $params['uuid']);
		$meObject = $db->get("conf.system.filesystem.mountpoint",
			$sfObject->get("mntentref"));
		// Does the shared folder support snapshots?
		if ("btrfs" !== $meObject->get("type")) {
			throw new \OMV\Exception(
				"The shared folder '%s' does not support snapshots.",
				$sfObject->get("name"));
		}
		$fs = \OMV\System\Filesystem\Filesystem::getImplByMountPoint(
			$meObject->get("dir"));
		$sfAbsPath = build_path(DIRECTORY_SEPARATOR, $meObject->get("dir"),
			$sfObject->get("reldirpath"));
		if (FALSE === $fs->isSubvolume($sfAbsPath)) {
			throw new \OMV\Exception(
				"The shared folder '%s' does not support snapshots.",
				$sfObject->get("name"));
		}
		$snapshotsDir = build_path(DIRECTORY_SEPARATOR,
			$meObject->get("dir"), ".snapshots");
		$snapshotPath = build_path(DIRECTORY_SEPARATOR,
			$snapshotsDir,
			sprintf("%s@%s_$(date +%%Y%%m%%dT%%H%%M%%S)",
				$sfObject->get("name"), $params['execution']));
		// Make sure that the `.snapshots` directory exists.
		if (FALSE === is_dir($snapshotsDir)) {
			$fs->createSubvolume($snapshotsDir);
		}
		// Create the scheduled job.
		$object = new \OMV\Config\ConfigObject("conf.system.cron.job");
		$object->setNew();
		$object->setAssoc([
			"type" => "userdefined",
			"enable" => TRUE,
			"execution" => $params['execution'],
			"command" => sprintf("btrfs subvolume snapshot %s %s",
				$sfAbsPath, $snapshotPath),
			"username" => "root",
			"sendemail" => TRUE,
			"comment" => sprintf("Snapshot,%s,%s",
				ucfirst($params['execution']), $sfObject->get("name"))
		]);
		$object->validate();
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Enumerate the cron jobs taking the snapshots of the specified
	 * shared folder.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder configuration object.
	 * @param context The context of the caller.
	 * @return An array containing the cron job configurations of the
	 *   specified shared folder. Check the `Cron::getList` RPC for
	 *   more information.
	 */
	public function enumerateScheduledSnapshotTasks($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the shared folder and mount point configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$sfObject = $db->get("conf.system.sharedfolder", $params['uuid']);
		$meObject = $db->get("conf.system.filesystem.mountpoint",
			$sfObject->get("mntentref"));
		// Build the path to search for.
		$sfAbsPath = build_path(DIRECTORY_SEPARATOR, $meObject->get("dir"),
			$sfObject->get("reldirpath"));
		// Find the scheduled task configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.system.cron.job", [
			"operator" => "stringContains",
			"arg0" => "command",
			"arg1" => $sfAbsPath
		]);
		// Convert the response into an associative array.
		return array_map(function($object) {
			return $object->getAssoc();
		}, $objects);
	}

	/**
	 * Enumerate the cron jobs taking the snapshots of all existing
	 * shared folders.
	 * @param params Not used.
	 * @param context The context of the caller.
	 * @return An array containing the cron job configurations of all
	 *   existing shared folder. Check the `Cron::getList` RPC for
	 *   more information.
	 */
	public function enumerateAllScheduledSnapshotTasks($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.system.sharedfolder");
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			$result = array_merge($result, $this->callMethod(
				"enumerateScheduledSnapshotTasks", [
					"uuid" => $objectv->getIdentifier()
				], $context));
		}
		return $result;
	}

	/**
	 * Delete a snapshots of a shared folder.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder configuration object.
	 *   \em id The ID of the subvolume to delete, e.g. 397.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function deleteSnapshot($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.sharemgmt.deletesnapshot");
		// Get the shared folder and mount point configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$sfObject = $db->get("conf.system.sharedfolder",
			$params['uuid']);
		$meObject = $db->get("conf.system.filesystem.mountpoint",
			$sfObject->get("mntentref"));
		$fs = \OMV\System\Filesystem\Filesystem::getImplByMountPoint(
			$meObject->get("dir"));
		$sfAbsPath = build_path(DIRECTORY_SEPARATOR, $meObject->get("dir"),
			$sfObject->get("reldirpath"));
		if (FALSE === $fs->isSubvolume($sfAbsPath)) {
			throw new \OMV\Exception(
				"The shared folder '%s' does not support snapshots.",
				$sfObject->get("name"));
		}
		$fs->deleteSnapshot($sfAbsPath, $params['id']);
	}

	/**
	 * Restore a snapshots of a shared folder.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder configuration object.
	 *   \em id The ID of the subvolume to restore.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function restoreSnapshot($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.sharemgmt.restoresnapshot");
		// Get the shared folder and mount point configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$sfObject = $db->get("conf.system.sharedfolder",
			$params['uuid']);
		$meObject = $db->get("conf.system.filesystem.mountpoint",
			$sfObject->get("mntentref"));
		$fs = \OMV\System\Filesystem\Filesystem::getImplByMountPoint(
			$meObject->get("dir"));
		$sfAbsPath = build_path(DIRECTORY_SEPARATOR, $meObject->get("dir"),
			$sfObject->get("reldirpath"));
		if (FALSE === $fs->isSubvolume($sfAbsPath)) {
			throw new \OMV\Exception(
				"The shared folder '%s' does not support snapshots.",
				$sfObject->get("name"));
		}
		// Get the list of snapshots.
		$snapshots = array_filter_ex($fs->listSnapshots($sfAbsPath),
			"id", $params['id']);
		if (is_null($snapshots) || empty($snapshots)) {
			throw new \OMV\Exception(
				"The snapshot '%s' from '%s' does not exist.",
				$params['id'], $sfObject->get("name"));
		}
		// Delete current subvolume.
		$fs->deleteSubvolume($sfAbsPath);
		// Restore the snapshot.
		$sourcePath = build_path(DIRECTORY_SEPARATOR, $meObject->get("dir"),
			$snapshots[0]['path']);
		$fs->createSnapshot($sourcePath, $sfAbsPath);
	}

	/**
	 * Create a shared folder from a snapshots.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the shared folder configuration object.
	 *   \em id The ID of the subvolume.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw \OMV\Exception
	 */
	public function fromSnapshot($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.sharemgmt.fromsnapshot");
		// Get the shared folder and mount point configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$sfObject = $db->get("conf.system.sharedfolder",
			$params['uuid']);
		$meObject = $db->get("conf.system.filesystem.mountpoint",
			$sfObject->get("mntentref"));
		$fs = \OMV\System\Filesystem\Filesystem::getImplByMountPoint(
			$meObject->get("dir"));
		$sfAbsPath = build_path(DIRECTORY_SEPARATOR, $meObject->get("dir"),
			$sfObject->get("reldirpath"));
		// Get the list of snapshots.
		$snapshots = array_filter_ex($fs->listSnapshots($sfAbsPath),
			"id", $params['id']);
		if (is_null($snapshots) || empty($snapshots)) {
			throw new \OMV\Exception(
				"The snapshot '%s' from '%s' does not exist.",
				$params['id'], $sfObject->get("name"));
		}
		// Get the `mode` settings from the origin shared folder.
		if (FALSE === ($stat = stat($sfAbsPath))) {
			throw new \OMV\Exception(
				"Failed to get file stats from '%s'.", $sfAbsPath);
		}
		// Create a new shared folder object.
		return $this->callMethod("set", [
			"uuid" => \OMV\Environment::get("OMV_CONFIGOBJECT_NEW_UUID"),
			"name" => $snapshots[0]['name'],
			"reldirpath" => $snapshots[0]['path'],
			"mntentref" => $meObject->get("uuid"),
			"mode" => decoct($stat['mode'] & 000777),
			"privileges" => $sfObject->isEmpty("privileges") ?
				[] : $sfObject->get("privileges"),
			"comment" => sprintf("Snapshot,%s,%s", $sfObject->get("name"),
				$snapshots[0]['otime'])
		], $context);
	}

	/**
	 * Get the snapshot lifecycle configuration.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 */
	public function getSnapshotLifecycle($params, $context) {
		return \OMV\Rpc\Rpc::call("Config", "get", [
			"id" => "conf.system.sharedfolder.snapshot.lifecycle"
		], $context);
	}

	/**
	 * Set the snapshot lifecycle configuration.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setSnapshotLifecycle($params, $context) {
		return \OMV\Rpc\Rpc::call("Config", "set", [
			"id" => "conf.system.sharedfolder.snapshot.lifecycle",
			"data" => $params
		], $context);
	}
}
