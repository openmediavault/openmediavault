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

class DiskMgmt extends \OMV\Rpc\ServiceAbstract {
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
		$this->registerMethod("getListBg");
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
	 *   fields \em devicename, \em canonicaldevicefile, \em devicefile,
	 *   \em devicelinks, \em model, \em size, \em description, \em vendor,
	 *   \em serialnumber, \em israid, \em isroot and \em isreadonly.
	 * @throw \OMV\Exception
	 */
	public function enumerateDevices($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the device containing the operating system. Mark it as
		// read-only to deny wiping this device.
		$rootDeviceFile = \OMV\System\System::getRootDeviceFile();
		// Get all existing devices except software RAID devices.
		if (FALSE === ($devs = \OMV\System\Storage\StorageDevice::enumerate(
		  OMV_STORAGE_DEVICE_TYPE_DISK))) {
			throw new \OMV\Exception(
			  "Failed to get list of hard disks and hardware RAID devices.");
		}
		// Prepare result list.
		$objects = [];
		foreach ($devs as $devk => $devv) {
			$sd = \OMV\System\Storage\StorageDevice::getStorageDevice($devv);
			if (is_null($sd) || !$sd->exists())
				continue;
			// Skip devices where no media is available, e.g. CDROM device
			// without an inserted media.
			if (FALSE === $sd->IsMediaAvailable())
				continue;
			$powerMode = "UNKNOWN";
			$temperature = FALSE;
			if (TRUE === $sd->hasSmartSupport()) {
				$si = $sd->getSmartInformation();
				$si->setNoCheck("standby");
				$powerMode = $si->getPowerMode();
				// Get the temperature if the SMART information is
				// available. Note, if the disk is in standby mode,
				// `smartctl` does not return any temperature value.
				if (TRUE === $si->isCached()) {
					$temperature = $si->getTemperature();
				}
			}
			$objects[] = [
				"devicename" => $sd->getDeviceName(),
				"canonicaldevicefile" => $sd->getCanonicalDeviceFile(),
				"devicefile" => $sd->getPredictableDeviceFile(),
				"devicelinks" => $sd->getDeviceFileSymlinks(),
				"model" => $sd->getModel(),
				"size" => $sd->getSize(),
				"description" => $sd->getDescription(),
				"vendor" => $sd->getVendor(),
				"serialnumber" => $sd->getSerialNumber(),
				"wwn" => $sd->getWorldWideName(),
				"israid" => $sd->isRaid(),
				"isroot" => \OMV\System\System::isRootDeviceFile(
					$sd->getCanonicalDeviceFile(), FALSE),
				"isreadonly" => $sd->isReadOnly(),
				"powermode" => $powerMode,
				"temperature" => (FALSE === $temperature) ?
					"" : $temperature,
				"hotpluggable" => $sd->isHotPluggable()
			];
		}
		return $objects;
	}

