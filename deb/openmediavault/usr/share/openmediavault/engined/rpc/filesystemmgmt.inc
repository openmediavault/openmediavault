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

class OMVRpcServiceFileSystemMgmt extends \OMV\Rpc\ServiceAbstract {
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
		$this->registerMethod("getListBg");
		$this->registerMethod("getCandidates");
		$this->registerMethod("getCandidatesBg");
		$this->registerMethod("getMountCandidates");
		$this->registerMethod("create");
		$this->registerMethod("createBtrfs");
		$this->registerMethod("grow");
		$this->registerMethod("setMountPoint");
		$this->registerMethod("umount");
		$this->registerMethod("hasFilesystem");
		$this->registerMethod("getDetails");
	}

	/**
	 * Helper function to get information about the given file system.
	 */
	private function getFsInfo($fs = NULL, $fsb = NULL) {
		$result = [
			"devicename" => "",
			"devicefile" => "",
			"predictabledevicefile" => "",
			"canonicaldevicefile" => "",
			"parentdevicefile" => "",
			"devlinks" => [],
			"uuid" => "",
			"label" => "",
			"type" => "",
			"blocks" => "-1", // as string
			"mounted" => FALSE,
			"mountpoint" => "",
			"used" => "-1", // as string
			"available" => "-1", // as string
			"size" => "-1", // as string
			"percentage" => -1,
			"description" => "",
			"propposixacl" => "",
			"propquota" => "",
			"propresize" => "",
			"propfstab" => "",
			"propcompress" => "",
			"propautodefrag" => "",
			"hasmultipledevices" => "",
			"devicefiles" => [],
			"comment" => "",
			"_readonly" => FALSE,
			"_used" => FALSE
		];
		if (!is_null($fs) && $fs->exists()) {
			$result = array_merge($result, [
				"devicename" => $fs->getDeviceName(),
				"devicefile" => $fs->getPreferredDeviceFile(),
				"predictabledevicefile" => $fs->getPredictableDeviceFile(),
				"canonicaldevicefile" => $fs->getCanonicalDeviceFile(),
				"parentdevicefile" => $fs->getParentDeviceFile(),
				"devlinks" => $fs->getDeviceFileSymlinks(),
				"mounted" => $fs->isMounted(),
				"uuid" => $fs->getUuid(),
				"label" => $fs->getLabel(),
				"type" => $fs->getType(),
				"description" => $fs->getDescription(),
				"hasmultipledevices" => $fs->hasMultipleDevices(),
				"devicefiles" => $fs->getDeviceFiles()
			]);
			if (TRUE === $result['mounted']) {
				if (FALSE !== ($fsStats = $fs->getStatistics())) {
					$result = array_merge($result, [
						"used" => binary_format($fsStats['used']),
						"available" => $fsStats['available'],
						"percentage" => $fsStats['percentage'],
						"blocks" => $fsStats['blocks'],
						"mountpoint" => $fsStats['mountpoint'],
						"size" => $fsStats['size']
					]);
				}
			}
		};
		if (!is_null($fsb)) {
			$result = array_merge($result, [
				"propposixacl" => $fsb->hasPosixAclSupport(),
				"propquota" => $fsb->hasQuotaSupport(),
				"propresize" => $fsb->hasResizeSupport(),
				"propfstab" => $fsb->hasFstabSupport(),
				"propreadonly" => $fsb->hasReadOnlySupport(),
				"propcompress" => $fsb->hasCompressSupport(),
				"propautodefrag" => $fsb->hasAutoDefragSupport()
			]);
		}
		return $result;
	}

	/**
	 * Helper function to get the mount point configuration object.
	 */
	private function getMountPointConfigObject($object) {
		// Check if the file system is in use. First try to get the
		// corresponding mount point configuration object. Query the
		// database for all device files that exist for the given
		// file system to increase the probability to find the mount
		// point configuration object.
		$fsNames = $object['devlinks'];
		$fsNames[] = $object['canonicaldevicefile'];
		if (TRUE === is_fs_uuid($object['uuid'])) {
			// To keep backward compatibility we need to search for
			// the file system UUID, too.
			$fsNames[] = $object['uuid'];
		}
		$db = \OMV\Config\Database::getInstance();
		$result = $db->getByFilter("conf.system.filesystem.mountpoint", [
			"operator" => "stringEnum",
			"arg0" => "fsname",
			// Remove duplicates and re-index the array.
			"arg1" => array_values(array_unique($fsNames))
		]);
		return empty($result) ? NULL : $result[0];
	}

	/**
	 * Helper function to check if the given file system is in use.
	 */
	private function hasMountPointConfigObject($object): bool {
		return !is_null($this->getMountPointConfigObject($object));
	}

	/**
	 * Enumerate all file systems that have been detected, except the
	 * file system containing the operation system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects with the following fields: \em uuid,
	 *   \em devicefile, \em type, \em label, \em blocks, \em size,
	 *   \em mountpoint, \em blocks, \em used, \em available, \em description,
	 *   \em propposixacl, \em propquota, \em propresize, \em propfstab,
	 *   \em mounted and \em percentage. Additional the internal fields
	 *   \em _used and \em _readonly are set.
	 * @throw \OMV\Exception
	 */
	public function enumerateFilesystems($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get list of all detected file systems.
		$filesystems = \OMV\System\Filesystem\Filesystem::getFilesystems();
		// Process the detected file systems and skip unwanted ones.
		$procs = [];
		foreach ($filesystems as $fs) {
			// Collect the file system information asynchronous.
			$procs[] = $this->asyncProc(function() use ($fs) {
				// Get the file system backend.
				$fsb = $fs->getBackend();
				if (is_null($fsb)) {
					throw new \OMV\Exception(
						"No file system backend set for '%s'.",
						$fs->getDeviceFile());
				}
				// Set default values.
				$object = $this->getFsInfo($fs, $fsb);
				// Check if the file system is in use by getting the mount
				// point configuration objects.
				$mpObject = $this->getMountPointConfigObject($object);
				// If such object exist, then check if it is referenced by
				// any other object, e.g. by a shared folder configuration
				// object.
				if (!is_null($mpObject)) {
					$db = \OMV\Config\Database::getInstance();
					$object['_used'] = $db->isReferenced($mpObject);
				}
				// Mark the device where the operating system is installed on
				// as used and read-only.
				if (\OMV\System\System::isRootDeviceFile($object['devicefile'])) {
					$object['_used'] = TRUE;
					$object['_readonly'] = TRUE;
				}
				return $object;
			});
		}
		return $this->waitAsyncProcs($procs);
	}

	/**
	 * Enumerate all file systems that have a mount point configuration
	 * object, except binds, and that are actually mounted.
	 * @param params The method parameters.
	 *   \em type Optional. Return only the file system of the specified type.
	 *   \em includeroot Optional. Set to TRUE to append the file system
	 *     '/dev/root' if mounted. Defaults to FALSE.
	 * @param context The context of the caller.
	 * @return An array of objects with the following fields: \em uuid,
	 *   \em devicefile, \em type, \em label, \em blocks, \em size,
	 *   \em mountpoint, \em blocks, \em used, \em available,
	 *   \em description, \em percentage, \em propposixacl, \em propquota,
	 *   \em propresize and \em propfstab.
	 */
	public function enumerateMountedFilesystems($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		if (!is_null($params)) {
			$this->validateMethodParams($params,
				"rpc.filesystemmgmt.enumeratemountedfilesystems");
		}
		// Get list of mount points, except bind mounts.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.system.filesystem.mountpoint", [
			"operator" => "not",
			"arg0" => [
				"operator" => "stringContains",
				"arg0" => "opts",
				"arg1" => "bind"
			]
		]);
		// Append root file system (/dev/root)?
		// Make sure the configuration object is not already in the list
		// of mount points. This happens if the 'sharerootfs' plugin is
		// installed.
		if (
			array_boolval($params, "includeroot", FALSE) &&
			(FALSE === $db->exists("conf.system.filesystem.mountpoint", [
				"operator" => "stringEquals",
				"arg0" => "dir",
				"arg1" => "/"
			]))
		) {
			$rootObject = new \OMV\Config\ConfigObject(
				"conf.system.filesystem.mountpoint");
			$rootObject->set("fsname", \OMV\System\System::getRootDeviceFile());
			$rootObject->set("dir", "/");
			$rootObject->set("comment", "");
			$rootObject->set("usagewarnthreshold", \OMV\Environment::get(
				"OMV_MONIT_SERVICE_FILESYSTEM_ROOT_USAGEWARNTHRESHOLD",
				85));
			array_unshift($objects, $rootObject);
		}
		// Get the file system details for each mount point.
		$procs = [];
		foreach ($objects as $objectk => $objectv) {
			// Process only the specified file system types?
			if (!is_null($params) && array_key_exists('type', $params) &&
					(array_value($params, 'type') !== $objectv->get("type"))) {
				continue;
			}
			// Collect the file system information asynchronous.
			$procs[] = $this->asyncProc(function() use ($objectv) {
				// Get the file system backend.
				$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
				$fsb = $fsbMngr->getBackendById($objectv->get("fsname"));
				if (is_null($fsb)) {
					// The device may not exist anymore, e.g. a USB device. Skip it.
					// throw new \OMV\Exception(
					//     "No file system backend exists for '%s'",
					//     $objectv['fsname']);
					return FALSE;
				}
				// Get the file system implementation.
				$fs = $fsb->getImpl($objectv->get("fsname"));
				if (is_null($fs) || !$fs->exists()) {
					// throw new \OMV\Exception(
					//     "Failed to get the '%s' file system implementation or '%s' ".
					//     "does not exist", $fsb->getType(), $objectv['fsname']);
					return FALSE;
				}
				// Check if the given file system is mounted based on the configured
				// mount point. Skip the file systems that are not mounted at the
				// moment.
				if (FALSE === $fs->isMounted()) {
					return FALSE;
				}
				// Get the file system details.
				return array_merge($this->getFsInfo($fs, $fsb), [
					"comment" => $objectv->get("comment"),
					"usagewarnthreshold" => $objectv->get("usagewarnthreshold"),
					// Override the dynamically determined mount point with the
					// configured one.
					"mountpoint" => $objectv->get("dir")
				]);
			});
		}
		// Remove elements that are no associative arrays (e.g. these are
		// file systems that are not mounted).
		// Re-index the filtered array.
		return array_values(array_filter(
			$this->waitAsyncProcs($procs), "is_assoc_array")
		);
	}

	/**
	 * Get the file systems that can be mounted.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects with the following fields: \em uuid,
	 *   \em devicefile, \em type, \em label, \em blocks, \em size,
	 *   \em mountpoint, \em blocks, \em used, \em available,
	 *   \em description, \em percentage, \em propposixacl, \em propquota,
	 *   \em propresize, \em propfstab, ...
	 */
	public function getMountCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$result = [];
		$objects = $this->callMethod("enumerateFilesystems", NULL, $context);
		$db = \OMV\Config\Database::getInstance();
		foreach ($objects as $objectk => $objectv) {
			// Ignore file systems that are already mounted.
			if ($objectv['mounted']) {
				continue;
			}
			// Skip some specific file systems.
			if (in_array($objectv['type'], ['swap', 'bcache'])) {
				continue;
			}
			// Is there already an mount point configuration object?
			if ($this->hasMountPointConfigObject($objectv)) {
				continue;
			}
			$result[] = $objectv;
		}
		return $result;
	}

	/**
	 * Get the list of configured file systems.
	 * @param object $params An object containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param object $context The context of the caller.
	 * @return array An array of objects with the following fields: \em uuid,
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
	 * @ŧhrow \OMV\Exception
	 */
	public function getList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get a list of all detected file systems.
		$detectedFsObjects = $this->callMethod("enumerateFilesystems",
			NULL, $context);
		// Try to detect file systems that are being initialized.
		$result = [];
		if (file_exists("/tmp")) {
			foreach (new \DirectoryIterator("/tmp") as $file) {
				if ($file->isDot())
					continue;
				if (!$file->isFile())
					continue;
				// Check if it is a file we are interested in. The filename
				// must look like omv-initfs@<device>.build, e.g.
				// omv-initfs@_dev_sdb.build
				$regex = '/^omv-initfs@.+\.build$/i';
				if (1 !== preg_match($regex, $file->getFilename()))
					continue;
				$fileName = sprintf("/tmp/%s", $file->getFilename());
				// Read the file content and decode JSON data into an
				// associative array.
				$jsonFile = new \OMV\Json\File($fileName);
				$jsonFile->open("r");
				$fsInfo = $jsonFile->read();
				$jsonFile->close();
				// Check whether the file system initialization process has
				// been finished already. This is done by simply checking if
				// the device file is already in the list of detected file
				// systems. If this is the case, then unlink the file system
				// build file.
				$found = FALSE;
				foreach ($detectedFsObjects as $fsObjectk => $fsObjectv) {
					$fsNames = $fsObjectv['devlinks'];
					$fsNames[] = $fsObjectv['devicefile'];
					$fsNames[] = $fsObjectv['canonicaldevicefile'];
					if (in_array($fsInfo['devicefile'], $fsNames)) {
						$found = TRUE;
						break;
					}
				}
				if (TRUE === $found) {
					if (TRUE === $jsonFile->exists()) {
						$jsonFile->unlink();
					}
					continue;
				}
				// The file system creation is still in progress. Get as much
				// as possible information about the file system and append
				// them to the result list.
				$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
				$fsb = $fsbMngr->getBackendByType($fsInfo['type']);
				if (is_null($fsb)) {
					throw new \OMV\Exception(
						"No file system backend exists for '%s'.",
						$fsInfo['type']);
				}
				$result[] = array_merge($this->getFsInfo(NULL, $fsb), [
					"devicefile" => $fsInfo['devicefile'],
					"parentdevicefile" => $fsInfo['parentdevicefile'],
					"devicefiles" => [ $fsInfo['devicefile'] ],
					"label" => $fsInfo['label'],
					"type" => $fsInfo['type'],
					"status" => 2, // Initializing
				]);
			}
		}
		// Get the configured file systems except 'bind' and 'loop' devices.
		$db = \OMV\Config\Database::getInstance();
		$mntents = $db->getByFilter("conf.system.filesystem.mountpoint", [
			"operator" => "not",
			"arg0" => [
				"operator" => "or",
				"arg0" => [
					"operator" => "stringContains",
					"arg0" => "opts",
					"arg1" => "bind"
				],
				"arg1" => [
					"operator" => "stringContains",
					"arg0" => "opts",
					"arg1" => "loop"
				]
			]
		]);
		foreach ($mntents as $mntentk => $mntentv) {
			// Get the file system backend.
			$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
			$fsb = $fsbMngr->getBackendByType($mntentv->get("type"));
			if (is_null($fsb)) {
				throw new \OMV\Exception(
				  "No file system backend exists for '%s'.",
				  $mntentv->get("type"));
			}
			// Is the file system detected and online?
			// Iterate over all detected file systems and check if the
			// configured one is in the list.
			foreach ($detectedFsObjects as $fsObjectk => $fsObjectv) {
				$fsNames = $fsObjectv['devlinks'];
				$fsNames[] = $fsObjectv['devicefile'];
				$fsNames[] = $fsObjectv['canonicaldevicefile'];
				if (TRUE === is_fs_uuid($fsObjectv['uuid'])) {
					$fsNames[] = $fsObjectv['uuid'];
				}
				if (in_array($mntentv->get("fsname"), $fsNames)) {
					$result[] = array_merge($fsObjectv, [
						"comment" => $mntentv->get("comment"),
						"usagewarnthreshold" => $mntentv->get("usagewarnthreshold"),
						"mountopts" => $mntentv->get("opts"),
						"status" => 1 // Online
					]);
					continue 2;
				}
			}
			// The configured file system is not in the list of detected
			// file systems. Append all available file system information
			// to the result list and mark it as 'Missing'.
			$result[] = array_merge($this->getFsInfo(NULL, $fsb), [
				"devicefile" => is_devicefile($mntentv->get("fsname")) ?
					$mntentv->get("fsname") : "",
				"devicefiles" => is_devicefile($mntentv->get("fsname")) ?
					[ $mntentv->get("fsname") ] : [],
				"uuid" => is_uuid($mntentv->get("fsname")) ?
					$mntentv->get("fsname") : "",
				"type" => $mntentv->get("type"),
				"mountpoint" => $mntentv->get("dir"),
				"comment" => $mntentv->get("comment"),
				"usagewarnthreshold" => $mntentv->get("usagewarnthreshold"),
				"mountopts" => $mntentv->get("opts"),
				"status" => 3, // Missing
				"_used" => $db->isReferenced($mntentv)
			]);
		}
		// Filter result.
		return $this->applyFilter($result, $params['start'],
			$params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Execute the getList() RPC as background process.
	 */
	public function getListBg($params, $context) {
		return $this->callMethodBg("getList", $params, $context);
	}

	/**
	 * Get list of devices that can be used to create a file system on.
	 * @param object $params The method parameters.
	 * @param object $context The context of the caller.
	 * @return array An array containing objects with the following fields:
	 *   devicefile, size and description.
	 * @throw \OMV\Exception
	 */
	public function getCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get a list of all potential usable devices.
		if (FALSE === ($devs = \OMV\System\Storage\StorageDevice::enumerateUnused()))
			throw new \OMV\Exception("Failed to get list of unused devices.");
		// Get a list of all detected file systems.
		$filesystems = \OMV\System\Filesystem\Filesystem::getFilesystems();
		// Get the list of device files that are occupied by a file system.
		$usedDevs = [];
		foreach ($filesystems as $filesystemk => $filesystemv) {
			$usedDevs[] = $filesystemv->getParentDeviceFile();
			// Check if the file system uses multiple devices, e.g.
			// a BTRFS RAID, and add them.
			if ($filesystemv->hasMultipleDevices()) {
				$usedDevs = array_merge($filesystemv->getDeviceFiles(),
					$usedDevs);
			}
		}
		// Prepare the result list.
		$result = [];
		foreach ($devs as $devk => $devv) {
			// Get the storage device object for the specified device file.
			$sd = \OMV\System\Storage\StorageDevice::getStorageDevice($devv);
			if (is_null($sd) || !$sd->exists())
				continue;
			// Skip read-only devices like CDROM.
			if (TRUE === $sd->isReadOnly())
				continue;
/* Do not check for references, otherwise a device file which is configured
   for S.M.A.R.T. monitoring is not added as a candidate.
			// Check if the device is referenced/used by a plugin.
			$db = \OMV\Config\Database::getInstance();
			if (TRUE === $db->exists("conf.service", [
				  "operator" => "stringContains",
				  "arg0" => "devicefile",
				  "arg1" => $sd->getDeviceFile()
			  ]))
				continue;
*/
			// Does this device already contain a file system?
			if (in_array($sd->getCanonicalDeviceFile(), $usedDevs))
				continue;
			// The device is a potential candidate to create a file system
			// on it.
			$result[] = [
				"devicefile" => $sd->getDeviceFile(),
				"size" => $sd->getSize(),
				"description" => $sd->getDescription()
			];
		}
		return $result;
	}

	/**
	 * Execute the getCandidates() RPC as background process.
	 */
	public function getCandidatesBg($params, $context) {
		return $this->callMethodBg("getCandidates", $params, $context);
	}

	/**
	 * Create a file system on the given storage device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file of the storage device on which
	 *     the file system is to be created.
	 *   \em type The file system to create, e.g. ext3 or xfs.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw \OMV\Exception
	 */
	public function create($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.filesystemmgmt.create");
		// Get the storage device object.
		$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
			$params['devicefile']);
		// Get the storage device backend of the given device.
		$sdbMngr = \OMV\System\Storage\Backend\Manager::getInstance();
		$sdbMngr->assertBackendExists($sd->getDeviceFile());
		$sdb = $sdbMngr->getBackend($sd->getDeviceFile());
		// Get the corresponding file system backend.
		$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
		$fsbMngr->assertBackendExistsByType($params['type']);
		$fsb = $fsbMngr->getBackendByType($params['type']);
		// Get the file system device file name from the storage device
		// backend (this may differ depending on the storage device).
		$fsDeviceFile = $sdb->fsDeviceFile($sd->getDeviceFile());
		// Create a file that contains the details of the file system being
		// initialized. The file is parsed by the 'FileSystemMgmt.getList'
		// RPC to display the state of the file system initialization
		// process. There is no other way to detect file systems being
		// initialized (blkid detects them after the initialization has
		// been finished).
		$fileName = sprintf("/tmp/omv-initfs@%s.build", str_replace(
			"/", "_", $sd->getDeviceFile()));
		$jsonFile = new \OMV\Json\File($fileName);
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params, $sd, $sdb, $fsb, $fsDeviceFile, $jsonFile) {
			// Create the file and write the file system information.
			$jsonFile->open("c");
			$jsonFile->write([
				"devicefile" => $fsDeviceFile,
				"parentdevicefile" => $sd->getDeviceFile(),
				"type" => $fsb->getType()
			]);
			$jsonFile->close();
			// Wipe all existing data on the storage device.
			$sd->wipe();
			// Create the partition if necessary.
			switch ($sdb->getType()) {
			case OMV_STORAGE_DEVICE_TYPE_SOFTWARERAID:
			case OMV_STORAGE_DEVICE_TYPE_DEVICEMAPPER:
			case OMV_STORAGE_DEVICE_TYPE_LOOPDEVICE:
				// No need to create a partition for those types.
				break;
			default:
				// Create a partition across the entire storage device.
				$cmdArgs = [];
				$cmdArgs[] = "--new=1:0:0";
				$cmdArgs[] = "--typecode=1:8300";
				$cmdArgs[] = "--print";
				$cmdArgs[] = escapeshellarg($sd->getDeviceFile());
				$cmd = new \OMV\System\Process("sgdisk", $cmdArgs);
				$cmd->setRedirect2to1();
				if (0 !== ($exitStatus = $this->exec($cmd, $output,
						$bgOutputFilename))) {
					throw new \OMV\ExecException($cmd, $output, $exitStatus);
				}
				break;
			}
			// Re-read the partition table.
			$cmdArgs = [];
			$cmdArgs[] = escapeshellarg($sd->getDeviceFile());
			$cmd = new \OMV\System\Process("partprobe", $cmdArgs);
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
			// We need to wait to give the kernel some time to re-read the
			// partition table and until the device file exists. Abort if
			// the device file does not exist after the specified time.
			$fsbd = new \OMV\System\BlockDevice($fsDeviceFile);
			$fsbd->waitForDevice(10);
			// Create the file system.
			$cmdArgs = [];
			$cmdArgs[] = "-V";
			$cmdArgs[] = sprintf("-t %s", $fsb->getType());
			$cmdArgs[] = $fsb->getMkfsOptions($sd);
			$cmdArgs[] = escapeshellarg($fsbd->getDeviceFile());
			$cmd = new \OMV\System\Process("mkfs", $cmdArgs);
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
			// Notify configuration changes.
			$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
			$dispatcher->notify(OMV_NOTIFY_CREATE,
				"org.openmediavault.conf.system.filesystem", [
					"devicefile" => $fsDeviceFile,
					"parentdevicefile" => $sd->getDeviceFile(),
					"type" => $fsb->getType()
				]);
			return $output;
		}, NULL, function() use($jsonFile) {
			// Cleanup
			$jsonFile->unlink();
		});
	}

	/**
	 * Create a Btrfs volume.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function createBtrfs($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
			"rpc.filesystemmgmt.createbtrfs");
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($params) {
		  	$deviceFiles = explode(",", $params['devicefiles']);
			// Wipe all existing data on the storage devices.
			foreach ($deviceFiles as $deviceFile) {
				$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
					$deviceFile);
				$sd->wipe();
			}
			// Create the Btrfs volume. We do not need to create an
			// partition on each associated device.
			// https://btrfs.readthedocs.io/en/latest/mkfs.btrfs.html#
			// https://btrfs.readthedocs.io/en/latest/mkfs.btrfs.html#profiles
			$dataProfile = $params['profile'];
			$metadataProfile = $params['profile'];
			if ("single" == $params['profile']) {
				$metadataProfile = "dup";
			}
			$cmdArgs = [];
			$cmdArgs[] = "--force";
			if (!empty($params['label'])) {
				$cmdArgs[] = "--label";
				$cmdArgs[] = escapeshellarg($params['label']);
			}
			$cmdArgs[] = "--data";
			$cmdArgs[] = escapeshellarg($dataProfile);
			$cmdArgs[] = "--metadata";
			$cmdArgs[] = escapeshellarg($metadataProfile);
			$cmdArgs = array_merge($cmdArgs, $deviceFiles);
			$cmd = new \OMV\System\Process("mkfs.btrfs", $cmdArgs);
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
		});
	}

	/**
	 * Grow a file system.
	 * @param params An array containing the following fields:
	 *   \em id The UUID or block special device of the file system to grow.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function grow($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.filesystemmgmt.grow");
		// Get the file system backend.
		$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
		$fsb = $fsbMngr->getBackendById($params['id']);
		if (is_null($fsb)) {
			throw new \OMV\Exception(
				"No file system backend exists for '%s'.",
				$params['id']);
		}
		// Check if the file system supports online resizing.
		if (!$fsb->hasResizeSupport()) {
			throw new \OMV\Exception(
				"The file system '%s' (type=%s) does not support online resizing.",
				$params['id'], $fsb->getType());
		}
		// Get the file system implementation.
		$fs = $fsb->getImpl($params['id']);
		if (is_null($fs) || !$fs->exists()) {
			throw new \OMV\Exception(
				"Failed to get the '%s' file system implementation or '%s' ".
				"does not exist.", $fsb->getType(), $params['id']);
		}
		// Grow the file system.
		$fs->grow();
	}

	/**
	 * Set a mount point configuration object.
	 * A new mount point database configuration object will be created.
	 * The file system will NOT be mounted, this is done by Salt when
	 * the configuration changes get applied. This way the file system
	 * can not be used before the system/configuration is in a proper
	 * state.
	 * @param params An array containing the following fields:
	 *   \em id The UUID or block special device of the file system to release.
	 *   \em usagewarnthreshold The threshold (in percent) where to start
	 *     warning when the capacity exceeds this given number.
	 *   \em comment Any text string. This field is optional.
	 * @param context The context of the caller.
	 * @return Returns the mount point configuration object.
	 */
	public function setMountPoint($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.filesystemmgmt.setMountPoint");
		// Check if there already exists a mount point for the given file
		// system.
		if (FALSE !== \OMV\Rpc\Rpc::call("FsTab", "getByFsName", [
			"fsname" => $params['id']
		], $context)) {
			throw new \OMV\Exception(
				"A mount point already exists for the file system '%s'.",
				$params['id']);
		}
		// Get the file system instance.
		$fs = \OMV\System\Filesystem\Filesystem::assertGetImpl(
			$params['id']);
		// Get the corresponding file system backend.
		$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
		$fsb = $fsbMngr->getBackendByType($fs->getType());
		// Get a predictable device file of the file system:
		// - /dev/disk/by-uuid/xxx
		// - /dev/disk/by-id/xxx
		// - /dev/xxx
		// Do not use the file system UUID itself, e.g.:
		// UUID=448f889a-105b-11e7-a91c-2b545744f57a
		// This is because this will make problems with LV/BTRFS snapshots
		// due the fact that they have the same file system UUID as their
		// origin.
		$fsName = $fs->getPredictableDeviceFile();
		// Optionally try to get the parent storage device on
		// which the file system is located. This device file
		// is used to get more specific mount options for the
		// file system.
		// Note, layered file systems do not provide that.
		$sd = NULL;
		$parentDeviceFile = $fs->getParentDeviceFile();
		if (is_devicefile($parentDeviceFile)) {
			$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
				$parentDeviceFile);
		}
		// Manage the mount point directory.
		$mp = \OMV\System\MountPoint::fromId($fsName);
		// Create a new mount point configuration object in the database.
		return \OMV\Rpc\Rpc::call("FsTab", "set", [
			"uuid" => \OMV\Environment::get("OMV_CONFIGOBJECT_NEW_UUID"),
			"fsname" => $fsName,
			"dir" => $mp->getPath(),
			"type" => $fs->getType(),
			"opts" => implode(",", $fsb->getFstabMntOptions($sd)),
			"freq" => 0,
			"passno" => 2,
			"usagewarnthreshold" => array_value($params,
				"usagewarnthreshold", 0),
			"comment" => array_value($params, "comment", "")
		], $context);
	}

	/**
	 * Unmount a file system.
	 * The file system will be unmounted and the mount point database
	 * configuration object will be deleted.
	 * @param params An array containing the following fields:
	 *   \em id The UUID or block special device of the file system to release.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function umount($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.filesystemmgmt.umount");
		// Try to obtain the mount point configuration object if this exists.
		$meObject = \OMV\Rpc\Rpc::call("FsTab", "getByFsName", [
			"fsname" => $params['id']
		], $context);
		if (FALSE === $meObject) {
			throw new \OMV\Exception(
				"No mount point database configuration object exists for the file system '%s'.",
				$params['id']);
		}
		// Get the file system and unmount it.
		$fs = \OMV\System\Filesystem\Filesystem::getImpl($params['id']);
		if (!is_null($fs) && $fs->exists()) {
			if (TRUE === $fs->isMounted()) {
				$fs->umount(TRUE);
			}
		}
		// Delete the mount point configuration object. Unmount the file
		// system and unlink the mount point. Changes to the fstab module
		// must not be applied immediately.
		\OMV\Rpc\Rpc::call("FsTab", "delete", [
			"uuid" => $meObject['uuid']
		], $context);
		\OMV\Rpc\Rpc::call("Config", "applyChanges", [
			"modules" => [ "fstab" ],
			"force" => TRUE
		], $context);
		// Finally unlink the mount point. Make sure not to unlink the
		// directory when the file system is still mounted.
		$mp = new \OMV\System\MountPoint($meObject['dir']);
		if (FALSE === $mp->isMountPoint()) {
			// Unlink if the mount point is empty (which should be the
			// normal case).
			$mp->unlink(FALSE, TRUE);
		}
	}

	/**
	 * Check if the given device containes a file system that is registered.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file to check.
	 * @param context The context of the caller.
	 * @return TRUE if a file system exists on the given device, otherwise
	 *   FALSE.
	 */
	public function hasFilesystem($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.filesystemmgmt.hasfilesystem");
		// Check if the given device file contains a file system.
		return (FALSE !== \OMV\System\Filesystem\Filesystem::hasFileSystem(
			$params['devicefile']));
	}

	/**
	 * Get details about the filesystem, e.g. the health status.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file to check.
	 * @param context The context of the caller.
	 * @return A string that contains the details about the given device.
	 */
	public function getDetails($params, $context) {
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$this->validateMethodParams($params, "rpc.filesystemmgmt.getdetails");
		$fs = \OMV\System\Filesystem\Filesystem::assertGetImpl(
			$params['devicefile']);
		return $fs->getDetails();
	}
}
