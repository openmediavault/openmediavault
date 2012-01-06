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

class FileSystemMgmtRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getList" => array(
				'{"type":"integer"}', // start
				'{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}' // sortDir
			),
			"create" => array('{
				"type":"object",
				"properties":{
					"devicefile":{"type":"string"},
					"type":{"type":"string","enum":["ext3","ext4",'.
					  '"jfs","xfs"]},
					"label":{"type":"string"}
				}
			}'),
			"resize" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_FILESYSTEM_UUID'].'}'
			),
			"delete" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_FILESYSTEM_ID'].'}'
			),
			"mount" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_FILESYSTEM_UUID'].'}'
			),
			"umount" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_FILESYSTEM_ID'].'}'
			),
			"hasFilesystem" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'}'
			)
		);
	}

	/**
	 * Enumerate all filesystems that have been detected, except the
	 * filesystem containing the operation system.
	 * @return An array of objects with the following fields: \em uuid,
	 * \em devicefile, \em type, \em label, \em blocks, \em capacity,
	 * \em mountpoint, \em blocks, \em used, \em available, \em description,
	 * \em mounted and \em percentage.
	 */
	public function enumerateFilesystems() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get list of all detected filesystems. Note, the list may also
		// contain RAID and LVM volumes, so the list must be filtered.
		if (FALSE === ($objects = OMVFilesystems::get())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get filesystems"));
		}
		// Get the root partition (the filesystem containing the operation
		// system) to filter it out.
		$rootDevice = OMVSystem::getRootPartition();
		// Process the detected filesystems and skip unwanted ones.
		$result = array();
		foreach ($objects as $objectk => $objectv) {
			// Is it a filesystem of interest?
			if (!in_array($objectv['type'], array("ext", "ext2", "ext3",
			  "ext4", "xfs", "jfs", "iso9660", "msdos", "ntfs", "udf",
			  "umsdos", "vfat", "ufs", "reiserfs", "btrfs"))) {
				continue;
			}
			// Skip the device where the operation system is installed on.
			if ($objectv['devicefile'] === $rootDevice) {
				continue;
			}
			// Use the devicefile to get the filesystem details because vfat
			// filesystems do not have a valid UUID. Any idea why?
			$fs = new OMVFilesystem($objectv['devicefile']);
			if (!$fs->exists()) {
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
				"blocks" => "0", // as string
				"mounted" => FALSE,
				"mountpoint" => "",
				"used" => gettext("n/a"),
				"available" => gettext("n/a"),
				"capacity" => gettext("n/a"),
				"percentage" => -1,
				"description" => "",
				"_used" => FALSE
			);
			// Check if the filesystem is used. First try to get the
			// corresponding mount point configuration object. If such object
			// exists, then check if it is referenced by any other object,
			// e.g. by a shared folder configuration object.
			if (FALSE !== ($meObject = OMVRpc::exec("FsTab", "getByFsName",
			  array($fs->getDeviceFile())))) {
				if (TRUE === $xmlConfig->exists("//mntentref[".
				  "contains(.,'{$meObject['uuid']}')]")) {
					$object['_used'] = TRUE;
				}
			}
			// If the filesystem is mounted then try to get more
			// informations about it. Note,it is not possible to get
			// details from unmounted filesystems, because on most kinds
			// of systems doing so requires very nonportable intimate
			// knowledge of file system structures. See man (1) df.
			if (TRUE === $fs->isMounted()) {
				$object['mounted'] = TRUE;
				// Get some more filesystem details if possible.
				if (FALSE !== ($fsStats = $fs->getStatistics())) {
					$object['used'] = binary_format($fsStats['used']);
					$object['available'] = $fsStats['available'];
					$object['percentage'] = $fsStats['percentage'];
					$object['blocks'] = $fsStats['blocks'];
					$object['mountpoint'] = $fsStats['mountpoint'];
					$object['capacity'] = binary_format(
					  $fsStats['available']);
					$object['description'] = sprintf(
					  gettext("%s (%s available)"),
					  !empty($object['label']) ? $object['label'] :
					  $object['devicefile'], $object['capacity']);
				}
			}
			$result[] = $object;
		}
		return $result;
	}

	/**
	 * Enumerate all filesystems that have a mount point configuration object,
	 * except binds, and are actually mounted.
	 * @return An array of objects with the following fields: \em uuid,
	 * \em devicefile, \em type, \em label, \em blocks, \em capacity,
	 * \em mountpoint, \em blocks, \em used, \em available, \em description,
	 * and \em percentage.
	 */
	public function enumerateMountedFilesystems() {
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
				"devicefile" => $fs->getDeviceFile(),
				"uuid" => $fs->getUuid(),
				"label" => $fs->getLabel(),
				"type" => $fs->getType(),
				"blocks" => "0", // as string
				"mountpoint" => "",
				"used" => gettext("n/a"),
				"available" => gettext("n/a"),
				"capacity" => gettext("n/a"),
				"percentage" => -1,
				"description" => ""
			);
			// Get some more filesystem details if possible.
			if (FALSE !== ($fsStats = $fs->getStatistics())) {
				$object['used'] = binary_format($fsStats['used']);
				$object['available'] = $fsStats['available'];
				$object['percentage'] = $fsStats['percentage'];
				$object['blocks'] = $fsStats['blocks'];
				$object['mountpoint'] = $fsStats['mountpoint'];
				$object['capacity'] = binary_format($fsStats['available']);
				$object['description'] = sprintf(
				  gettext("%s (%s available)"), !empty($object['label']) ?
				  $object['label'] : $object['devicefile'],
				  $object['capacity']);
			}
			$result[] = $object;
		}
		return $result;
	}

	/**
	 * Get the list of filesystems that have been detected, except the
	 * filesystem containing the operation system.
	 * @return An array of objects with the following fields: \em uuid,
	 * \em devicefile, \em type, \em label, \em blocks, \em capacity,
	 * \em mountpoint, \em blocks, \em used, \em available, \em description,
	 * \em mounted, \em percentage and \em status.
	 * The field 'status' has the following meaning:
	 * 1 - Online
	 * 2 - Initializing in progress
	 * 3 - Missing
	 */
	public function getList($start, $count, $sortField, $sortDir) {
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
		// Enumerate all detected filesystems. Mark each as initialized.
		$objects = OMVRpc::exec("FileSystemMgmt", "enumerateFilesystems");
		foreach ($objects as $objectk => &$objectv) {
			$objectv['status'] = 1;
		}
		// Try to detect filesystems that are being initialized.
		foreach (new DirectoryIterator("/tmp") as $file) {
			if ($file->isDot())
				continue;
			if ($file->isFile()) {
				// Check if it is a file we are interested in. The filename
				// must look like omv-initfs@<device>.build, e.g.
				// omv-initfs@_dev_sdb.build
				$regex = '/^omv-initfs@.+\.build$/i';
				if (1 !== preg_match($regex, $file->getFilename()))
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
				foreach ($objects as $objectk => &$objectv) {
					if ($objectv['devicefile'] === $fsInfo['devicefile']) {
						$initialized = TRUE;
						break;
					}
				}
				if (TRUE === $initialized) {
					if (TRUE === file_exists($fileName)) {
						unlink($fileName);
					}
					continue;
				}
				// Add the filesystem to the result list.
				$objects[] = array(
					"devicefile" => $fsInfo['devicefile'],
					"uuid" => "", // Not available
					"label" => $fsInfo['label'],
					"type" => $fsInfo['type'],
					"blocks" => "0", // as string
					"mounted" => FALSE,
					"mountpoint" => "",
					"used" => gettext("n/a"),
					"available" => gettext("n/a"),
					"capacity" => gettext("n/a"),
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
		$xpath = "//system/fstab/mntent[not(contains(opts,'bind'))]";
		$mntents = $xmlConfig->getList($xpath);
		if (is_null($mntents)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		foreach ($mntents as $mntentk => $mntentv) {
			// Skip valid mount point configuration objects (the filesystem
			// exists in this case).
			$fs = new OMVFilesystem($mntentv['fsname']);
			if ($fs->exists()) {
				continue;
			}
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
				"blocks" => "0", // as string
				"mounted" => TRUE,
				"mountpoint" => $mntentv['dir'],
				"used" => gettext("n/a"),
				"available" => gettext("n/a"),
				"capacity" => gettext("n/a"),
				"percentage" => -1,
				"description" => "",
				"status" => 3,
				"_used" => $xmlConfig->exists($xpath)
			);
		}
		// Filter result
		return $this->applyFilter($objects, $start, $count, $sortField,
		  $sortDir);
	}

	/**
	 * Get list of devices that can be used to create a filesystem on.
	 * @return An array containing objects with the following fields:
	 * devicefile, size, capacity and description.
	 */
	public function getCandidates() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$result = array();
		// Prepare list of used devices.
		$usedDevs = array();
		// Get list of filesystems.
		$fs = OMVFilesystems::get();
		// Get the devices that are used by a physical volume and add them to
		// the list of used devices.
		if (FALSE === ($devs = OMVPhysicalVolumes::get())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of physical volumes");
		}
		$usedDevs = array_merge($usedDevs, $devs);
		// Get all existing logical volumes.
		if (FALSE === ($devs = OMVLogicalVolumes::get())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of logical volumes");
		}
		foreach ($devs as $devk => $devv) {
			$lv = new OMVLogicalVolume($devv);
			if (!$lv->exists())
				continue;
			// Does this device already contain a filesystem?
			if (array_key_exists($lv->getDeviceFile(), $fs))
				continue;
			$result[] = array(
				"devicefile" => $lv->getDeviceFile(),
				"size" => $lv->getSize(),
				"capacity" => $lv->getCapacity(),
				"description" => $lv->getDescription()
			);
		}
		// Get all existing RAID devices.
		if (FALSE === ($devs = OMVRaidDevices::get())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of RAID devices");
		}
		foreach ($devs as $devk => $devv) {
			$raid = new OMVRaidDevice($devv);
			if (!$raid->exists())
				continue;
			// Add the devices used by the RAID to the list of used devices.
			$usedDevs = array_merge($usedDevs, $raid->getDevices());
			// Is this RAID device already used as a physical volume?
			if (in_array($raid->getDeviceFile(), $usedDevs))
				continue;
			// Does this device already contain a filesystem?
			if (array_key_exists($raid->getDeviceFile(), $fs))
				continue;
			$result[] = array(
				"devicefile" => $raid->getDeviceFile(),
				"size" => $raid->getSize(),
				"capacity" => $raid->getCapacity(),
				"description" => $raid->getDescription()
			);
		}
		// Get all physical disk devices on the system.
		unset($devs);
		$devs = OMVRpc::exec("DiskMgmt", "enumerateDevices");
		// Prepare result
		foreach ($devs as $devk => $devv) {
			// Is this hard disk device used in a RAID device?
			if (in_array($devv['devicefile'], $usedDevs))
				continue;
			// Check if device is referenced/used by a plugin.
			if (TRUE === $xmlConfig->exists("//services/devicefile[".
			  "contains(.,'{$devv['devicefile']}')]"))
				continue;
			// Does this device already contain a filesystem?
			if (TRUE === OMVFilesystem::hasFileSystem($devv['devicefile']))
				continue;
			$result[] = array(
				"devicefile" => $devv['devicefile'],
				"size" => $devv['size'],
				"capacity" => $devv['capacity'],
				"description" => $devv['description']
			);
		}
		return $result;
	}

	/**
	 * Create a filesystem on the given device.
	 */
	public function create($data) {
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
		// Check uniqueness. If there exists a mount point for the given
		// device then it has already a filesystem that is in usage.
		if (FALSE !== OMVRpc::exec("FsTab", "getByFsName", array(
		  $data['devicefile']))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
			  sprintf(gettext("There already exists a mount point for %s"),
			  $data['devicefile']));
		}
		// Create the filesysteme UUID.
		$object = array(
			"devicefile" => $data['devicefile'],
			"type" => $data['type'],
			"label" => $data['label']
		);
		// Notify registered modules. The filesystem will be created then.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.filesystem", $object);
		// Stupid workaround to ensure that the filesystem creation was
		// successful (there is no other situation where the WebGUI can be
		// notified that the creation has been failed):
		// - Wait some seconds
		// - Check if a error log exists
		// - Throw an exception if necessary to notify display an error
		//   message in the WebGUI
		sleep(2);
		$fileName = sprintf("omv-initfs@%s", strtr($data['devicefile'],
		  "/", "_"));
		$filePath = sprintf("/tmp/%s.fail", $fileName);
		if (@file_exists($filePath)) {
			$filePath = sprintf("%s/%s.log", $GLOBALS['OMV_LOG_DIR'],
			  $fileName);
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  gettext("Failed to create filesystem. Please check the ".
			  "syslog or '%s' for more details."), $filePath));
		}
	}

	/**
	 * Resize a filesystem.
     * @param uuid The UUID of the filesystem to resize.
	 */
	public function resize($uuid) {
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
		// Get filesystem informations.
		$fs = new OMVFilesystem($uuid);
		if (!$fs->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("Filesystem '%s' not found"), $uuid));
		}
		// Grow the filesystem
		switch (strtolower($fs->getType())) {
		case "ext":
		case "ext2":
		case "ext3":
		case "ext4":
			$cmd = sprintf("sudo resize2fs %s", $fs->getDeviceFile());
			break;
		case "xfs":
			$cmd = sprintf("sudo xfs_growfs %s",
			  OMVFilesystem::buildMountPath($fs->getUuid()));
			break;
		case "jfs":
			$cmd = sprintf("sudo mount -o remount,resize %s",
			  OMVFilesystem::buildMountPath($fs->getUuid()));
			break;
		default:
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("Unknown filesystem '%s'"), $fs->getType()));
			break;
		}
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Delete a filesystem.
	 * The filesystem will be umounted and deleted.
     * @param id The UUID or devicefile of the filesystem to delete.
	 */
	public function delete($id) {
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
		// Get filesystem informations.
		$fs = new OMVFilesystem($id);
		if (!$fs->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("Filesystem '%s' not found"), $id));
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
		// Notify configuration changes.
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.filesystem", $fsObject);
	}

	/**
	 * Mount a filesystem.
     * @param uuid The UUID of the filesystem to mount.
	 */
	public function mount($uuid) {
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
		// Get the filesystem.
		$fs = new OMVFilesystem($uuid);
		if (!$fs->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("Filesystem '%s' not found"), $uuid));
		}
		// Get mount options depending on the filesystem type.
		switch ($fs->getType()) {
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
		case "ntfs":
			$fsName = $fs->getDeviceFile();
			$opts = "defaults";
			break;
		default:
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  gettext("Filesystem '%s' not supported"), $type));
			break;
		}
		// Check for duplicates.
		$xpath = sprintf("//system/fstab/mntent[fsname='%s']", $fsName);
		if (TRUE === $xmlConfig->exists($xpath)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
			  gettext("A mount point already exists for the given filesystem"));
		}
		// Prepare fstab mount point entry configuration object.
		$object = array(
			"uuid" => OMVUtil::uuid(),
			"fsname" => $fsName,
			"dir" => OMVFilesystem::buildMountPath($fs->getUuid()),
			"type" => $fs->getType(),
			"opts" => $opts,
			"freq" => 0,
			"passno" => 2
		);
		// Append objects to configuration.
		if (FALSE === $xmlConfig->set("//system/fstab", array(
		  "mntent" => $object))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save the configuration.
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.fstab.mntent", $object);
		// Mount the filesystem.
		$cmd = sprintf("sudo mount %s", is_uuid($fsName) ? "-U {$fsName}" :
		  $fsName);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Umount a filesystem.
     * @param id The UUID or devicefile of the filesystem to umount.
	 */
	public function umount($id) {
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
		// Get the configuration object.
		if (is_devicefile($id)) {
			// Use the filesystem name to identify the object.
			$xpath = sprintf("//system/fstab/mntent[fsname='%s']", $id);
		} else {
			// Use the filesystem UUID to identify the object by its mount
			// point directory:
			// /media/<fsuuid>
			// /media/D81A-F053 (VFAT)
			// /media/74A225C8A2258F (NTFS)
			// /media/9e8aaee0-ab3a-467f-9bdb-7cced3b8afc4 (EXT2/3/4, JFS, XFS)
			$xpath = sprintf("//system/fstab/mntent[dir='%s']",
			  OMVFilesystem::buildMountPath($id));
		}
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Unmount the filesystem if it is mounted.
		if (TRUE === OMVFilesystem::isMountedEx("mountpoint", $object['dir'])) {
			$cmd = sprintf("sudo umount -f %s", $object['dir']);
			OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
		}
		// Delete the mount point configuration object.
		OMVRpc::exec("FsTab", "delete", array($object['uuid']));
	}

	/**
	 * Check if the given device containes a filesystem that is registered.
	 * @param devicefile The device file to check.
	 */
	public function hasFilesystem($devicefile) {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Check if the given device file contains a file system.
		return OMVFilesystem::hasFileSystem($devicefile);
	}
}
?>