	/**
	 * Enumerate all disk devices on the system. The field \em hdparm will be
	 * added to the hard disk objects if there exists additional hard disk
	 * parameters (e.g. S.M.A.R.T. or AAM) that can be defined individually
	 * per hard disk.
	 * @param object $params An object containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param object $context The context of the caller.
	 * @return object An array containing the requested objects. The field
	 *   \em total contains the total number of objects, \em data contains
	 *   the object array. An exception will be thrown in case of an error.
	 */
	public function getList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
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
			$confObjects = $db->getByFilter("conf.system.hdparm", [
				"operator" => "stringEnum",
				"arg0" => "devicefile",
				"arg1" => array_merge([ $objectv['devicefile'] ],
					$objectv['devicelinks'])
			]);
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
	 * Execute the getList() RPC as background process.
	 */
	public function getListBg($params, $context) {
		return $this->callMethodBg("getList", $params, $context);
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.diskmgmt.sethdparm");
		// Try to get a /dev/disk/by-id device file.
		$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
			$params['devicefile']);
		if (TRUE === $sd->hasDeviceFileById())
			$params['devicefile'] = $sd->getDeviceFileById();
		// Create the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.system.hdparm");
		$object->setAssoc($params);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $object->isNew()) {
			// Check uniqueness.
			$db->assertIsUnique($object, "devicefile");
		}
		$db->set($object);
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.diskmgmt.wipe");
		// Get the device file and ensure it exists.
		$bd = new \OMV\System\BlockDevice($params['devicefile']);
		$bd->assertExists();
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($bd, $params) {
			// Wipe the device.
			if (TRUE === boolvalEx($params['secure'])) {
				// Secure wipe the device.
				$cmdArgs = [];
				$cmdArgs[] = "-v";
				$cmdArgs[] = "-n 1";
				$cmdArgs[] = escapeshellarg($bd->getDeviceFile());
				$cmd = new \OMV\System\Process("shred", $cmdArgs);
				$cmd->setRedirect2to1();
				if (0 !== ($exitStatus = $this->exec($cmd, $output,
						$bgOutputFilename))) {
					throw new \OMV\ExecException($cmd, $output, $exitStatus);
				}
			} else {
				// Quick wipe the device (see omv-initfs).
				$cmdArgs = [];
				$cmdArgs[] = "--zap-all";
				$cmdArgs[] = escapeshellarg($bd->getDeviceFile());
				$cmd = new \OMV\System\Process("sgdisk", $cmdArgs);
				$cmd->setRedirect2to1();
				if (0 !== ($exitStatus = $this->exec($cmd, $output,
						$bgOutputFilename))) {
					throw new \OMV\ExecException($cmd, $output, $exitStatus);
				}
				// To make sure really everything is wiped ...
				$cmdArgs = [];
				$cmdArgs[] = "if=/dev/zero";
				$cmdArgs[] = sprintf("of=%s", escapeshellarg(
					$bd->getDeviceFile()));
				$cmdArgs[] = "bs=10M";
				$cmdArgs[] = "count=1";
				$cmd = new \OMV\System\Process("dd", $cmdArgs);
				$cmd->setRedirect2to1();
				if (0 !== ($exitStatus = $this->exec($cmd, $output,
						$bgOutputFilename))) {
					throw new \OMV\ExecException($cmd, $output, $exitStatus);
				}
				// Erase the metadata/signatures at the end of the device.
				// Note, we still need to support 32bit systems, so use
				// BC math functions.
				$sectorSize = $bd->getSectorSize();
				$sizeInSectors = bcdiv($bd->getSize(), strval($sectorSize), 0);
				$count = bcdiv("10485760", strval($sectorSize), 0); // 10MiB
				$seek = bcsub($sizeInSectors, $count, 0);
				$cmdArgs = [];
				$cmdArgs[] = "if=/dev/zero";
				$cmdArgs[] = sprintf("of=%s", escapeshellarg(
					$bd->getDeviceFile()));
				$cmdArgs[] = sprintf("bs=%d", $sectorSize);
				$cmdArgs[] = sprintf("count=%s", $count);
				$cmdArgs[] = sprintf("seek=%s", $seek);
				$cmd = new \OMV\System\Process("dd", $cmdArgs);
				$cmd->setRedirect2to1();
				if (0 !== ($exitStatus = $this->exec($cmd, $output,
						$bgOutputFilename))) {
					throw new \OMV\ExecException($cmd, $output, $exitStatus);
				}
			}
			// Reread partition table.
			$cmdArgs = [];
			$cmdArgs[] = "--rereadpt";
			$cmdArgs[] = escapeshellarg($bd->getDeviceFile());
			$cmd = new \OMV\System\Process("blockdev", $cmdArgs);
			$cmd->setRedirect2to1();
			if (0 !== ($exitStatus = $this->exec($cmd, $output,
					$bgOutputFilename))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
			return $output;
		});
	}

	/**
	 * Rescan SCSI bus.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function rescan($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Execute shell script to rescan SCSI bus.
		$script = new \OMV\System\ShellScript(
		  "[ -x /sys/class/scsi_host ] && for hostdir in ".
		  "\$(find /sys/class/scsi_host -iname \"host*\" -type l); ".
		  "do echo \"- - -\" > \${hostdir}/scan; done");
		$script->execute();
		// Execute shell script to detect SCSI device size changes.
		$script = new \OMV\System\ShellScript(
		  "[ -x /sys/class/scsi_device ] && for devicedir in ".
		  "\$(find /sys/class/scsi_device -iname \"*:*\" -type l); ".
		  "do echo 1 > \${devicedir}/device/rescan; done");
		$script->execute();
	}
}
