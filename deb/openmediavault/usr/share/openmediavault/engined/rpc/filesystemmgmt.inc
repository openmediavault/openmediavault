<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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

class OMVRpcServiceFileSystemMgmt extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "FileSystemMgmt";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("enumerateFilesystems");
		$this->registerMethod("enumerateMountedFilesystems");
		$this->registerMethod("getList");
		$this->registerMethod("getCandidates");
		$this->registerMethod("create");
		$this->registerMethod("resize");
		$this->registerMethod("delete");
		$this->registerMethod("mount");
		$this->registerMethod("umount");
		$this->registerMethod("hasFilesystem");
	}

	/**
	 * Enumerate all filesystems that have been detected, except the
	 * filesystem containing the operation system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects with the following fields: \em uuid,
	 *   \em devicefile, \em type, \em label, \em blocks, \em size,
	 *   \em mountpoint, \em blocks, \em used, \em available, \em description,
	 *   \em posixacl, \em mounted and \em percentage.
	 * @throw E_MISC_FAILURE
	 */
	public function enumerateFilesystems($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get list of all detected filesystems. Note, the list may also
		// contain RAID and LVM volumes, so the list must be filtered.
		if(FALSE === ($objects = OMVFilesystems::get())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get filesystems"));
		}
		// Get the root partition (the filesystem containing the operation
		// system) to filter it out.
		$rootDeviceFile = OMVSystem::getRootDeviceFile();
		// Process the detected filesystems and skip unwanted ones.
		$result = array();
		foreach($objects as $objectk => $objectv) {
			// Is it a filesystem of interest?
			if(!in_array($objectv['type'], array("ext", "ext2", "ext3",
			  "ext4", "xfs", "jfs", "iso9660", "msdos", "ntfs", "udf",
			  "umsdos", "vfat", "ufs", "reiserfs", "btrfs", "hfsplus",
			  "zfs")))
				continue;
			// Skip the device where the operating system is installed on.
			if($objectv['devicefile'] === $rootDeviceFile)
				continue;
			// Use the devicefile to get the filesystem details because vfat
			// filesystems do not have a valid UUID. Any idea why?
			$fs = new OMVFilesystem($objectv['devicefile']);
			if(!$fs->exists()) {
//				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
//				  sprintf(gettext("Filesystem '%s' not found"),
//				  $objectv['devicefile']));
				continue;
			}
			// Set default values.
			$object = array(
				"devicefile" => $fs->getDeviceFile(),
				"uuid" => $fs->getUuid(),
				"label" => $fs->getLabel(),
				"type" => $fs->getType(),
				"blocks" => "-1", // as string
				"mounted" => FALSE,
				"mountpoint" => "",
				"used" => "-1", // as string
				"available" => "-1", // as string
				"size" => "-1", // as string
				"percentage" => -1,
				"description" => "",
				"posixacl" => $fs->hasPosixAclSupport(),
				"_used" => FALSE
			);
			// Check if the filesystem is used. First try to get the
			// corresponding mount point configuration object. If such object
			// exists, then check if it is referenced by any other object,
			// e.g. by a shared folder configuration object.
			if(FALSE !== ($meObject = OMVRpc::exec("FsTab", "getByFsName",
			  array("id" => $fs->getDeviceFile()), $context))) {
				if(TRUE === $xmlConfig->exists("//mntentref[".
				  "contains(.,'{$meObject['uuid']}')]")) {
					$object['_used'] = TRUE;
				}
			}
			// If the filesystem is mounted then try to get more
			// informations about it. Note,it is not possible to get
			// details from unmounted filesystems, because on most kinds
			// of systems doing so requires very nonportable intimate
			// knowledge of filesystem structures. See man (1) df.
			if(TRUE === $fs->isMounted()) {
				$object['mounted'] = TRUE;
				// Get some more filesystem details if possible.
				if(FALSE !== ($fsStats = $fs->getStatistics())) {
					$object['used'] = binary_format($fsStats['used']);
					$object['available'] = $fsStats['available'];
					$object['percentage'] = $fsStats['percentage'];
					$object['blocks'] = $fsStats['blocks'];
					$object['mountpoint'] = $fsStats['mountpoint'];
					$object['size'] = $fsStats['size'];
					$object['description'] = sprintf(
					  gettext("%s (%s available)"),
					  !empty($object['label']) ? $object['label'] :
					  $object['devicefile'], binary_format(
					  $object['available']));
				}
			}
			$result[] = $object;
		}
		return $result;
	}

	/**
	 * Enumerate all filesystems that have a mount point configuration
	 * object, except binds, and are actually mounted.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects with the following fields: \em uuid,
	 *   \em devicefile, \em type, \em label, \em blocks, \em size,
	 *   \em mountpoint, \em blocks, \em used, \em available,
	 *   \em description and \em percentage.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function enumerateMountedFilesystems($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get list of mount points, except bind mounts.
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
			// Get the filesystem details.
			$object = array(
				"devicefile" => $fs->getDeviceFile(),
				"uuid" => $fs->getUuid(),
				"label" => $fs->getLabel(),
				"type" => $fs->getType(),
				"blocks" => "-1", // as string
				"mountpoint" => "",
				"used" => "-1", // as string
				"available" => "-1", // as string
				"size" => "-1", // as string
				"percentage" => -1,
				"description" => "",
				"posixacl" => $fs->hasPosixAclSupport()
			);
			// Get some more filesystem details if possible.
			if(FALSE !== ($fsStats = $fs->getStatistics())) {
				$object['used'] = binary_format($fsStats['used']);
				$object['available'] = $fsStats['available'];
				$object['percentage'] = $fsStats['percentage'];
				$object['blocks'] = $fsStats['blocks'];
				$object['mountpoint'] = $fsStats['mountpoint'];
				$object['size'] = $fsStats['size'];
				$object['description'] = sprintf(
				  gettext("%s (%s available)"), !empty($object['label']) ?
				  $object['label'] : $object['devicefile'], binary_format(
				  $object['available']));
			}
			$result[] = $object;
		}
		return $result;
	}

	/**
	 * Get the list of filesystems that have been detected, except the
	 * filesystem containing the operation system.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param context The context of the caller.
	 * @return An array of objects with the following fields: \em uuid,
	 *   \em devicefile, \em type, \em label, \em blocks, \em size,
	 *   \em mountpoint, \em blocks, \em used, \em available,
	 *   \em description, \em mounted, \em percentage and \em status.
	 *   The field 'status' has the following meaning:
	 *   1 - Online
	 *   2 - Initializing in progress
	 *   3 - Missing
	 * @throw E_CONFIG_GET_OBJECT_FAILED
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
		// Enumerate all detected filesystems. Mark each as initialized.
		$objects = $this->callMethod("enumerateFilesystems", NULL, $context);
		foreach($objects as $objectk => &$objectv) {
			$objectv['status'] = 1;
		}
		// Try to detect filesystems that are being initialized.
		foreach(new DirectoryIterator("/tmp") as $file) {
			if($file->isDot())
				continue;
			if($file->isFile()) {
				// Check if it is a file we are interested in. The filename
				// must look like omv-initfs@<device>.build, e.g.
				// omv-initfs@_dev_sdb.build
				$regex = '/^omv-initfs@.+\.build$/i';
				if(1 !== preg_match($regex, $file->getFilename()))
					continue;
				$fileName = sprintf("/tmp/%s", $file->getFilename());
				// Read the file content and decode JSON data into an
				// associative array.
				$fsInfo = json_decode(file_get_contents($fileName), TRUE);
				// Check whether the filesystem initialization process has
				// been finished already. If yes, then unlink the file. The
				// filesystem has then been already detected by blkid, thus
				// it is already in the list of detected filesystems.
				$initialized = FALSE;
				foreach($objects as $objectk => &$objectv) {
					if($objectv['devicefile'] === $fsInfo['devicefile']) {
						$initialized = TRUE;
						break;
					}
				}
				if(TRUE === $initialized) {
					if(TRUE === file_exists($fileName))
						unlink($fileName);
					continue;
				}
				// Add the filesystem to the result list.
				$objects[] = array(
					"devicefile" => $fsInfo['devicefile'],
					"uuid" => "", // Not available
					"label" => $fsInfo['label'],
					"type" => $fsInfo['type'],
					"blocks" => "-1", // as string
					"mounted" => FALSE,
					"mountpoint" => "",
					"used" => "-1", // as string
					"available" => "-1", // as string
					"size" => "-1", // as string
					"percentage" => -1,
					"description" => "",
					"status" => 2,
					"_used" => FALSE
				);
			}
		}
		// Add filesystems configured to be mounted but device does not
		// exist anymore. This is necessary to be able remove invalid mount
		// point configuration objects. Mark such filesystem as missing.
		$xpath = "//system/fstab/mntent[not(contains(opts,'bind') or ".
		  "contains(opts,'loop'))]";
		$mntents = $xmlConfig->getList($xpath);
		if(is_null($mntents)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		foreach($mntents as $mntentk => $mntentv) {
			// Skip valid mount point configuration objects (the filesystem
			// exists in this case).
			$fs = new OMVFilesystem($mntentv['fsname']);
			if($fs->exists())
				continue;
			// Check if the mount point configuration object is referenced by
			// any other object, e.g. by a shared folder configuration object.
			$xpath = sprintf("//mntentref[contains(.,'%s')]", $mntentv['uuid']);
			// Append as much informations as possible.
			$objects[] = array(
				"devicefile" => is_devicefile($mntentv['fsname']) ?
				  $mntentv['fsname'] : "",
				"uuid" => is_uuid($mntentv['fsname']) ?
				  $mntentv['fsname'] : "",
				"label" => gettext("n/a"),
				"type" => $mntentv['type'],
				"blocks" => "-1", // as string
				"mounted" => TRUE,
				"mountpoint" => $mntentv['dir'],
				"used" => "-1", // as string
				"available" => "-1", // as string
				"size" => "-1", // as string
				"percentage" => -1,
				"description" => "",
				"status" => 3,
				"_used" => $xmlConfig->exists($xpath)
			);
		}
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get list of devices that can be used to create a filesystem on.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing objects with the following fields:
	 *   devicefile, size and description.
	 * @throw E_MISC_FAILURE
	 */
	public function getCandidates($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get a list of all potential usable devices.
		if(FALSE === ($devs = OMVStorageDevices::enumerateUnused())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  "Failed to get list of unused devices");
		}
		// Prepare the result list.
		$result = array();
		foreach($devs as $devk => $devv) {
			// Get the object that implements the given storage device.
			$sd = OMVStorageDeviceFactory::get($devv);
			if(is_null($sd) || !$sd->exists())
				continue;
			// Check if device is referenced/used by a plugin.
			$xpath = sprintf("//services/devicefile[contains(.,'%s')]",
			  $sd->getDeviceFile());
			if(TRUE === $xmlConfig->exists($xpath))
				continue;
			// Does this device already contain a filesystem?
			if(FALSE !== OMVFilesystem::hasFileSystem($sd->getDeviceFile()))
				continue;
			// The device is a potential candidate to create a filesystem
			// on it.
			$result[] = array(
				"devicefile" => $sd->getDeviceFile(),
				"size" => $sd->getSize(),
				"description" => $sd->getDescription()
			);
		}
		return $result;
	}

	/**
	 * Create a filesystem on the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The block special device file.
	 *   \em type The filesystem to create, e.g. ext3 or xfs.
	 *   \em label The label of the filesystem.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_CONFIG_OBJECT_UNIQUENESS
	 * @throw E_EXEC_FAILED
	 * @throw E_MISC_FAILURE
	 */
	public function create($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "devicefile":{"type":"string"},
				  "type":{"type":"string","enum":["ext3","ext4",'.
					'"jfs","xfs"]},
				  "label":{"type":"string"}
			  }
		  }');
		// Check uniqueness. If there exists a mount point for the given
		// device then it has already a filesystem that is in usage.
		if(FALSE !== OMVRpc::exec("FsTab", "getByFsName", array(
		  "id" => $params['devicefile']), $context)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
			  sprintf(gettext("There already exists a mount point for %s"),
			  $params['devicefile']));
		}
		// Create the filesystem.
		$cmdArgs = array();
		$cmdArgs[] = "-b";
		$cmdArgs[] = "-t";
		$cmdArgs[] = $params['type'];
		if(!empty($params['label'])) {
			$cmdArgs[] = "-L";
			$cmdArgs[] = escapeshellarg($params['label']);
		}
		$cmd = sprintf("omv-initfs %s %s 2>&1", implode(" ", $cmdArgs),
		  $params['devicefile']);
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.filesystem", array(
			  "devicefile" => $params['devicefile'],
			  "type" => $params['type'],
			  "label" => $params['label']
		  ));
		// Stupid workaround to ensure that the filesystem creation was
		// successful (there is no other situation where the WebGUI can be
		// notified that the creation has been failed):
		// - Wait some seconds
		// - Check if a error log exists
		// - Throw an exception if necessary to notify display an error
		//   message in the WebGUI
		sleep(2);
		$fileName = sprintf("omv-initfs@%s", strtr($params['devicefile'],
		  "/", "_"));
		$filePath = sprintf("/tmp/%s.fail", $fileName);
		if(@file_exists($filePath)) {
			$filePath = sprintf("%s/%s.log", $GLOBALS['OMV_LOG_DIR'],
			  $fileName);
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  gettext("Failed to create filesystem. Please check the syslog or '%s' for more details."),
			  $filePath));
		}
	}

	/**
	 * Resize a filesystem.
	 * @param params An array containing the following fields:
	 *   \em id The UUID or block special device of the filesystem to resize.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 * @throw E_EXEC_FAILED
	 */
	public function resize($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "id":{'.$GLOBALS['OMV_JSONSCHEMA_FILESYSTEM_UUID'].'}
			  }
		  }');
		// Get filesystem informations.
		$fs = new OMVFilesystem($params['id']);
		if(!$fs->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("Filesystem '%s' not found"), $params['id']));
		}
		// Grow the filesystem
		switch(strtolower($fs->getType())) {
		case "ext":
		case "ext2":
		case "ext3":
		case "ext4":
			$cmd = sprintf("resize2fs %s", $fs->getDeviceFile());
			break;
		case "xfs":
			$cmd = sprintf("xfs_growfs %s",
			  OMVFilesystem::buildMountPath($fs->getUuid()));
			break;
		case "jfs":
			$cmd = sprintf("mount -o remount,resize %s",
			  OMVFilesystem::buildMountPath($fs->getUuid()));
			break;
		default:
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("Unknown filesystem '%s'"), $fs->getType()));
			break;
		}
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Delete a filesystem. The filesystem will be unmounted and deleted.
	 * @param params An array containing the following fields:
	 *   \em id The UUID or block special device of the filesystem to delete.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 * @throw E_EXEC_FAILED
	 */
	public function delete($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "id":{'.$GLOBALS['OMV_JSONSCHEMA_FILESYSTEM_ID'].'}
			  }
		  }');
		// Get filesystem informations.
		$fs = new OMVFilesystem($params['id']);
		if(!$fs->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("Filesystem '%s' not found"), $params['id']));
		}
		$fsObject = array(
			"devicefile" => $fs->getDeviceFile(),
			"uuid" => $fs->getUuid(),
			"label" => $fs->getLabel(),
			"type" => $fs->getType()
		);
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.system.storage.filesystem", $fsObject);
		// Delete the associated fstab mount point entry if necessary.
		if(is_devicefile($params['id'])) {
			// Use the filesystem name to identify the object.
			$xpath = sprintf("//system/fstab/mntent[fsname='%s']",
			  $params['id']);
		} else {
			// Use the filesystem UUID to identify the object by its mount
			// point directory:
			// /media/<fsuuid>
			// /media/D81A-F053 (VFAT)
			// /media/74A225C8A2258F (NTFS)
			// /media/9e8aaee0-ab3a-467f-9bdb-7cced3b8afc4 (EXT2/3/4, JFS, XFS)
			$xpath = sprintf("//system/fstab/mntent[dir='%s']",
			  OMVFilesystem::buildMountPath($params['id']));
		}
		$meObject = $xmlConfig->get($xpath);
		if(!is_null($meObject)) {
			// Delete the mount point configuration object. Unmount the
			// filesystem and unlink the mount point. Changes to the fstab
			// module must not be applied immediately.
			OMVRpc::exec("FsTab", "delete", array(
			  "uuid" => $meObject['uuid']), $context);
			OMVRpc::exec("Config", "applyChanges", array(
				  "modules" => array("fstab"),
				  "force" => TRUE
			  ), $context);
		}
		// Finally erase the filesystem. Whether the partition type is 'dos'
		// then it is necessary to erase the MBR before, otherwise 'wipefs'
		// fails, e.g.:
		// wipefs: error: /dev/sdh1: appears to contain 'dos' partition table
		if(in_array($fs->getPartitionType(), array("dos", "vfat"))) {
			// http://en.wikipedia.org/wiki/Master_boot_record
			$cmd = sprintf("export LANG=C; dd if=/dev/zero of=%s bs=512 ".
			  "count=1 2>&1", $fs->getDeviceFile());
			if(0 !== $this->exec($cmd, $output)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
		}
		$cmd = sprintf("export LANG=C; wipefs -a %s 2>&1",
		  $fs->getDeviceFile());
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Notify configuration changes.
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.filesystem", $fsObject);
	}

	/**
	 * Mount a filesystem.
	 * @param params An array containing the following fields:
	 *   \em id The UUID or block special device of the filesystem to mount.
	 *   \em fstab If set to FALSE, no fstab entry will be created, thus the
	 *     given filesystem is mounted only.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 * @throw E_CONFIG_SAVE_FAILED
	 */
	public function mount($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "id":{'.$GLOBALS['OMV_JSONSCHEMA_FILESYSTEM_UUID'].'},
				  "fstab":{"type":"boolean"}
			  }
		  }');
		// Get the filesystem.
		$fs = new OMVFilesystem($params['id']);
		if(!$fs->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("Filesystem '%s' not found"),
			  $params['id']));
		}
		// Get the storage device containing the filesystem.
		if(FALSE === ($storageDeviceFile = $fs->getStorageDeviceFile())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  gettext("Failed to get storage device file from '%s'"),
			  $params['id']));
		}
		$sd = OMVStorageDeviceFactory::get($storageDeviceFile);
		if(is_null($sd) || !$sd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $storageDeviceFile);
		}
		// Get mount options depending on the filesystem type.
		switch($fs->getType()) {
		case "ext3":
			$fsName = $fs->getUuid();
			$opts = $GLOBALS['OMV_FSTAB_MNTOPS_EXT3'];
			break;
		case "ext4":
			$fsName = $fs->getUuid();
			$opts = $GLOBALS['OMV_FSTAB_MNTOPS_EXT4'];
			break;
		case "jfs":
			$fsName = $fs->getUuid();
			$opts = $GLOBALS['OMV_FSTAB_MNTOPS_JFS'];
			break;
		case "xfs":
			$fsName = $fs->getUuid();
			$opts = $GLOBALS['OMV_FSTAB_MNTOPS_XFS'];
			break;
		case "vfat":
			$fsName = $fs->getDeviceFileByUuid();
			$opts = $GLOBALS['OMV_FSTAB_MNTOPS_VFAT'];
			break;
		case "ntfs":
			$fsName = $fs->getDeviceFileByUuid();
			$opts = $GLOBALS['OMV_FSTAB_MNTOPS_NTFS'];
			break;
		case "hfsplus":
			$fsName = $fs->getUuid();
			$opts = $GLOBALS['OMV_FSTAB_MNTOPS_HFSPLUS'];
			break;
		default:
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  gettext("Filesystem '%s' not supported"), $type));
			break;
		}
		// Create fstab entry?
		if(TRUE === boolval($params['fstab'])) {
			// Check for duplicates. Create a new mount point configuration
			// object if necessary.
			if(FALSE === OMVRpc::exec("FsTab", "getByFsName",
			  array("id" => $fsName), $context)) {
				OMVRpc::exec("FsTab", "set", array(
					  "uuid" => $GLOBALS['OMV_UUID_UNDEFINED'],
					  "fsname" => $fsName,
					  "dir" => OMVFilesystem::buildMountPath($fs->getUuid()),
					  "type" => $fs->getType(),
					  "opts" => $opts,
					  "freq" => 0,
					  "passno" => 2
				  ), $context);
				// Apply the changes to the '/etc/fstab' file immediately
				// to mount the filesystem.
				OMVRpc::exec("Config", "applyChanges", array(
					  "modules" => array("fstab"),
					  "force" => TRUE
				  ), $context);
			}
		}
	}

	/**
	 * Unmount a filesystem.
	 * @param params An array containing the following fields:
	 *   \em id The UUID or block special device of the filesystem to unmount.
	 *   \em fstab If set to FALSE, the fstab entry will not be removed (if
	 *     existing), thus the given filesystem is unmounted only.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_EXEC_FAILED
	 * @throw E_MISC_FAILURE
	 */
	public function umount($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "id":{'.$GLOBALS['OMV_JSONSCHEMA_FILESYSTEM_ID'].'},
				  "fstab":{"type":"boolean"}
			  }
		  }');
		// Remove fstab entry?
		if(TRUE === boolval($params['fstab'])) {
			// Get the fstab mount point configuration object.
			if(is_devicefile($params['id'])) {
				// Use the filesystem name to identify the object.
				$xpath = sprintf("//system/fstab/mntent[fsname='%s']",
				  $params['id']);
			} else {
				// Use the filesystem UUID to identify the object by its mount
				// point directory:
				// /media/<fsuuid>
				// /media/D81A-F053 (VFAT)
				// /media/74A225C8A2258F (NTFS)
				// /media/9e8aaee0-ab3a-467f-9bdb-7cced3b8afc4 (EXT2/3/4, JFS, XFS)
				$xpath = sprintf("//system/fstab/mntent[dir='%s']",
				  OMVFilesystem::buildMountPath($params['id']));
			}
			$object = $xmlConfig->get($xpath);
			if(is_null($object)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				  $xpath);
			}
			// Delete the mount point configuration object. Unmount the
			// filesystem and unlink the mount point. Changes to the fstab
			// module must not be applied immediately.
			OMVRpc::exec("FsTab", "delete", array(
			  "uuid" => $object['uuid']), $context);
			OMVRpc::exec("Config", "applyChanges", array(
				  "modules" => array("fstab"),
				  "force" => TRUE
			  ), $context);
		} else { // Only unmount the filesystem.
			$fs = new OMVFilesystem($params['id']);
			if(!$fs->exists()) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf(gettext("Filesystem '%s' not found"),
				  $params['id']));
			}
			if(TRUE === $fs->isMounted()) {
				if(FALSE === $fs->umount(TRUE)) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, $fs->getLastError());
				}
			}
		}
	}

	/**
	 * Check if the given device containes a filesystem that is registered.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file to check.
	 * @param context The context of the caller.
	 * @return TRUE if a filesystem exists on the given device, otherwise
	 *   FALSE.
	 */
	public function hasFilesystem($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'}
			  }
		  }');
		// Check if the given device file contains a filesystem.
		return (FALSE !== OMVFilesystem::hasFileSystem(
		  $params['devicefile']));
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceFileSystemMgmt());
?>
