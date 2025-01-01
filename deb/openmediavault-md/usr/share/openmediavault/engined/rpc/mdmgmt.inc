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

class MdMgmt extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "MdMgmt";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("enumerateDevices");
		$this->registerMethod("getList");
		$this->registerMethod("get");
		$this->registerMethod("getSlaves");
		$this->registerMethod("getCandidates");
		$this->registerMethod("create");
		$this->registerMethod("grow");
		$this->registerMethod("add");
		$this->registerMethod("remove");
		$this->registerMethod("delete");
		$this->registerMethod("getDetail");
	}

	/**
	 * Enumerate all RAID devices on the system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing RAID device objects with following
	 *   fields: name, devicefile, uuid, level, numdevices, devices,
	 *   size, state, description and _used.
	 */
	public function enumerateDevices($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get all existing software RAID devices.
		if (FALSE === ($devs = \OMV\System\Storage\StorageDevice::enumerate(
		  OMV_STORAGE_DEVICE_TYPE_SOFTWARERAID))) {
			throw new \OMV\Exception(
			  "Failed to get list of software RAID devices.");
		}
		// Prepare the result object.
		$result = [];
		foreach ($devs as $devk => $devv) {
			$raid = new \OMV\System\Storage\StorageDeviceMd($devv);
			if (!$raid->exists())
				continue;
			$used = FALSE;
			// Is this RAID device used by another storage device?
			if (\OMV\System\Storage\StorageDevice::isUsed(
			  $raid->getDeviceFile())) {
				$used = TRUE;
			// Does the RAID device contain a filesystem and is it used?
			} else if (FALSE !== \OMV\Rpc\Rpc::call("FsTab", "getByFsName", [
				"fsname" => $raid->getDeviceFile()
			], $context)) {
				$used = TRUE;
			}
			$result[] = [
				"_used" => $used,
				"name" => $raid->getName(),
				"devicefile" => $raid->getDeviceFile(),
				"uuid" => $raid->getUuid(),
				"level" => $raid->getLevel(),
				"numdevices" => $raid->getNumDevices(),
				"devices" => $raid->getSlaves(),
				"size" => $raid->getSize(),
				"state" => $raid->getState(),
				"description" => $raid->getDescription()
			];
		}
		return $result;
	}

	/**
	 * Get list of available RAID devices.
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
		// Enumerate all RAID devices on the system.
		$devs = $this->callMethod("enumerateDevices", NULL, $context);
		// Filter the result list.
		return $this->applyFilter($devs, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get details about the given RAID device.
	 * @param params The method parameters.
	 *   \em devicefile The RAID device file, e.g. /dev/md0.
	 * @param context The context of the caller.
	 * @return An object containing objects with the following fields:
	 *   name, devicefile, uuid, level, numdevices, devices, size,
	 *   state, description and _used.
	 */
	public function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the storage device.
		$raid = new \OMV\System\Storage\StorageDeviceMd(
			$params['devicefile']);
		$raid->assertExists();
		return [
			"_used" => $used,
			"name" => $raid->getName(),
			"canonicaldevicefile" => $raid->getCanonicalDeviceFile(),
			"devicefile" => $raid->getPredictableDeviceFile(),
			"uuid" => $raid->getUuid(),
			"level" => $raid->getLevel(),
			"numdevices" => $raid->getNumDevices(),
			"devices" => $raid->getSlaves(),
			"size" => $raid->getSize(),
			"state" => $raid->getState(),
			"description" => $raid->getDescription()
		];
	}

	/**
	 * Get list of slave/component devices from the given RAID device.
	 * @param params The method parameters.
	 *   \em devicefile The RAID device file, e.g. /dev/md0.
	 * @param context The context of the caller.
	 * @return An array containing objects with the following fields:
	 *   devicefile, size, vendor, serialnumber and description.
	 */
	public function getSlaves($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get MD the storage device.
		$raid = new \OMV\System\Storage\StorageDeviceMd(
		  $params['devicefile']);
		$raid->assertExists();
		// Get the slave/component devices of the given RAID.
		$devs = $raid->getSlaves();
		// Prepare the result list.
		$result = [];
		foreach ($devs as $devk => $devv) {
			$sd = \OMV\System\Storage\StorageDevice::getStorageDevice($devv);
			if (is_null($sd) || !$sd->exists())
				continue;
			$result[] = [
				"devicefile" => $sd->getDeviceFile(),
				"size" => $sd->getSize(),
				"vendor" => $sd->getVendor(),
				"serialnumber" => $sd->getSerialNumber(),
				"description" => $sd->getDescription()
			];
		}
		return $result;
	}

	/**
	 * Get list of devices that can be used to create a RAID.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing objects with the following fields:
	 *   devicefile, size, vendor, serialnumber and description.
	 */
	public function getCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get a list of all potential usable devices.
		if (FALSE === ($devs = \OMV\System\Storage\StorageDevice::enumerateUnused()))
			throw new \OMV\Exception("Failed to get list of unused devices.");
		// Prepare the result list.
		$result = [];
		foreach ($devs as $devk => $devv) {
			// Get the object that implements the given storage device.
			$sd = \OMV\System\Storage\StorageDevice::getStorageDevice($devv);
			if (is_null($sd) || !$sd->exists())
				continue;
			// Skip read-only devices like CDROM.
			if (TRUE === $sd->isReadOnly())
				continue;
			// Skip devices that are connected via USB.
			if (TRUE === $sd->isUsb())
				continue;
			// Check if the device is referenced/used by a plugin.
			$db = \OMV\Config\Database::getInstance();
			if (TRUE === $db->exists("conf.service", [
					"operator" => "stringContains",
					"arg0" => "devicefile",
					"arg1" => $sd->getDeviceFile()
				]))
				continue;
			// Does this device already contain a filesystem?
			if (FALSE !== \OMV\System\Filesystem\Filesystem::hasFileSystem(
				$sd->getDeviceFile()))
				continue;
			// The device is a potential candidate to be a member of a
			// Software RAID.
			$result[] = [
				"devicefile" => $sd->getDeviceFile(),
				"size" => $sd->getSize(),
				"vendor" => $sd->getVendor(),
				"serialnumber" => $sd->getSerialNumber(),
				"description" => $sd->getDescription()
			];
		}
		return $result;
	}

	/**
	 * Create a RAID device.
	 * @param params The method parameters.
	 *   \em devices The devices to be used.
	 *   \em name The name of the RAID device. Can be empty.
	 *   \em level The RAID level, e.g. stripe, mirror, linear ,raid10,
	 *   raid5 or raid6.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function create($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.mdmgmt.create");
		// Check if name is unique.
		if (!empty($params['name'])) {
			// A name looks like: '<HOSTNAME>:<NAME>'
			$name = sprintf("%s:%s", \OMV\System\Net\Dns::getHostname(),
				$params['name']);
			$devs = $this->callMethod("enumerateDevices", NULL, $context);
			if (!empty(array_filter_ex($devs, 'name', $name))) {
				throw new \OMV\Exception("The name '%s' already exists.",
					$params['name']);
			}
		}
		// Generate the RAID device name.
		$deviceName = \OMV\System\System::getNextDevice("disk", "md");
		$devices = $params['devices'];
		// Create the RAID device.
		$cmdArgs = [];
		$cmdArgs[] = sprintf("/dev/%s", $deviceName);
		$cmdArgs[] = "-l";
		$cmdArgs[] = $params['level'];
		$cmdArgs[] = "-n";
		$cmdArgs[] = count($devices);
		if (!empty($params['name'])) {
			$cmdArgs[] = "-N";
			$cmdArgs[] = $params['name'];
		}
		$cmdArgs[] = implode(" ", $devices);
		$cmd = new \OMV\System\Process("omv-mkraid", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Notify the configuration change.
		$object = new \OMV\Config\ConfigObject("conf.system.mdadm.device");
		$object->setAssoc([
			"name" => $params['name'],
			"devicefile" => sprintf("/dev/%s", $deviceName),
			"level" => $params['level'],
			"numdevices" => count($devices),
//			"devices" => $devices
		]);
		$object->set("devices", $devices); // Workaround
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
			$object->getModel()->getNotificationId(), $object->getAssoc());
		// Return the 'fake' configuration object.
		return $object->getAssoc();
	}

	/**
	 * Grow a RAID device.
	 * @param params The method parameters.
	 *   \em devicefile The RAID device file, e.g. /dev/md0.
	 *   \em devices The devices to be added.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function grow($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.mdmgmt.grow");
		// Get MD software RAID device details to create notification message.
		$raid = new \OMV\System\Storage\StorageDeviceMd(
		  $params['devicefile']);
		$raid->assertExists();
		$numDevices = $raid->getNumDevices();
		$devices = $raid->getSlaves();
		// Calculate new number of devices.
		$newDevices = $params['devices'];
		$numDevices += count($newDevices);
		// Add new devices to the RAID device.
		$cmdArgs = [];
		$cmdArgs[] = "--manage";
		$cmdArgs[] = escapeshellarg($raid->getDeviceFile());
		$cmdArgs[] = "--add";
		$cmdArgs[] = implode(" ", $newDevices);
		$cmd = new \OMV\System\Process("mdadm", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Use the largest size that fits on all current drives.
		$cmdArgs = [];
		$cmdArgs[] = "--grow";
		$cmdArgs[] = "--size=max";
		$cmdArgs[] = escapeshellarg($raid->getDeviceFile());
		$cmd = new \OMV\System\Process("mdadm", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Specify the new number of active devices in the array.
		// See for more details:
		// https://raid.wiki.kernel.org/index.php/Growing
		// https://raid.wiki.kernel.org/index.php/Write-intent_bitmap
		$cmdArgs = [];
		$cmdArgs[] = "--grow";
		$cmdArgs[] = sprintf("--raid-devices=%d", $numDevices);
		$cmdArgs[] = escapeshellarg($raid->getDeviceFile());
		$cmd = new \OMV\System\Process("mdadm", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Notify the configuration change.
		$object = new \OMV\Config\ConfigObject("conf.system.mdadm.device");
		$object->setAssoc([
			"name" => $raid->getName(),
			"devicefile" => $raid->getDeviceFile(),
			"level" => $raid->getLevel(),
			"numdevices" => $numDevices,
//			"devices" => array_merge($devices, $newDevices)
		]);
		$object->set("devices", array_merge($devices, $newDevices)); // Workaround
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
			$object->getModel()->getNotificationId(), $object->getAssoc());
		// Return the 'fake' configuration object.
		return $object->getAssoc();
	}

	/**
	 * Add devices to a RAID device.
	 * @param params The method parameters.
	 *   \em devicefile The RAID device file, e.g. /dev/md0.
	 *   \em devices The devices to be added.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function add($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.mdmgmt.add");
		// Get MD software RAID device details to create notification message.
		$raid = new \OMV\System\Storage\StorageDeviceMd(
		  $params['devicefile']);
		$raid->assertExists();
		$numDevices = $raid->getNumDevices();
		$devices = $raid->getSlaves();
		// Calculate new number of devices.
		$newDevices = $params['devices'];
		$numDevices += count($newDevices);
		// Add new devices to the RAID device.
		$cmdArgs = [];
		$cmdArgs[] = "--manage";
		$cmdArgs[] = escapeshellarg($raid->getDeviceFile());
		$cmdArgs[] = "--add";
		$cmdArgs[] = implode(" ", $newDevices);
		$cmd = new \OMV\System\Process("mdadm", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Notify the configuration change.
		$object = new \OMV\Config\ConfigObject("conf.system.mdadm.device");
		$object->setAssoc([
			"name" => $raid->getName(),
			"devicefile" => $raid->getDeviceFile(),
			"level" => $raid->getLevel(),
			"numdevices" => $numDevices,
//			"devices" => array_merge($devices, $newDevices)
		]);
		$object->set("devices", array_merge($devices, $newDevices)); // Workaround
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
			$object->getModel()->getNotificationId(), $object->getAssoc());
		// Return the 'fake' configuration object.
		return $object->getAssoc();
	}

	/**
	 * Remove devices from a RAID device. The MD superblock of the removed
	 * devices is not affected or removed.
	 * @param params The method parameters.
	 *   \em devicefile The RAID device file, e.g. /dev/md0.
	 *   \em devices The devices to be removed.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function remove($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.mdmgmt.remove");
		// Get MD software RAID device details to create notification message.
		$raid = new \OMV\System\Storage\StorageDeviceMd(
		  $params['devicefile']);
		$raid->assertExists();
		$numDevices = $raid->getNumDevices();
		$devices = $raid->getSlaves();
		// Calculate new number of devices.
		$rmDevices = $params['devices'];
		$numDevices += count($rmDevices);
		// Remove the devices from the RAID device. The devices to remove
		// must be marked as failed before they can be removed from the
		// RAID device.
		// - Mark the devices as failed.
		$cmdArgs = [];
		$cmdArgs[] = "--manage";
		$cmdArgs[] = escapeshellarg($raid->getDeviceFile());
		$cmdArgs[] = "--fail";
		$cmdArgs[] = implode(" ", $rmDevices);
		$cmd = new \OMV\System\Process("mdadm", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
		// - Finally remove the devices.
		$cmdArgs = [];
		$cmdArgs[] = "--manage";
		$cmdArgs[] = escapeshellarg($raid->getDeviceFile());
		$cmdArgs[] = "--remove";
		$cmdArgs[] = implode(" ", $rmDevices);
		$cmd = new \OMV\System\Process("mdadm", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Notify the configuration change.
		$object = new \OMV\Config\ConfigObject("conf.system.mdadm.device");
		$object->setAssoc([
			"name" => $raid->getName(),
			"devicefile" => $raid->getDeviceFile(),
			"level" => $raid->getLevel(),
			"numdevices" => $numDevices,
//			"devices" => array_diff($devices, $rmDevices)
		]);
		$object->set("devices", array_values(array_diff($devices,
			$rmDevices))); // Workaround
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
			$object->getModel()->getNotificationId(), $object->getAssoc());
		// Return the 'fake' configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a RAID device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The RAID device file to be deleted, e.g. /dev/md0.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function delete($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get MD software RAID device details to create notification message.
		$raid = new \OMV\System\Storage\StorageDeviceMd(
		  $params['devicefile']);
		$raid->assertExists();
		// Delete the RAID device.
		$cmdArgs = [];
		$cmdArgs[] = escapeshellarg($raid->getDeviceFile());
		$cmd = new \OMV\System\Process("omv-rmraid", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Notify the configuration change.
		$object = new \OMV\Config\ConfigObject("conf.system.mdadm.device");
		$object->setAssoc([
			"name" => $raid->getName(),
			"devicefile" => $raid->getDeviceFile(),
			"level" => $raid->getLevel(),
			"numdevices" => $raid->getNumDevices(),
//			"devices" => $raid->getSlaves()
		]);
		$object->set("devices", $raid->getSlaves()); // Workaround
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
			$object->getModel()->getNotificationId(), $object->getAssoc());
		// Return the 'fake' configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get detail about a RAID device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The RAID device file to get details from.
	 * @param context The context of the caller.
	 * @return The details of the given RAID device as string.
	 */
	public function getDetail($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the MD software RAID device and make sure it exists.
		$raid = new \OMV\System\Storage\StorageDeviceMd(
		  $params['devicefile']);
		$raid->assertExists();
		// Get the software RAID details.
		return $raid->getDetail();
	}
}
