<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
	 * @throw \OMV\Exception
	 */
	public function enumerateFilesystems($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get list of all detected filesystems.
		$filesystems = \OMV\System\Filesystem\Filesystem::getFilesystems();
		// Process the detected filesystems and skip unwanted ones.
		$procs = [];
		foreach ($filesystems as $fs) {
			// Collect the filesystem information asynchronous.
			$procs[] = $this->asyncProc(function() use ($fs) {
				// Get the filesystem backend.
				$fsb = $fs->getBackend();
				if (is_null($fsb)) {
					throw new \OMV\Exception(
						"No filesystem backend set for '%s'.",
						$fs->getDeviceFile());
				}
				// Set default values.
				$object = [
					"devicefile" => $fs->getPreferredDeviceFile(),
					"parentdevicefile" => $fs->getParentDeviceFile(),
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
					"description" => $fs->hasLabel() ?
						$fs->getLabel() : $fs->getDeviceFile(),
					"propposixacl" => $fsb->hasPosixAclSupport(),
					"propquota" => $fsb->hasQuotaSupport(),
					"propresize" => $fsb->hasResizeSupport(),
					"propfstab" => $fsb->hasFstabSupport(),
					"propreadonly" => $fsb->hasReadOnlySupport(),
					"propcompress" => $fsb->hasCompressSupport(),
					"propautodefrag" => $fsb->hasAutoDefragSupport(),
					"hasmultipledevices" => $fs->hasMultipleDevices(),
					"devicefiles" => $fs->getDeviceFiles(),
					"_readonly" => $fsb->hasReadOnlySupport(),
					"_used" => FALSE
				];
				// Check if the filesystem is used. First try to get the
				// corresponding mount point configuration object. Query the
				// database for all device files that exist for the given
				// filesystem to increase the probability to find the mount
				// point configuration object. If such object exist, then
				// check if it is referenced by any other object, e.g. by a
				// shared folder configuration object.
				$fsNames = $fs->getDeviceFileSymlinks();
				if (TRUE === $fs->hasUuid()) {
					$fsNames[] = $fs->getUuid();
				}
				$filter = [
					"operator" => "stringEquals",
					"arg0" => "fsname",
					"arg1" => array_shift($fsNames)
				];
				foreach ($fsNames as $fsNamek => $fsNamev) {
					$filter = [
						"operator" => "or",
						"arg0" => $filter,
						"arg1" => [
							"operator" => "stringEquals",
							"arg0" => "fsname",
							"arg1" => $fsNamev
						]
					];
				}
				$db = \OMV\Config\Database::getInstance();
				$mpObjects = $db->getByFilter("conf.system.filesystem.mountpoint",
					$filter);
				if (!empty($mpObjects)) {
					$object['_used'] = $db->isReferenced($mpObjects[0]);
				}
				// Mark the device where the operating system is installed on
				// as used and read-only.
				if (\OMV\System\System::isRootDeviceFile($object['devicefile'])) {
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
				return $object;
			});
		}
		return $this->waitAsyncProcs($procs);
	}

	/**
	 * Enumerate all filesystems that have a mount point configuration
	 * object, except binds, and that are actually mounted.
	 * @param params The method parameters.
	 *   \em includeroot TRUE to append the filesystem '/dev/root' if mounted.
	 *     Defaults to FALSE.
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
		// Append '/dev/root'?
		if (!is_null($params) && (TRUE === array_boolval($params,
				"includeroot", FALSE))) {
			$rootObject = new \OMV\Config\ConfigObject(
				"conf.system.filesystem.mountpoint");
			$rootObject->set("fsname", \OMV\System\System::getRootDeviceFile());
			$rootObject->set("dir", "/");
			array_unshift($objects, $rootObject);
		}
		// Get the file system details for each mount point.
		$procs = [];
		foreach ($objects as $objectk => $objectv) {
			// Collect the filesystem information asynchronous.
			$procs[] = $this->asyncProc(function() use ($objectv) {
				// Get the filesystem backend.
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
				// Get the filesystem details.
				$object = [
					"devicefile" => $fs->getPreferredDeviceFile(),
					"parentdevicefile" => $fs->getParentDeviceFile(),
					"uuid" => $fs->getUuid(),
					"label" => $fs->getLabel(),
					"type" => $fs->getType(),
					"blocks" => "-1", // as string
					"mountpoint" => $objectv->get("dir"),
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
					"propcompress" => $fsb->hasCompressSupport(),
					"propautodefrag" => $fsb->hasAutoDefragSupport(),
					"hasmultipledevices" => $fs->hasMultipleDevices(),
					"devicefiles" => $fs->getDeviceFiles()
				];
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
				return $object;
			});
		}
		// Remove elements that are no associative arrays (e.g. these are
		// filesystems that are not mounted).
		return array_filter($this->waitAsyncProcs($procs), "is_assoc_array");
	}

	/**
	 * Get the list of filesystems that have been detected.
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
		// Enumerate all detected filesystems.
		$objects = $this->callMethod("enumerateFilesystems", NULL, $context);
		foreach ($objects as $objectk => &$objectv) {
			// Mark each filesystem as 'Online' by default.
			$objectv['status'] = 1; // Online
		}
		// Try to detect filesystems that are being initialized.
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
				if (TRUE === $jsonFile->exists())
					$jsonFile->unlink();
				continue;
			}
			// Get the filesystem backend.
			$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
			$fsb = $fsbMngr->getBackendByType($fsInfo['type']);
			if (is_null($fsb)) {
				throw new \OMV\Exception(
				  "No filesystem backend exists for '%s'.",
				  $fsInfo['type']);
			}
			// Add the filesystem to the result list.
			$objects[] = [
				"devicefile" => $fsInfo['devicefile'],
				"parentdevicefile" => $fsInfo['parentdevicefile'],
				"devicefiles" => [ $fsInfo['devicefile'] ],
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
				"propcompress" => $fsb->hasCompressSupport(),
				"propautodefrag" => $fsb->hasAutoDefragSupport(),
				"hasmultipledevices" => false,
				"status" => 2, // Initializing
				"_used" => FALSE
			];
		}
		// Add filesystems configured to be mounted but device does not
		// exist anymore. This is necessary to be able remove invalid mount
		// point configuration objects. Mark such filesystem as missing.
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
			// Get the filesystem backend.
			$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
			$fsb = $fsbMngr->getBackendByType($mntentv->get("type"));
			if (is_null($fsb)) {
				throw new \OMV\Exception(
				  "No filesystem backend exists for '%s'.",
				  $mntentv->get("type"));
			}
			// Skip valid mount point configuration objects (the filesystem
			// exists in this case).
			$fs = $fsb->getImpl($mntentv->get("fsname"));
			if (!is_null($fs) && $fs->exists())
				continue;
			// Append as much information as possible.
			$objects[] = [
				"devicefile" => is_devicefile($mntentv->get("fsname")) ?
					$mntentv->get("fsname") : "",
				"devicefiles" => is_devicefile($mntentv->get("fsname")) ?
					[ $mntentv->get("fsname") ] : [],
				"uuid" => is_uuid($mntentv->get("fsname")) ?
					$mntentv->get("fsname") : "",
				"label" => "",
				"type" => $mntentv->get("type"),
				"blocks" => "-1", // as string
				"mounted" => FALSE,
				"mountable" => TRUE,
				"mountpoint" => $mntentv->get("dir"),
				"used" => "-1", // as string
				"available" => "-1", // as string
				"size" => "-1", // as string
				"percentage" => -1,
				"description" => "",
				"propposixacl" => $fsb->hasPosixAclSupport(),
				"propquota" => $fsb->hasQuotaSupport(),
				"propresize" => $fsb->hasResizeSupport(),
				"propfstab" => $fsb->hasFstabSupport(),
				"propcompress" => $fsb->hasCompressSupport(),
				"propautodefrag" => $fsb->hasAutoDefragSupport(),
				"hasMultipleDevices" => FALSE,
				"status" => 3, // Missing
				"_used" => $db->isReferenced($mntentv)
			];
		}
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Execute the getList() RPC as background process.
	 */
	public function getListBg($params, $context) {
		return $this->callMethodBg("getList", $params, $context);
	}

	/**
	 * Get list of devices that can be used to create a filesystem on.
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
		// Get a list of all detected filesystems.
		$filesystems = \OMV\System\Filesystem\Filesystem::getFilesystems();
		// Get the list of device files that are occupied by a filesystem.
		$usedDevs = [];
		foreach ($filesystems as $filesystemk => $filesystemv) {
			$usedDevs[] = $filesystemv->getParentDeviceFile();
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
			// Does this device already contain a filesystem?
			if (in_array($sd->getCanonicalDeviceFile(), $usedDevs))
				continue;
			// The device is a potential candidate to create a filesystem
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
	 * Create a filesystem on the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The block special device file.
	 *   \em type The filesystem to create, e.g. ext3 or xfs.
	 *   \em label The label of the filesystem.
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
		// Check uniqueness. If there exists a mount point for the given
		// device then it has already a filesystem that is in use.
		if (FALSE !== \OMV\Rpc\Rpc::call("FsTab", "getByFsName", [
			"fsname" => $sd->getDeviceFile()
		], $context)) {
			throw new \OMV\Exception("A mount point already exists for '%s'.",
			  $sd->getDeviceFile());
		}
		// Is the filesystem label unique?
		if (FALSE === empty($params['label'])) {
			$filesystems = \OMV\System\Filesystem\Filesystem::getFilesystems();
			foreach ($filesystems as $filesystemk => $filesystemv) {
				if (FALSE === $filesystemv->hasLabel())
				 	continue;
				if ($filesystemv->getLabel() === $params['label']) {
					throw new \OMV\Exception(
						"The filesystem label '%s' already exists.",
						$params['label']);
				}
			}
		}
		// Get the storage device backend of the given device.
		$sdbMngr = \OMV\System\Storage\Backend\Manager::getInstance();
		$sdbMngr->assertBackendExists($sd->getDeviceFile());
		$sdb = $sdbMngr->getBackend($sd->getDeviceFile());
		// Get the corresponding filesystem backend.
		$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
		$fsbMngr->assertBackendExistsByType($params['type']);
		$fsb = $fsbMngr->getBackendByType($params['type']);
		// Get the filesystem device file name from the storage device
		// backend (this may differ depending on the storage device).
		$fsDeviceFile = $sdb->fsDeviceFile($sd->getDeviceFile());
		// Create a file that contains the details of the filesystem being
		// initialized. The file is parsed by the 'FileSystemMgmt.getList'
		// RPC to display the state of the filesystem initialization
		// process. There is no other way to detect filesystems being
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
				"type" => $fsb->getType(),
				"label" => $params['label']
			]);
			$jsonFile->close();
			// Wipe all existing data on the storage device.
			$this->writeBgProcOutput($bgOutputFilename,
				"===== Wipe signatures from device =====\n");
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
				$this->writeBgProcOutput($bgOutputFilename,
					"===== Create partition =====\n");
				$cmdArgs = [];
				$cmdArgs[] = "--new=1:0:0";
				$cmdArgs[] = "--typecode=1:8300";
				$cmdArgs[] = "--print";
				$cmdArgs[] = escapeshellarg($sd->getDeviceFile());
				$cmd = new \OMV\System\Process("sgdisk", $cmdArgs);
				$cmd->setRedirect2to1();
				if (0 !== ($exitStatus = $this->exec($cmd->getCommandLine(),
					$output, $bgOutputFilename))) {
					throw new \OMV\ExecException($cmd->getCommandLine(),
						$output, $exitStatus);
				}
				break;
			}
			// Re-read the partition table.
			$cmdArgs = [];
			$cmdArgs[] = escapeshellarg($sd->getDeviceFile());
			$cmd = new \OMV\System\Process("partprobe", $cmdArgs);
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd->getCommandLine(),
				$output, $bgOutputFilename))) {
				throw new \OMV\ExecException($cmd->getCommandLine(),
					$output, $exitStatus);
			}
			// We need to wait to give the kernel some time to re-read the
			// partition table and until the device file exists. Abort if
			// the device file does not exist after the specified time.
			$fsbd = new \OMV\System\BlockDevice($fsDeviceFile);
			$fsbd->waitForDevice(10);
			// Create the file system.
			$this->writeBgProcOutput($bgOutputFilename,
				"===== Create file system =====\n");
			$cmdArgs = [];
			$cmdArgs[] = "-V";
			$cmdArgs[] = sprintf("-t %s", $fsb->getType());
			$cmdArgs[] = $fsb->getMkfsOptions($sd);
			if (!empty($params['label'])) {
				$cmdArgs[] = sprintf("-L %s", escapeshellarg(
				  $params['label']));
			}
			$cmdArgs[] = escapeshellarg($fsbd->getDeviceFile());
			$cmd = new \OMV\System\Process("mkfs", $cmdArgs);
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd->getCommandLine(),
				$output, $bgOutputFilename))) {
				throw new \OMV\ExecException($cmd->getCommandLine(), $output,
					$exitStatus);
			}
			// Notify configuration changes.
			$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
			$dispatcher->notify(OMV_NOTIFY_CREATE,
				"org.openmediavault.conf.system.filesystem", [
					"devicefile" => $fsDeviceFile,
					"parentdevicefile" => $sd->getDeviceFile(),
					"type" => $fsb->getType(),
					"label" => $params['label']
				]);
			return $output;
		}, NULL, function() use($jsonFile) {
			// Cleanup
			$jsonFile->unlink();
		});
	}

	/**
	 * Resize a filesystem.
	 * @param params An array containing the following fields:
	 *   \em id The UUID or block special device of the filesystem to resize.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function resize($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.filesystemmgmt.resize");
		// Get the filesystem backend.
		$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
		$fsb = $fsbMngr->getBackendById($params['id']);
		if (is_null($fsb)) {
			throw new \OMV\Exception("No filesystem backend exists for '%s'.",
			  $params['id']);
		}
		// Check if the filesystem supports online resizing.
		if (!$fsb->hasResizeSupport()) {
			throw new \OMV\Exception(
			  "The filesystem '%s' (type=%s) does not support online resizing.",
			  $params['id'], $fsb->getType());
		}
		// Get the filesystem implementation.
		$fs = $fsb->getImpl($params['id']);
		if (is_null($fs) || !$fs->exists()) {
			throw new \OMV\Exception(
			  "Failed to get the '%s' filesystem implementation or '%s' ".
			  "does not exist.", $fsb->getType(), $params['id']);
		}
		// Grow the filesystem.
		$fs->grow();
	}

	/**
	 * Delete a filesystem. The filesystem will be unmounted and deleted.
	 * @param params An array containing the following fields:
	 *   \em id The UUID or block special device of the filesystem to delete.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function delete($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.filesystemmgmt.delete");

		// !!! Note !!!
		// If the file system is missing some of the following code paths
		// are ignored. In this case only the configuration is modified.

		// Get the file system if available.
		$fs = \OMV\System\Filesystem\Filesystem::getImpl($params['id']);
		// Get the mount point configuration object that belongs to
		// the file system to be deleted.
		$filter = [
			"operator" => "stringEquals",
			"arg0" => "fsname",
			"arg1" => $params['id']
		];
		// If the file system exists, then append additional filters
		// to be sure to find a mount point configuration object.
		if (!is_null($fs) && $fs->exists()) {
			$devs = $fs->getDeviceFileSymlinks();
			foreach ($devs as $devk => $devv) {
				$filter = [
					"operator" => "or",
					"arg0" => $filter,
					"arg1" => [
						"operator" => "stringEquals",
						"arg0" => "fsname",
						"arg1" => $devv
					]
				];
			}
		}
		try {
			$db = \OMV\Config\Database::getInstance();
			$meObject = $db->getByFilter("conf.system.filesystem.mountpoint",
				$filter, 1);
		} catch(\Exception $e) {
			$meObject = NULL;
		}
		// Initialize the default notification object.
		$fsObject = [
			"devicefile" => is_devicefile($params['id']) ? $params['id'] : "",
			"uuid" => is_uuid($params['id']) ? $params['id'] : "",
			"label" => "",
			"type" => ""
		];
		// Update the notification object.
		if (!is_null($fs) && $fs->exists()) {
			$fsObject = [
				"devicefile" => $fs->getDeviceFile(),
				"uuid" => $fs->getUuid(),
				"label" => $fs->getLabel(),
				"type" => $fs->getType()
			];
		} else if (!is_null($meObject)) {
			$fsObject['type'] = $meObject->get("type");
		}
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE,
			"org.openmediavault.conf.system.filesystem", $fsObject);
		// Delete the associated fstab mount point entry.
		if (!is_null($meObject)) {
			// Delete the mount point configuration object. Unmount the
			// filesystem and unlink the mount point. Changes to the fstab
			// module must not be applied immediately.
			\OMV\Rpc\Rpc::call("FsTab", "delete", [
				"uuid" => $meObject->get("uuid")
			], $context);
			\OMV\Rpc\Rpc::call("Config", "applyChanges", [
				"modules" => [ "fstab" ],
				"force" => TRUE
			], $context);
		}
		// Finally erase the filesystem.
		if (!is_null($fs) && $fs->exists())
			$fs->remove();
		// Notify configuration changes.
		$dispatcher->notify(OMV_NOTIFY_DELETE,
			"org.openmediavault.conf.system.filesystem", $fsObject);
		// Return the configuration object.
		return $fsObject;
	}

	/**
	 * Mount a filesystem.
	 * @param params An array containing the following fields:
	 *   \em id The UUID or block special device of the filesystem to mount.
	 *   \em fstab If set to FALSE, no fstab entry will be created, thus the
	 *     given filesystem is mounted only.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\Exception
	 * @throw \OMV\Config\ConfigDirtyException
	 */
	public function mount($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.filesystemmgmt.mount");
		// Get the corresponding filesystem backend.
		$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
		$fsbMngr->assertBackendExistsById($params['id']);
		$fsb = $fsbMngr->getBackendById($params['id']);
		// Get the filesystem.
		$fs = $fsb->getImpl($params['id']);
		if (is_null($fs) || !$fs->exists()) {
			throw new \OMV\Exception(
				"Failed to get the '%s' filesystem implementation or '%s' ".
				"does not exist.", $fsb->getType(), $params['id']);
		}
		// Get a predictable device file of the filesystem:
		// - /dev/disk/by-label/xxx
		// - /dev/disk/by-id/xxx
		// - /dev/xxx
		// Do not use the filesystem UUID itself, e.g.:
		// UUID=448f889a-105b-11e7-a91c-2b545744f57a
		// This is because this will make problems with LV/BTRFS snapshots
		// due the fact that they have the same filesystem UUID as their
		// origin.
		$fsName = $fs->getPredictableDeviceFile();
		// Try to obtain the mount point configuration object if this exists.
		$meObject = \OMV\Rpc\Rpc::call("FsTab", "getByFsName", [
			"fsname" => $fsName
		], $context);
		// Create fstab entry?
		if (TRUE === boolvalEx($params['fstab'])) {
			// Check for duplicates. Create a new mount point configuration
			// object if necessary.
			if (FALSE === $meObject) {
				// Do not allow filesystems which labels contain blanks
				// because SaltStack has a bug and can't handle such
				// filesystems.
				// See https://github.com/saltstack/salt/issues/54508
				if ($fs->hasLabel() && (1 == preg_match("/ /",
						$fs->getLabel()))) {
					throw new \OMV\Exception(
						"The filesystem label contains blanks.");
				}
				// Optionally try to get the parent storage device on
				// which the filesystem is located. This device file
				// is used to get more specific mount options for the
				// filesystem.
				// Note, layered filesystems do not provide that.
				$sd = NULL;
				$parentDeviceFile = $fs->getParentDeviceFile();
				if (is_devicefile($parentDeviceFile)) {
					$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
						$parentDeviceFile);
				}
				// Create a new mount configuration object in the database.
				\OMV\Rpc\Rpc::call("FsTab", "set", [
					"uuid" => \OMV\Environment::get(
						"OMV_CONFIGOBJECT_NEW_UUID"),
					"fsname" => $fsName,
					"dir" => \OMV\System\MountPoint::buildPath($fsName),
					"type" => $fs->getType(),
					"opts" => implode(",", $fsb->getFstabMntOptions($sd)),
					"freq" => 0,
					"passno" => 2
				], $context);
				// Apply the changes to the '/etc/fstab' file immediately
				// to mount the filesystem.
				\OMV\Rpc\Rpc::call("Config", "applyChanges", [
					"modules" => [ "fstab" ],
					"force" => TRUE
				], $context);
			} else {
				// Check if the configuration is marked as dirty, otherwise
				// the /etc/fstab file is not up-to-date and the mount fails
				// because of missing entries.
				if ($this->isModuleDirty("fstab")) {
					throw new \OMV\Config\ConfigDirtyException();
				}
				// Umount the mount point if it is already in use, which
				// is the case when an USB device is unplugged without
				// unmounting it.
				if (TRUE === $fs->isMounted()) {
					$fs->umount();
				}
				// Does the mount directory exist? Re-create it if
				// necessary.
				$mp = new \OMV\System\MountPoint($meObject['dir']);
				$mp->create();
				// Finally mount the filesystem.
				$fs->mount();
			}
		} else {
			// Try to create/re-create the mount directory. This is only
			// possible if a appropriate mount point configuration object
			// exists.
			if ((FALSE !== $meObject) && is_object($meObject)) {
				$mp = new \OMV\System\MountPoint($meObject['dir']);
				$mp->create();
			}
			// Mount the filesystem.
			if (TRUE === $fs->isMounted()) {
				throw new \OMV\Exception(
					"The filesystem '%s' is already mounted.",
					$fsName);
			}
			$fs->mount();
		}
	}

	/**
	 * Unmount a filesystem.
	 * @param params An array containing the following fields:
	 *   \em id The UUID or block special device of the filesystem to unmount.
	 *   \em fstab If set to FALSE, the fstab entry will not be removed (if
	 *     existing), thus the given filesystem is unmounted only.
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
		// Get the file system.
		$fs = \OMV\System\Filesystem\Filesystem::getImpl($params['id']);
		// Remove fstab entry?
		if (TRUE === boolvalEx($params['fstab'])) {
			// Get the fstab mount point configuration object.
			$filter = [
				"operator" => "stringEquals",
				"arg0" => "fsname",
				"arg1" => $params['id']
			];
			// If the file system exists, then append additional filters
			// to be sure to find a mount point configuration object.
			if (!is_null($fs) && $fs->exists()) {
				// Add all existing filesystem device files.
				$devs = $fs->getDeviceFileSymlinks();
				foreach ($devs as $devk => $devv) {
					$filter = [
						"operator" => "or",
						"arg0" => $filter,
						"arg1" => [
							"operator" => "stringEquals",
							"arg0" => "fsname",
							"arg1" => $devv
						]
					];
				}
				// To keep backward compatibility we need to search for the
				// filesystem UUID, too.
				if (TRUE === $fs->hasUuid()) {
					$filter = [
						"operator" => "or",
						"arg0" => $filter,
						"arg1" => [
							"operator" => "stringEquals",
							"arg0" => "fsname",
							"arg1" => $fs->getUuid()
						]
					];
				}
			}
			$db = \OMV\Config\Database::getInstance();
			$meObjects = $db->getByFilter("conf.system.filesystem.mountpoint",
				$filter);
			// If there is a mount point configuration object for this file
			// system then delete it, otherwise unmount the file system only.
			if (!empty($meObjects)) {
				$meObject = $meObjects[0];
				// Delete the mount point configuration object. Unmount the
				// filesystem and unlink the mount point. Changes to the fstab
				// module must not be applied immediately.
				\OMV\Rpc\Rpc::call("FsTab", "delete", [
					"uuid" => $meObject->get("uuid")
				], $context);
				\OMV\Rpc\Rpc::call("Config", "applyChanges", [
					"modules" => [ "fstab" ],
					"force" => TRUE
				], $context);
				// Exit here, everything is done.
				return;
			} else {
				// This file systems seems to be not under our control because
				// of the missing 'conf.system.filesystem.mountpoint'
				// configuration object. In this case unmount the file system
				// if it exists, otherwise NO further action is necessary.
			}
		} else { // Only unmount the filesystem.
			// The file system MUST exist here, otherwise throw an error.
			\OMV\System\Filesystem\Filesystem::assertGetImpl($params['id']);
		}
		// Finally unmount the specified file system if it exists.
		if (!is_null($fs) && $fs->exists()) {
			if (TRUE === $fs->isMounted())
				$fs->umount(TRUE);
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
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.filesystemmgmt.hasfilesystem");
		// Check if the given device file contains a filesystem.
		return (FALSE !== \OMV\System\Filesystem\Filesystem::hasFileSystem(
			$params['devicefile']));
	}
}
