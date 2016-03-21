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
require_once("openmediavault/functions.inc");

class OMVRpcServiceDiskMgmt extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "DiskMgmt";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("enumerateDevices");
		$this->registerMethod("getList");
		$this->registerMethod("getHdParm");
		$this->registerMethod("setHdParm");
		$this->registerMethod("wipe");
		$this->registerMethod("rescan");
	}

	/**
	 * Enumerate all disk devices on the system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing physical disk device objects with the
	 *   fields \em devicename, \em devicefile, \em model, \em size,
	 *   \em description, \em vendor, \em serialnumber, \em israid,
	 *   and \em isroot.
	 * @throw \OMV\Exception
	 */
	public function enumerateDevices($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the device containing the operating system. Mark it as
		// read-only to deny wiping this device.
		$rootDeviceFile = \OMV\System\System::getRootDeviceFile();
		// Get all existing devices except software RAID devices.
		if (FALSE === ($devs = \OMV\System\Storage\StorageDevice::enumerate(
		  OMV_STORAGE_DEVICE_TYPE_HDD | OMV_STORAGE_DEVICE_TYPE_HBA))) {
			throw new \OMV\Exception(
			  "Failed to get list of hard disks and hardware RAID devices");
		}
		// Prepare result list.
		$objects = array();
		foreach ($devs as $devk => $devv) {
			$sd = \OMV\System\Storage\StorageDevice::getStorageDevice($devv);
			if (!is_null($sd) && $sd->exists()) {
				// Prepare device object.
				$objects[] = array(
					"devicename" => $sd->getDeviceName(),
					"devicefile" => $sd->getDeviceFile(),
					"devicefilebyid" => $sd->getDeviceFileById(),
					"model" => $sd->getModel(),
					"size" => $sd->getSize(),
					"description" => $sd->getDescription(),
					"vendor" => $sd->getVendor(),
					"serialnumber" => $sd->getSerialNumber(),
					"israid" => $sd->isRaid(),
					"isroot" => (0 === strpos($rootDeviceFile,
					  $sd->getDeviceFile())) ? TRUE : FALSE
				);
			}
		}
		return $objects;
	}

	/**
	 * Enumerate all disk devices on the system. The field \em hdparm will be
	 * added to the hard disk objects if there exists additional hard disk
	 * parameters (e.g. S.M.A.R.T. or AAM) that can be defined individually
	 * per hard disk.
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
		// Enumerate all disk devices on the system.
		$objects = $this->callMethod("enumerateDevices", NULL, $context);
		// Append additional configuration data.
		foreach ($objects as $objectk => &$objectv) {
			// Check if there exists a hdparm configuration object for the
			// given device. Note, we have to check for '/dev/xxx' and
			// '/dev/disk/by-id/xxx' entries.
			$db = \OMV\Config\Database::getInstance();
			$confObjects = $db->getByPredicate("conf.system.hdparm",
			  sprintf("devicefile='%s' or devicefile='%s']",
			  $objectv['devicefilebyid'], $objectv['devicefile']));
			if (1 <= count($confObjects)) {
				// Append the first found configuration values.
				$objectv['hdparm'] = $confObjects[0]->getAssoc();
			}
		}
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a hard disk parameters config object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function getHdParm($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.hdparm", $params['uuid']);
		// Return the values.
		return $object->getAssoc();
	}

	/**
	 * Set (add/update) a hard disk parameters config object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setHdParm($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.diskmgmt.sethdparm");
		// Create the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.system.hdparm");
		$object->setAssoc($params);
		// Check if the given device exists.
		\OMV\System\Storage\StorageDevice::assertGetStorageDevice(
		  $object->get("devicefile"));
		$sd = \OMV\System\Storage\StorageDevice::getStorageDevice(
		  $object->get("devicefile"));
		// Update the configuration data.
		$object->set("devicefile", $sd->getDeviceFileById());
		// Set the configuration object.
		$isNew = $object->isNew();
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $isNew) {
			// Check uniqueness.
			$db->assertIsUnique($object, "devicefile");
		}
		$db->set($object);
		// Notify the configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify($isNew ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.hdparm", $object->getAssoc());
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Wipe the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file to wipe, e.g. /dev/sdb.
	 *   \em secure Set to TRUE to secure wipe the device.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw \OMV\ExecException
	 */
	public function wipe($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.diskmgmt.wipe");
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
			// Wipe the device.
			if (TRUE === boolvalEx($params['secure'])) {
				// Secure wipe the device.
				$cmd = sprintf("export LANG=C; shred -v -n 1 %s 2>&1",
				  escapeshellarg($params['devicefile']));
				if (0 !== $this->exec($cmd, $output, $bgOutputFilename))
					throw new \OMV\ExecException($cmd, $output);
			} else {
				// Quick wipe the device (see omv-initfs).
				$cmd = sprintf("export LANG=C; sgdisk --zap-all ".
				  "%s 2>&1", escapeshellarg($params['devicefile']));
				if (0 !== $this->exec($cmd, $output, $bgOutputFilename))
					throw new \OMV\ExecException($cmd, $output);
				// To make sure really everything is wiped ...
				$cmd = sprintf("export LANG=C; dd if=/dev/zero of=%s bs=4M ".
				  "count=1 2>&1", escapeshellarg($params['devicefile']));
				if (0 !== $this->exec($cmd, $output, $bgOutputFilename))
					throw new \OMV\ExecException($cmd, $output);
			}
			// Reread partition table.
			$cmd = sprintf("export LANG=C; blockdev --rereadpt %s 2>&1",
			  escapeshellarg($params['devicefile']));
			if (0 !== $this->exec($cmd, $output, $bgOutputFilename))
				throw new \OMV\ExecException($cmd, $output);
			$this->finalizeBgProcStatus($bgStatusFilename, $output);
			exit(0);
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}

	/**
	 * Rescan SCSI bus.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function rescan($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Execute shell script to rescan SCSI bus.
		$script = new \OMV\System\ShellScript(
		  "[ -x /sys/class/scsi_host ] && for hostdir in ".
		  "\$(find /sys/class/scsi_host -iname \"host*\" -type l); ".
		  "do echo \"- - -\" | tee \${hostdir}/scan >/dev/null; done");
		$script->execute();
	}
}
