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

class Smart extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Smart";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("enumerateDevices");
		$this->registerMethod("enumerateMonitoredDevices");
		$this->registerMethod("getList");
		$this->registerMethod("getListBg");
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
		$this->registerMethod("getDeviceSettings");
		$this->registerMethod("setDeviceSettings");
		$this->registerMethod("getAttributes");
		$this->registerMethod("getSelfTestLogs");
		$this->registerMethod("getInformation");
		$this->registerMethod("getExtendedInformation");
		$this->registerMethod("getScheduleList");
		$this->registerMethod("getScheduledTest");
		$this->registerMethod("setScheduledTest");
		$this->registerMethod("deleteScheduledTest");
		$this->registerMethod("executeScheduledTest");
	}

	/**
	 * Enumerate all devices supporting S.M.A.R.T.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing disk device objects with the
	 *   following fields: \em devicename, \em canonicaldevicefile,
	 *   \em devicefile, \em devicelinks, \em model, \em size,
	 *   \em temperature, \em description, \em vendor and
	 *   \em serialnumber.
	 * @throw \OMV\Exception
	 */
	public function enumerateDevices($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get all existing hard disks devices.
		if (FALSE === ($devs = \OMV\System\Storage\StorageDevice::enumerate(
				OMV_STORAGE_DEVICE_TYPE_ALL & ~(OMV_STORAGE_DEVICE_TYPE_LOOPDEVICE |
				OMV_STORAGE_DEVICE_TYPE_SOFTWARERAID |
				OMV_STORAGE_DEVICE_TYPE_DEVICEMAPPER)))) {
			throw new \OMV\Exception("Failed to get list of storage devices");
		}
		// Prepare result.
		$procs = [];
		foreach ($devs as $devk => $devv) {
			// Collect the device information asynchronous.
			$procs[] = $this->asyncProc(function() use ($devv) {
				// Get the storage device object.
				$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice($devv);
				// Skip devices that do not support S.M.A.R.T.
				if (FALSE === $sd->hasSmartSupport()) {
					return FALSE;
				}
				// Get the S.M.A.R.T. information about the given device.
				$si = $sd->getSmartInformation();
				// Try to get the device temperature via S.M.A.R.T.
				$temperature = $si->getTemperature();
				// Get the device details collected via S.M.A.R.T.
				$info = $si->getInformation();
				// Prepare device object
				return [
					"devicename" => $sd->getDeviceName(),
					"canonicaldevicefile" => $sd->getCanonicalDeviceFile(),
					"devicefile" => $sd->getPredictableDeviceFile(),
					"devicelinks" => $sd->getDeviceFileSymlinks(),
					"model" => !empty($info['devicemodel']) ?
						$info['devicemodel'] : $sd->getModel(),
					"size" => $sd->getSize(),
					"temperature" => (FALSE === $temperature) ?
						"" : $temperature,
					"description" => $sd->getDescription(),
					"vendor" => $sd->getVendor(),
					"serialnumber" => !empty($info['serialnumber']) ?
						$info['serialnumber'] : $sd->getSerialNumber(),
					"wwn" => $sd->getWorldWideName(),
					"overallstatus" => $si->getOverallStatus()
				];
			}, function($e) use ($devv) {
				// Try to get as much information as possible about this device.
				// However, this will not contain any SMART information.
				$sd = \OMV\System\Storage\StorageDevice::getStorageDevice($devv);
				if (is_null($sd) || !$sd->exists()) {
					return FALSE;
				}
				return [
					"result" => [
						"devicename" => $sd->getDeviceName(),
						"canonicaldevicefile" => $sd->getCanonicalDeviceFile(),
						"devicefile" => $sd->getPredictableDeviceFile(),
						"devicelinks" => $sd->getDeviceFileSymlinks(),
						"model" => $sd->getModel(),
						"size" => $sd->getSize(),
						"temperature" => "",
						"description" => $sd->getDescription(),
						"vendor" => $sd->getVendor(),
						"serialnumber" => $sd->getSerialNumber(),
						"wwn" => $sd->getWorldWideName(),
						"overallstatus" => \OMV\System\Storage\SmartInformation::SMART_ASSESSMENT_BAD_STATUS
					],
					"error" => FALSE
				];
			});
		}
		// Remove elements that are no associative arrays (those devices
		// that have returned FALSE because of missing S.M.A.R.T. support).
		// Re-index the filtered array.
		return array_values(array_filter(
			$this->waitAsyncProcs($procs), "is_assoc_array")
		);
	}

	/**
	 * Get list of devices that are monitored.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing disk device objects with the
	 *   following fields: \em devicename, \em devicefile, \em devicelinks,
	 *   \em model, \em size, \em temperature, \em description, \em vendor
	 *   and \em serialnumber.
	 */
	public function enumerateMonitoredDevices($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Enumerate all devices supporting S.M.A.R.T.
		$objects = $this->callMethod("enumerateDevices", NULL, $context);
		// Prepare the result list, only return devices that are monitored.
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			// Check if there exists a device configuration object for the
			// current processed device. Note, we have to check for 'dev/xxx'
			// and 'dev/disk/by-id/xxx' entries.
			$db = \OMV\Config\Database::getInstance();
			$confObjects = $db->getByFilter(
				"conf.service.smartmontools.device", [
					"operator" => "stringEnum",
					"arg0" => "devicefile",
					"arg1" => array_merge([ $objectv['devicefile'] ],
  						$objectv['devicelinks'])
				]);
			if (0 >= count($confObjects))
				continue;
			$confObject = $confObjects[0];
			if (TRUE !== $confObject->get("enable"))
				continue;
			$result[] = $objectv;
		}
		return $result;
	}

	/**
	 * Get all devices supporting S.M.A.R.T.
	 * @param object $params An object containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @param object $context The context of the caller.
	 * @return array An array containing the requested objects. The field
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
		// Enumerate all devices supporting S.M.A.R.T.
		$objects = $this->callMethod("enumerateDevices", NULL, $context);
		// Append additional configuration data.
		foreach ($objects as $objectk => &$objectv) {
			$objectv['uuid'] = \OMV\Environment::get(
				"OMV_CONFIGOBJECT_NEW_UUID");
			$objectv['monitor'] = FALSE;
			// Prepare the filter that is used to find the device settings
			// configuration object and to check if there are scheduled
			// jobs for this device.
			$filter = [
				"operator" => "stringEnum",
				"arg0" => "devicefile",
				"arg1" => array_merge([ $objectv['devicefile'] ],
					$objectv['devicelinks'])
			];
			// Check if there exists a device configuration object for the
			// given device. Note, we have to check for 'dev/xxx' and
			// 'dev/disk/by-id/xxx' entries.
			$db = \OMV\Config\Database::getInstance();
			$confObjects = $db->getByFilter(
				"conf.service.smartmontools.device", $filter);
			// If found, then finally get the S.M.A.R.T. device settings
			// configuration object.
			if (0 < count($confObjects)) {
				$confObject = $confObjects[0];
				// Append the device settings.
				$objectv['uuid'] = $confObject->get("uuid");
				$objectv['monitor'] = $confObject->get("enable");
			}
			// Are there any scheduled task associated to this device?
			$objectv['_used'] = $db->exists(
				"conf.service.smartmontools.job", $filter);
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
	 * Get the settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 */
	public function getSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.smartmontools");
		// Remove useless properties from the object.
		$object->remove("monitor");
		$object->remove("scheduledtests");
		return $object->getAssoc();
	}

	/**
	 * Set the settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.smart.setsettings");
		// Get the existing configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.smartmontools");
		$object->setAssoc($params);
		// Set the configuration object.
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get the device settings.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file, e.g.
	 *     \li /dev/sdb
	 *     \li /dev/disk/by-id/ata-ST31000528AS_8VI5C2AZ
	 * @param context The context of the caller.
	 * @return The configuration object.
	 */
	public function getDeviceSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Set the default filter argument.
		$filter = [
			"operator" => "stringEnum",
			"arg0" => "devicefile",
			"arg1" => [ $params['devicefile'] ]
		];
		// If the given device file exists, then extend the filter by
		// adding the all device files retrieved via UDEV. This will
		// increase the probability that we will have a hit on the
		// database query.
		$sd = \OMV\System\Storage\StorageDevice::getStorageDevice(
			$params['devicefile']);
		if (!is_null($sd) && $sd->exists()) {
			array_push($filter["arg1"], ...$sd->getDeviceFileSymlinks());
		}
		// Get the configuration object. Assert one object exists.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->getByFilter("conf.service.smartmontools.device",
			$filter, 1);
		// Are there any scheduled task associated to this device?
		$object->add("_used", "boolean", FALSE);
		$object->set("_used", $db->exists(
			"conf.service.smartmontools.job", $filter));
		return $object->getAssoc();
	}

	/**
	 * Set the device settings.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 *   \em devicefile The device file, e.g.
	 *     \li /dev/sdb
	 *     \li /dev/disk/by-id/ata-ST31000528AS_8VI5C2AZ
	 *   \em enable Enable/disable S.M.A.R.T. monitoring.
	 *   \em tempdiff Report if the temperature had changed by at
	 *     least N degrees Celsius since last report.
	 *   \em tempmax Report if the temperature is greater than or
	 *     equal to N degrees Celsius.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setDeviceSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.smart.setdevicesettings");
		// Check if the given device exists and supports S.M.A.R.T.
		$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
			$params['devicefile']);
		// Ensure that the device has S.M.A.R.T. support.
		$sd->assertHasSmartSupport();
		// Prepare the configuration data.
		$object = new \OMV\Config\ConfigObject(
		  "conf.service.smartmontools.device");
		$object->setAssoc([
			"uuid" => $params['uuid'],
			"enable" => $params['enable'],
			"devicefile" => $sd->getPredictableDeviceFile(),
			"devicetype" => $sd->getSmartDeviceType(),
			"tempdiff" => $params['tempdiff'],
			"tempmax" => $params['tempmax']
		]);
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
	 * Get the extended S.M.A.R.T. information about the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file, e.g. /dev/sdb.
	 * @param context The context of the caller.
	 * @return The requested extended S.M.A.R.T. information as plain text.
	 */
	public function getExtendedInformation($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the S.M.A.R.T. information.
		$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
			$params['devicefile']);
		// Ensure that the device has S.M.A.R.T. support.
		$sd->assertHasSmartSupport();
		// Get the S.M.A.R.T. information.
		$si = $sd->getSmartInformation();
		return $si->getExtendedInformation();
	}

	/**
	 * Get 'S.M.A.R.T. Attributes Data Structure' about the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file, e.g. /dev/sdb.
	 * @param context The context of the caller.
	 * @return The requested S.M.A.R.T. attributes.
	 */
	public function getAttributes($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the S.M.A.R.T. information.
		$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
			$params['devicefile']);
		// Ensure that the device has S.M.A.R.T. support.
		$sd->assertHasSmartSupport();
		// Get the S.M.A.R.T. information.
		$si = $sd->getSmartInformation();
		return $si->getAttributes();
	}

	/**
	 * Get 'SMART Self-test log structure' about the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file, e.g. /dev/sdb.
	 * @param context The context of the caller.
	 * @return The requested Self-test logs.
	 */
	public function getSelfTestLogs($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the S.M.A.R.T. information.
		$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
			$params['devicefile']);
		// Ensure that the device has S.M.A.R.T. support.
		$sd->assertHasSmartSupport();
		// Get the S.M.A.R.T. information.
		$si = $sd->getSmartInformation();
		return $si->getSelfTestLogs();
	}

	/**
	 * Get all S.M.A.R.T. identity information for the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file, e.g. /dev/sdb.
	 * @param context The context of the caller.
	 * @return The requested S.M.A.R.T. identity information.
	 */
	public function getInformation($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the S.M.A.R.T. information.
		$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
			$params['devicefile']);
		// Ensure that the device has S.M.A.R.T. support.
		$sd->assertHasSmartSupport();
		// Get the S.M.A.R.T. information.
		$si = $sd->getSmartInformation();
		$result = $si->getInformation();
		// Get additional information:
		// - Temperature
		// - Power Cycle Count
		// - Power-On Hours (POH)
		if (FALSE !== ($value = $si->getTemperature())) {
			$result['temperature'] = $value;
		}
		if (-1 !== ($value = $si->getPowerCycleCount())) {
			$result['powercycles'] = $value;
		}
		if (-1 !== ($value = $si->getPowerOnHours())) {
			$result['poweronhours'] = $value;
		}
		return $result;
	}

	/**
	 * Get list of schedule configuration objects.
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
	public function getScheduleList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.smartmontools.job");
		// Add additional share informations.
		$objectsAssoc = [];
		foreach ($objects as $objectk => &$objectv) {
			$objectAssoc = $objectv->getAssoc();
			// Append storage device information.
			$objectAssoc = array_merge($objectAssoc, [
				"devicefile" => "",
				"devicelinks" => [],
				"model" => "",
				"serialnumber" => "",
				"size" => "-1" // as string
			]);
			// Get the storage device object.
			$sd = \OMV\System\Storage\StorageDevice::getStorageDevice(
				$objectv->get("devicefile"));
			if (!is_null($sd) && $sd->exists()) {
				// Update storage device information.
				$objectAssoc = array_merge($objectAssoc, [
					"devicefile" => $sd->getCanonicalDeviceFile(),
					"devicelinks" => $sd->getDeviceFileSymlinks(),
					"model" => $sd->getModel(),
					"serialnumber" => $sd->getSerialNumber(),
					"size" => $sd->getSize()
				]);
			}
			$objectsAssoc[] = $objectAssoc;
		}
		// Filter result.
		return $this->applyFilter($objectsAssoc, $params['start'],
			$params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a scheduled test configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function getScheduledTest($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		return $db->getAssoc("conf.service.smartmontools.job",
			$params['uuid']);
	}

	/**
	 * Set a scheduled test config object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setScheduledTest($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.smart.setscheduledtest");
		// Check if the given device exists.
		$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
			$params['devicefile']);
		// Ensure that the device has S.M.A.R.T. support.
		$sd->assertHasSmartSupport();
		// Use a predictable device file.
		$params['devicefile'] = $sd->getPredictableDeviceFile();
		// Prepare the configuration data.
		$object = new \OMV\Config\ConfigObject(
			"conf.service.smartmontools.job");
		$object->setAssoc($params);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Delete a scheduled test configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function deleteScheduledTest($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.smartmontools.job", $params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}

	/**
	 * Execute a scheduled test.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 */
	public function executeScheduledTest($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$jobObject = $db->get("conf.service.smartmontools.job", $params['uuid']);
		// Build the filter to query the device settings.
		$filter = [
			"operator" => "stringEnum",
			"arg0" => "devicefile",
			"arg1" => [ $jobObject->get("devicefile") ]
		];
		// If the given device file exists, then extend the filter by
		// adding the all device files retrieved via UDEV. This will
		// increase the probability that we will have a hit on the
		// database query.
		$sd = \OMV\System\Storage\StorageDevice::getStorageDevice(
			$jobObject->get("devicefile"));
		if (!is_null($sd) && $sd->exists()) {
			array_push($filter["arg1"], ...$sd->getDeviceFileSymlinks());
		}
		// Get the associated device configuration object.
		$deviceObject = $db->getByFilter("conf.service.smartmontools.device",
			$filter, 1);
		// Create the background process.
		return $this->execBgProc(function($bgStatusFilename, $bgOutputFilename)
		  use ($jobObject, $deviceObject) {
			// Execute the S.M.A.R.T. self test.
			$testMap = [
				"s" => "short",
				"l" => "long",
				"c" => "conveyance",
				"o" => "offline"
			];
			$test = mb_strtolower($jobObject->get("type"));
			if (TRUE === array_key_exists($test, $testMap))
				$test = $testMap[$test];
			$cmdArgs = [];
			$cmdArgs[] = "--xall";
			$cmdArgs[] = sprintf("--test=%s", $test);
			$deviceType = $deviceObject->get("devicetype");
			if (!empty($deviceType))
				$cmdArgs[] = sprintf("--device=%s", $deviceType);
			$cmdArgs[] = $jobObject->get("devicefile");
			$cmd = new \OMV\System\Process("smartctl", $cmdArgs);
			$cmd->setRedirect2to1();
			// Execute the command.
			$exitStatus = $this->exec($cmd, $output, $bgOutputFilename);
			// Check the exit status:
			// Bit 0: Command line did not parse
			// Bit 1: Device open failed, or device did not return an
			//        IDENTIFY DEVICE structure
			if (($exitStatus !== 0) && (($exitStatus & 0) ||
				($exitStatus & 1))) {
				throw new \OMV\ExecException($cmd, $output, $exitStatus);
			}
			return $output;
		});
	}
}
