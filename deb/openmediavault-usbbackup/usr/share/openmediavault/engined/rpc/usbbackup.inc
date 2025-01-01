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
require_once("openmediavault/functions.inc");

class OMVRpcServiceUsbBackup extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "UsbBackup";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getCandidates");
		$this->registerMethod("getList");
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("delete");
		$this->registerMethod("execute");
	}

	/**
	 * Get list of filesystems that can be used for backup jobs.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing objects with the following fields:
	 *   The filesystem \em uuid, \em devicefile, \em label, \em type and
	 *   \em description.
	 */
	public function getCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get list of all detected filesystems. Note, the list may also
		// contain RAID and LVM volumes, so the list must be filtered.
		$fsbMngr = \OMV\System\Filesystem\Backend\Manager::getInstance();
		$objects = $fsbMngr->enumerate();
		// Collect volumes that are candidates for a backup job.
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			// Skip filesystem we do not support.
			if (!$fsbMngr->isSupported($objectv['type']))
				continue;
			// Skip the device where the operating system is installed on.
			if (\OMV\System\System::isRootDeviceFile($objectv['devicefile']))
				continue;
			// Get the filesystem backend.
			$fsb = $fsbMngr->getBackendByType($objectv['type']);
			// Skip filesystems that support union mounts, e.g. UnionFS,
			// aufs or mhddfs. Such filesystems do not have a device file
			// which is required to write the UDEV rule.
			if (FALSE === $fsb->hasDeviceFile())
				continue;
			// Get the filesystem implementation.
			$fs = $fsb->getImpl($objectv['devicefile']);
			// Get the parent storage device containing the filesystem.
			$parentDeviceFile = $fs->getParentDeviceFile();
			if (FALSE === is_devicefile($parentDeviceFile))
				continue;
			$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
				$parentDeviceFile);
			// Skip devices that contain multiple partitions/file systems.
			if (1 < count($sd->getPartitionDeviceNames())) {
				continue;
			}
			// Skip everything which is not an USB, eSATA, SAS, SD or eMMC
			// device or is a read-only medium.
			if (!($sd->isHotPluggable() || $sd->isAta() || $sd->isMmc()) ||
					$sd->isReadOnly()) {
				continue;
			}
			// Prepare result object.
			$result[] = [
				"uuid" => $objectv['uuid'],
				"devicefile" => $fs->getPredictableDeviceFile(),
				"label" => $objectv['label'],
				"type" => $objectv['type'],
				"description" => $sd->getDescription(),
				"propposixacl" => $fsb->hasPosixAclSupport()
			];
		}
		return $result;
	}

	/**
	 * Get list of USB backup job configuration objects.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
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
		// Get the configuration objects.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.usbbackup.job");
		// Add additional information.
		$objectsAssoc = [];
		foreach ($objects as $objectk => $objectv) {
			// Add new properties.
			$objectv->add("running", "boolean", FALSE);
			$objectv->add("sharedfoldername", "string", gettext("n/a"));
			// Get the shared folder configuration object.
			$sfObject = $db->get("conf.system.sharedfolder",
			  $objectv->get("sharedfolderref"));
			// Update the 'sharedfoldername' property.
			$objectv->set("sharedfoldername", $sfObject->get("name"));
			// Get job status.
			$statusFile = sprintf("/run/openmediavault-usbbackup-%s",
			  md5($objectv->get("devicefile")));
			$objectv->set("running", file_exists($statusFile));
			$objectsAssoc[] = $objectv->getAssoc();
		}
		// Filter the result.
		return $this->applyFilter($objectsAssoc, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a USB backup job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		return $db->getAssoc("conf.service.usbbackup.job", $params['uuid']);
	}

	/**
	 * Set a USB backup job configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function set($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.usbbackup.set");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.service.usbbackup.job");
		$object->setAssoc($params);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a backup job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function delete($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.usbbackup.job", $params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}

	/**
	 * Execute a backup job.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the cron job to execute.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw \OMV\Config\ConfigDirtyException
	 */
	public function execute($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Check if the module is marked as dirty. This is an indication
		// that the rsync cron script has not been created or updated
		// until now.
		if ($this->isModuleDirty("usbbackup"))
			throw new \OMV\Config\ConfigDirtyException();
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.usbbackup.job", $params['uuid']);
		// Get the device file of the destination storage device and
		// generate the mount point path. This is generated using the
		// MD5 sum of the device file.
		$deviceFile = $object->get("devicefile");
		$dir = \OMV\System\MountPoint::buildPath(md5($deviceFile));
		// Make sure the mount point exists.
		$mp = new \OMV\System\MountPoint($dir);
		$mp->create();
		// Get the file system and mount it if necessary. Note, the
		// file system is not listed in the fstab, so we need to
		// specify the mount point, too.
		if (FALSE === $mp->isMounted()) {
			\OMV\System\Filesystem\Filesystem::assertGetImpl($deviceFile);
			$fs = \OMV\System\Filesystem\Filesystem::getImpl($deviceFile);
			$fs->mount([], $dir);
		}
		// Create a background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($object, $mp) {
			// Execute the rsync script.
			$cmdArgs = [];
  			$cmdArgs[] = "--shell";
  			$cmdArgs[] = "--non-interactive";
  			$cmdArgs[] = "--";
  			$cmdArgs[] = build_path(DIRECTORY_SEPARATOR,
				\OMV\Environment::get("OMV_USBBACKUP_SCRIPTS_DIR",
					"/var/lib/openmediavault/usbbackup.d"),
				sprintf("%s%s", \OMV\Environment::get(
					"OMV_USBBACKUP_RSYNC_SCRIPT_PREFIX", "rsync-"),
					$object->getIdentifier()));
  			$cmd = new \OMV\System\Process("sudo", $cmdArgs);
  			$cmd->setEnv("SHELL", "/bin/sh");
  			$cmd->setRedirect2to1();
  			$cmdLine = $cmd->getCommandLine();
			// Execute the command.
			$exitStatus = $this->exec($cmdLine, $output, $bgOutputFilename);
			// Cleanup.
			$mp->umount();
			$mp->unlink();
			// Check the exit status.
			if (0 !== $exitStatus)
				throw new \OMV\ExecException($cmdLine, $output, $exitStatus);
			return $output;
		});
	}
}
