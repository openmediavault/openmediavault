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
	 *   \em propposixacl, \em propquota, \em propresize, \em propfstab,
	 *   \em mounted and \em percentage. Additional the internal fields
	 *   \em _used and \em _readonly are set.
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
		if (FALSE === ($objects = OMVFilesystems::enumerate())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get filesystems"));
		}
		// Process the detected filesystems and skip unwanted ones.
		$result = array();
		foreach ($objects as $objectk => $objectv) {
			// Is it a filesystem of interest?
			if (!OMVFilesystems::isSupported($objectv['type']))
				continue;
			// Get the filesystem backend.
			$fsb = OMVFilesystems::getBackendByType($objectv['type']);
			if (is_null($fsb)) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("No filesystem backend exists for '%s'",
				  $objectv['type']));
			}
			// Get the filesystem implementation. Use the devicefile to get
			// the filesystem details because VFAT filesystems do not have
			// a valid UUID.
			$fs = $fsb->getImpl($objectv['devicefile']);
			if (is_null($fs) || !$fs->exists()) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
				  "Failed to get the '%s' filesystem implementation or '%s' ".
				  "does not exist", $fsb->getType(), $objectv['devicefile']));
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
				"description" => $fs->hasLabel() ? $fs->getLabel() :
				  $fs->getDeviceFile(),
				"propposixacl" => $fsb->hasPosixAclSupport(),
				"propquota" => $fsb->hasQuotaSupport(),
				"propresize" => $fsb->hasResizeSupport(),
				"propfstab" => $fsb->hasFstabSupport(),
				"_used" => FALSE
			);
			// Check if the filesystem is used. First try to get the
			// corresponding mount point configuration object. If such object
			// exists, then check if it is referenced by any other object,
			// e.g. by a shared folder configuration object.
			if (FALSE !== ($meObject = OMVRpc::exec("FsTab", "getByFsName",
			  array("id" => $fs->getDeviceFile()), $context))) {
			  	$xpath = sprintf("//mntentref[contains(.,'%s')]",
			  	  $meObject['uuid']);
				if (TRUE === $xmlConfig->exists($xpath))
					$object['_used'] = TRUE;
			}
			// Mark the device where the operating system is installed on
			// as used and read-only.
			if (OMVSystem::isRootDeviceFile($object['devicefile'])) {
				$object['_used'] = TRUE;
				$object['_readonly'] = TRUE;
			}
			// If the filesystem is mounted then try to get more
			// informations about it. Note,it is not possible to get
			// details from unmounted filesystems, because on most kinds
			// of systems doing so requires very nonportable intimate
			// knowledge of filesystem structures. See man (1) df.
			if (TRUE === $fs->isMounted()) {
				$object['mounted'] = TRUE;
				// Get some more filesystem details if possible.
				if (FALSE !== ($fsStats = $fs->getStatistics())) {
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
	 * object, except binds, and that are actually mounted.
	 * @param params The method parameters.
	 *   \em includeRoot TRUE to append the filesystem '/dev/root' if mounted.
	 *     Defaults to FALSE.
	 * @param context The context of the caller.
	 * @return An array of objects with the following fields: \em uuid,
	 *   \em devicefile, \em type, \em label, \em blocks, \em size,
	 *   \em mountpoint, \em blocks, \em used, \em available,
	 *   \em description, \em percentage, \em propposixacl, \em propquota,
	 *   \em propresize and \em propfstab.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_MISC_FAILURE
	 */
	public function enumerateMountedFilesystems($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		if (!is_null($params))
			$this->validateMethodParams($params, '{
				  "type":"object",
				  "properties":{
					  "includeRoot":{"type":"boolean","optional":true}
				  }
			  }');
		// Get list of mount points, except bind mounts.
		$xpath = "//system/fstab/mntent[not(contains(opts,'bind'))]";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Append '/dev/root'?
		if (TRUE === array_boolval($params, "includeRoot", FALSE)) {
			array_unshift($objects, array(
				"fsname" => "/dev/root",
				"dir" => "/"
			));
		}
		// Get the filesystem details for each mount point.
		$result = array();
		foreach ($objects as $objectk => $objectv) {
			// Get the filesystem backend.
			$fsb = OMVFilesystems::getBackendByType($objectv['type']);
			if (is_null($fsb)) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("No filesystem backend exists for '%s'",
				  $objectv['type']));
			}
			// Get the filesystem implementation.
			$fs = $fsb->getImpl($objectv['fsname']);
			if (is_null($fs) || !$fs->exists()) {
//				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
//				  "Failed to get the '%s' filesystem implementation or '%s' ".
//				  "does not exist", $fsb->getType(), $objectv['fsname']));
				continue;
			}
			// Check if the given filesystem is mounted based on the configured
			// mount point. Skip the filesystems that are not mounted at the
			// moment.
			if (FALSE === $fs->isMounted())
				continue;
			// Get the filesystem details.
			$object = array(
				"devicefile" => $fs->getDeviceFile(),
				"uuid" => $fs->getUuid(),
				"label" => $fs->getLabel(),
				"type" => $fs->getType(),
				"blocks" => "-1", // as string
				"mountpoint" => $objectv['dir'],
				"used" => "-1", // as string
				"available" => "-1", // as string
				"size" => "-1", // as string
				"percentage" => -1,
				"description" => $fs->hasLabel() ? $fs->getLabel() :
				  $fs->getDeviceFile(),
				"propposixacl" => $fsb->hasPosixAclSupport(),
				"propquota" => $fsb->hasQuotaSupport(),
				"propresize" => $fsb->hasResizeSupport(),
				"propfstab" => $fsb->hasFstabSupport(),
			);
			// Get some more filesystem details if possible.
			if (FALSE !== ($fsStats = $fs->getStatistics())) {
				$object['used'] = binary_format($fsStats['used']);
				$object['available'] = $fsStats['available'];
				$object['percentage'] = $fsStats['percentage'];
				$object['blocks'] = $fsStats['blocks'];
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
	 * Get the list of filesystems that have been detected.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param context The context of the caller.
	 * @return An array of objects with the following fields: \em uuid,
	 *   \em devicefile, \em type, \em label, \em blocks, \em size,
	 *   \em mountpoint, \em blocks, \em used, \em available,
	 *   \em description, \em mounted, \em percentage, \em status,
	 *   \em propposixacl, \em propquota, \em propresize and \em propfstab.
	 *   The field 'status' has the following meaning:<ul>
	 *   \li 1 - Online
	 *   \li 2 - Initializing in progress
	 *   \li 3 - Missing
	 *   </ul>
	 *  Additional the internal fields \em _used and \em _readonly are set.
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
		// Enumerate all detected filesystems.
		$objects = $this->callMethod("enumerateFilesystems", NULL, $context);
		foreach ($objects as $objectk => &$objectv) {
			// Mark each filesystem as as initialized and 'Online'
			// by default.
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
				if(1 !== preg_match($regex, $file->getFilename()))
					continue;
				$fileName = sprintf("/tmp/%s", $file->getFilename());
				// Read the file content and decode JSON data into an
				// associative array.
				$jsonFile = new OMVJsonFile($fileName);
				$jsonFile->open("r");
				$fsInfo = $jsonFile->read();
				$jsonFile->close();
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
					if (TRUE === file_exists($fileName))
						unlink($fileName);
					continue;
				}
				// Get the filesystem backend.
				$fsb = OMVFilesystems::getBackendByType($fsInfo['type']);
				if (is_null($fsb)) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf("No filesystem backend exists for '%s'",
					  $fsInfo['type']));
				}
				// Add the filesystem to the result list.
				$objects[] = array(
					"devicefile" => $fsInfo['devicefile'],
					"uuid" => "", // Not available
					"label" => $fsInfo['label'],
					"type" => $fsInfo['type'],
					"blocks" => "-1", // as string
					"mounted" => FALSE,
					"mountable" => FALSE,
					"mountpoint" => "",
					"used" => "-1", // as string
					"available" => "-1", // as string
					"size" => "-1", // as string
					"percentage" => -1,
					"description" => "",
					"propposixacl" => $fsb->hasPosixAclSupport(),
					"propquota" => $fsb->hasQuotaSupport(),
					"propresize" => $fsb->hasResizeSupport(),
					"propfstab" => $fsb->hasFstabSupport(),
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
		if (is_null($mntents)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		foreach ($mntents as $mntentk => $mntentv) {
			// Get the filesystem backend.
			$fsb = OMVFilesystems::getBackendByType($mntentv['type']);
			if (is_null($fsb)) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("No filesystem backend exists for '%s'",
				  $mntentv['type']));
			}
			// Skip valid mount point configuration objects (the filesystem
			// exists in this case).
			$fs = $fsb->getImpl($mntentv['fsname']);
			if (!is_null($fs) && $fs->exists())
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
				"mounted" => FALSE,
				"mountable" => TRUE,
				"mountpoint" => $mntentv['dir'],
				"used" => "-1", // as string
				"available" => "-1", // as string
				"size" => "-1", // as string
				"percentage" => -1,
				"description" => "",
				"propposixacl" => $fsb->hasPosixAclSupport(),
				"propquota" => $fsb->hasQuotaSupport(),
				"propresize" => $fsb->hasResizeSupport(),
				"propfstab" => $fsb->hasFstabSupport(),
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
		if (FALSE === ($devs = OMVStorageDevices::enumerateUnused())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  "Failed to get list of unused devices");
		}
		// Prepare the result list.
		$result = array();
		foreach ($devs as $devk => $devv) {
			// Get the object that implements the given storage device.
			$sd = OMVStorageDeviceFactory::get($devv);
			if (is_null($sd) || !$sd->exists())
				continue;
			// Check if device is referenced/used by a plugin.
			$xpath = sprintf("//services/devicefile[contains(.,'%s')]",
			  $sd->getDeviceFile());
			if (TRUE === $xmlConfig->exists($xpath))
				continue;
			// Does this device already contain a filesystem?
			if (FALSE !== OMVFilesystem::hasFileSystem($sd->getDeviceFile()))
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
	 * @return The name of the background process status file.
	 * @throw E_MISC_DEVICE_NOT_FOUND
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
				  "type":{"type":"string"},
				  "label":{"type":"string"}
			  }
		  }');
		// Get the storage device object.
		$sd = OMVStorageDeviceFactory::get($params['devicefile']);
		if (is_null($sd) || !$sd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $params['devicefile']);
		}
		// Check uniqueness. If there exists a mount point for the given
		// device then it has already a filesystem that is in use.
		if (FALSE !== OMVRpc::exec("FsTab", "getByFsName", array(
		  "id" => $sd->getDeviceFile()), $context)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
			  sprintf(gettext("There already exists a mount point for %s"),
			  $sd->getDeviceFile()));
		}
		// Get the storage device backend of the given device.
		$sdb = OMVStorageDevices::getBackend($sd->getDeviceFile());
		if (is_null($sdb)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  "No storage device backend exists for device %s",
			  $sd->getDeviceFile()));
		}
		// Get the corresponding filesystem backend.
		$fsb = OMVFilesystems::getBackendByType($params['type']);
		if (is_null($fsb)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  "No filesystem backend exists for type '%s'", $params['type']));
		}
		// Get the filesystem device file name from the storage device
		// backend (this may differ depending on the storage device).
		$fsDeviceFile = $sdb->fsDeviceFile($sd->getDeviceFile());
		// Create a background process.
		$bgStatusFilename = $this->createBgProcStatus();
		$pid = $this->fork();
		if ($pid > 0) { // Parent process.
			$this->initializeBgProcStatus($bgStatusFilename, $pid);
			return $bgStatusFilename;
		}
		// Child process.
		try {
			$bgOutputFilename = $this->createBgProcOutput();
			$this->updateBgProcStatus($bgStatusFilename, "outputfilename",
			  $bgOutputFilename);
			// Create a file that contains the details of the filesystem being
			// initialized. The file is parsed by the 'FileSystemMgmt.getList'
			// RPC to display the state of the filesystem initialization
			// process. There is no other way to detect filesystems being
			// initialized (blkid detects them after the initialization has
			// been finished).
			$fileName = sprintf("/tmp/omv-initfs@%s.build", str_replace(
			  $sd->getDeviceFile(), "/", "_"));
			$jsonFile = new OMVJsonFile($fileName);
			$jsonFile->open("c");
			$jsonFile->write(array(
				"devicefile" => $fsDeviceFile,
				"type" => $fsb->getType(),
				"label" => $params['label']
			));
			$jsonFile->close();
			// Create partitions if necessary.
			switch ($sdb->getType()) {
			case OMV_STORAGE_DEVICE_TYPE_SOFTWARERAID:
			case OMV_STORAGE_DEVICE_TYPE_DEVICEMAPPER:
				// Wipe existing filesystems.
				$cmd = sprintf("export LANG=C; wipefs -a %s 2>&1",
				  $sd->getDeviceFile());
				if (0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
				// No need to create a partition.
				break;
			default:
				// Wipe existing filesystems.
				$cmd = sprintf("export LANG=C; sgdisk --zap-all %s 2>&1",
				  $sd->getDeviceFile());
				if (0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
				// Use entire device.
				$cmd = sprintf("export LANG=C; sgdisk --new=1:0:0 ".
				  "--typecode=1:8300 --print %s 2>&1", escapeshellarg(
				  $sd->getDeviceFile()));
				if (0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
				sleep(2);
				break;
			}
			// Reread partition table.
			$cmd = sprintf("export LANG=C; partprobe %s", $sd->getDeviceFile());
			if (0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			// Create the filesystem.
			$cmdArgs = array();
			$cmdArgs[] = "-V";
			$cmdArgs[] = sprintf("-t %s", $fsb->getType());
			$cmdArgs[] = $fsb->getMkfsOptions($sd);
			if (!empty($params['label']))
				$cmdArgs[] = sprintf("-L %s", escapeshellarg($params['label']));
			$cmdArgs[] = $fsDeviceFile;
			$cmd = sprintf("export LANG=C; mkfs %s 2>&1", implode(" ",
			  $cmdArgs));
			if (0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			// Notify configuration changes.
			$dispatcher = &OMVNotifyDispatcher::getInstance();
			$dispatcher->notify(OMV_NOTIFY_CREATE,
			  "org.openmediavault.system.storage.filesystem", array(
				  "devicefile" => $sd->getDeviceFile(),
				  "type" => $fsb->getType(),
				  "label" => $params['label']
			  ));
			// Cleanup
			$jsonFile->unlink();
			// Finalize background process.
			$this->finalizeBgProcStatus($bgStatusFilename, $output);
			exit(0);
		} catch(Exception $e) {
			// Cleanup.
			$jsonFile->unlink();
			// Finalize background process.
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
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
		// Get the filesystem backend.
		$fsb = OMVFilesystems::getBackend($params['id']);
		if (is_null($fsb)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("No filesystem backend exists for '%s'",
			  $params['id']));
		}
		// Check if the filesystem supports online resizing.
		if (!$fsb->hasResizeSupport()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("The filesystem '%s' (type=%s) does not support ".
			  "online resizing", $params['id'], $fsb->getType()));
		}
		// Get the filesystem implementation.
		$fs = $fsb->getImpl($params['id']);
		if (is_null($fs) || !$fs->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  "Failed to get the '%s' filesystem implementation or '%s' ".
			  "does not exist", $fsb->getType(), $params['id']));
		}
		// Grow the filesystem.
		if (!$fs->grow()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to grow the filesystem '%s': %s",
			  $fs->getDeviceFile(), $fs->getLastError()));
		}
	}

	/**
	 * Delete a filesystem. The filesystem will be unmounted and deleted.
	 * @param params An array containing the following fields:
	 *   \em id The UUID or block special device of the filesystem to delete.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_FAILURE
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

		// !!! Note !!!
		// If the filesystem is missing some of the following code paths
		// are ignored. In this case only the configuration is modified.

		// Initialize the default notification object.
		$fsObject = array(
			"devicefile" => is_devicefile($params['id']) ? $params['id'] : "",
			"uuid" => is_uuid($params['id']) ? $params['id'] : "",
			"label" => "",
			"type" => ""
		);
		// Get the fstab mount point configuration object that belongs to
		// the filesystem to be deleted.
		if (is_devicefile($params['id'])) {
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
		// Get the filesystem if available.
		$fs = OMVFilesystemFactory::get($params['id']);
		// Update the notification object.
		if (!is_null($fs) && $fs->exists()) {
			$fsObject = array(
				"devicefile" => $fs->getDeviceFile(),
				"uuid" => $fs->getUuid(),
				"label" => $fs->getLabel(),
				"type" => $fs->getType()
			);
		} else if (!is_null($meObject)) {
			$fsObject['type'] = $meObject['type'];
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.system.storage.filesystem", $fsObject);
		// Delete the associated fstab mount point entry.
		if (!is_null($meObject)) {
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
		// Finally erase the filesystem.
		if ($fs->exists()) {
			if (FALSE === $fs->remove()) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("Failed to remove filesystem '%s': %s",
				  $fs->getDeviceFile(), $fs->getLastError()));
			}
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
		// Get the corresponding filesystem backend.
		$fsb = OMVFilesystems::getBackend($params['id']);
		if (is_null($fsb)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  "Filesystem '%s' not found or no backend exists",
			  $params['id']));
		}
		// Get the filesystem.
		$fs = $fsb->getImpl($params['id']);
		if (is_null($fs) || !$fs->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  "Failed to get the '%s' filesystem implementation or '%s' ".
			  "does not exist", $fsb->getType(), $params['id']));
		}
		// Get the storage device containing the filesystem, e.g. /dev/sdb or
		// /dev/cciss/c0d0.
		if (FALSE === ($storageDeviceFile = $fs->getStorageDeviceFile())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  gettext("Failed to get storage device file from '%s'"),
			  $fs->getDeviceFile()));
		}
		// Get the according storage device object.
		$sd = OMVStorageDeviceFactory::get($storageDeviceFile);
		if (is_null($sd) || !$sd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $storageDeviceFile);
		}
		// Get mount options depending on the filesystem type.
		switch (strtolower($fs->getType())) {
		case "ext2":
		case "ext3":
		case "ext4":
		case "jfs":
		case "xfs":
		case "hfsplus":
		case "btrfs":
			$fsName = $fs->getUuid();
			$opts = $fsb->getFstabMntOptions($sd);
			break;
		case "vfat":
		case "ntfs":
			$fsName = $fs->getDeviceFileByUuid();
			$opts = $fsb->getFstabMntOptions($sd);
			break;
		default:
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("Filesystem '%s' not supported"),
			  $fs->getType()));
			break;
		}
		// Create fstab entry?
		if (TRUE === boolvalEx($params['fstab'])) {
			// Check for duplicates. Create a new mount point configuration
			// object if necessary.
			if (FALSE === ($object = OMVRpc::exec("FsTab", "getByFsName",
			  array("id" => $fsName), $context))) {
				OMVRpc::exec("FsTab", "set", array(
					  "uuid" => $GLOBALS['OMV_UUID_UNDEFINED'],
					  "fsname" => $fsName,
					  "dir" => OMVFilesystem::buildMountPath($fs->getUuid()),
					  "type" => $fs->getType(),
					  "opts" => implode(",", $opts),
					  "freq" => 0,
					  "passno" => 2
				  ), $context);
				// Apply the changes to the '/etc/fstab' file immediately
				// to mount the filesystem.
				OMVRpc::exec("Config", "applyChanges", array(
					  "modules" => array("fstab"),
					  "force" => TRUE
				  ), $context);
			} else {
				// Mount the file system.
				$me = new OMVMntEnt($object['fsname'], $object['dir']);
				// Umount the mount point if it is already in use, which
				// is the case when an USB device is unplugged without
				// unmounting it.
				if (TRUE == $me->isMounted()) {
					if (FALSE === $me->umount()) {
						throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
						  sprintf("Failed to umount '%s': %s",
						  $object['dir'], $me->getLastError()));
					}
				}
				if (FALSE === $fs->mount()) {
					throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
					  sprintf("Failed to mount '%s': %s",
					  $object['fsname'], $fs->getLastError()));
				}
			}
		} else {
			// Mount the filesystem.
			if (TRUE === $fs->isMounted()) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("The filesystem '%s' is already mounted",
				  $fsName));
			}
			if (FALSE === $fs->mount()) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("Failed to mount '%s': %s", $fsName,
				  $fs->getLastError()));
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
		if (TRUE === boolvalEx($params['fstab'])) {
			// Get the fstab mount point configuration object.
			if (is_devicefile($params['id'])) {
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
			if (is_null($object)) {
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
			$fs = OMVFilesystemFactory::get($params['id']);
			if (is_null($fs) || !$fs->exists()) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf(gettext("Filesystem '%s' not found"),
				  $params['id']));
			}
			if (TRUE === $fs->isMounted()) {
				if (FALSE === $fs->umount(TRUE)) {
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
