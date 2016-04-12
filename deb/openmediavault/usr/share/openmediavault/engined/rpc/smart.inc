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
class OMVRpcServiceSmart extends \OMV\Rpc\ServiceAbstract {
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
	 *   following fields: \em devicename, \em devicefile, \em model,
	 *   \em size, \em temperature, \em description, \em vendor and
	 *   \em serialnumber.
	 * @throw \OMV\Exception
	 */
	public function enumerateDevices($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get all existing hard disks devices.
		if (FALSE === ($devs = \OMV\System\Storage\StorageDevice::enumerate(
		  OMV_STORAGE_DEVICE_TYPE_ALL & ~(OMV_STORAGE_DEVICE_TYPE_LOOPDEVICE |
		  OMV_STORAGE_DEVICE_TYPE_SOFTWARERAID)))) {
			throw new \OMV\Exception("Failed to get list of storage devices");
		}
		// Prepare result.
		$objects = array();
		foreach ($devs as $devk => $devv) {
			// Get the storage device object.
			\OMV\System\Storage\StorageDevice::assertStorageDeviceExists($devv);
			$sd = \OMV\System\Storage\StorageDevice::getStorageDevice($devv);
			// Skip devices that do not support S.M.A.R.T.
			if (FALSE === $sd->hasSmartSupport())
				continue;
			// Get the S.M.A.R.T. information about the given device.
			$smartInfo = $sd->getSmartInformation();
			// Try to get the device temperature via S.M.A.R.T.
			$temperature = $smartInfo->getTemperature();
			// Prepare device object
			$objects[] = array(
				"devicename" => $sd->getDeviceName(),
				"devicefile" => $sd->getDeviceFile(),
				"devicefilebyid" => $sd->getDeviceFileById(),
				"model" => $sd->getModel(),
				"size" => $sd->getSize(),
				"temperature" => (FALSE === $temperature) ?
				  "" : sprintf("%d°C", $temperature),
				"description" => $sd->getDescription(),
				"vendor" => $sd->getVendor(),
				"serialnumber" => $sd->getSerialNumber(),
				"overallstatus" => $smartInfo->getOverallStatus()
			);
		}
		return $objects;
	}

	/**
	 * Get list of devices that are monitored.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing disk device objects with the
	 *   following fields: \em devicename, \em devicefile, \em model,
	 *   \em size, \em temperature, \em description, \em vendor and
	 *   \em serialnumber.
	 */
	public function enumerateMonitoredDevices($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Enumerate all devices supporting S.M.A.R.T.
		$objects = $this->callMethod("enumerateDevices", NULL, $context);
		// Prepare the result list, only return devices that are monitored.
		$result = array();
		foreach ($objects as $objectk => &$objectv) {
			// Check if there exists a device configuration object for the
			// current processed device. Note, we have to check for 'dev/xxx'
			// and 'dev/disk/by-id/xxx' entries.
			$db = \OMV\Config\Database::getInstance();
			$predicate = sprintf("devicefile='%s' or devicefile='%s'",
			  $objectv['devicefilebyid'], $objectv['devicefile']);
			$confObjects = $db->getByPredicate(
			  "conf.service.smartmontools.device",
			  $predicate);
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
		// Enumerate all devices supporting S.M.A.R.T.
		$objects = $this->callMethod("enumerateDevices", NULL, $context);
		// Append additional configuration data.
		foreach ($objects as $objectk => &$objectv) {
			$objectv['uuid'] = $GLOBALS['OMV_UUID_UNDEFINED'];
			$objectv['monitor'] = FALSE;
			// Check if there exists a device configuration object for the
			// given device. Note, we have to check for 'dev/xxx' and
			// 'dev/disk/by-id/xxx' entries.
			$db = \OMV\Config\Database::getInstance();
			$predicate = sprintf("devicefile='%s' or devicefile='%s'",
			  $objectv['devicefilebyid'], $objectv['devicefile']);
			$confObjects = $db->getByPredicate(
			  "conf.service.smartmontools.device",
			  $predicate);
			// If found, then finally get the S.M.A.R.T. device settings
			// configuration object.
			if (0 < count($confObjects)) {
				$confObject = $confObjects[0];
				// Append the device settings.
				$objectv['uuid'] = $confObject->get("uuid");
				$objectv['monitor'] = $confObject->get("enable");
			}
		}
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get the settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 */
	public function getSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.smart.setsettings");
		// Get the existing configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.smartmontools");
		$object->setAssoc($params);
		// Set the configuration object.
		$db->set($object);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smart", $object->getAssoc());
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.smart.getdevicesettings");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		return $db->getAssoc("conf.service.smartmontools.device",
		  "devicefile", $params['devicefile']);
	}

	/**
	 * Set the device settings.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 *   \em devicefile The device file, e.g.
	 *     \li /dev/sdb
	 *     \li /dev/disk/by-id/ata-ST31000528AS_8VI5C2AZ
	 *   \em enable Enable/disable S.M.A.R.T. monitoring.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function setDeviceSettings($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Check if the given device exists and supports S.M.A.R.T.
		\OMV\System\Storage\StorageDevice::assertStorageDeviceExists(
		  $params['devicefile']);
		$sd = \OMV\System\Storage\StorageDevice::getStorageDevice(
		  $params['devicefile']);
		// Ensure that the device has S.M.A.R.T. support.
		$sd->assertHasSmartSupport();
		// Prepare the configuration data.
		$object = new ConfigObject("conf.service.smartmontools.device");
		$object->setAssoc([
			"uuid" => $params['uuid'],
			"enable" => $params['enable'],
			"devicefile" => $sd->getDeviceFileById(),
			"devicetype" => $sd->getSmartDeviceType()
		]);
		// Set the configuration object.
		$isNew = $object->isNew();
		$db = \OMV\Config\Database::getInstance();
		if (TRUE === $isNew) {
			// Check uniqueness.
			$db->assertIsUnique($object, "devicefile");
		}
		$db->set($object);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify($isNew ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smart.monitor.device",
		  $object->getAssoc());
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the S.M.A.R.T. information.
		\OMV\System\Storage\StorageDevice::assertStorageDeviceExists(
		  $params['devicefile']);
		$sd = \OMV\System\Storage\StorageDevice::getStorageDevice(
		  $params['devicefile']);
		// Ensure that the device has S.M.A.R.T. support.
		$sd->assertHasSmartSupport();
		// Get the S.M.A.R.T. information.
		$smartInfo = $sd->getSmartInformation();
		if (FALSE === ($result = $smartInfo->getExtendedInformation())) {
			throw new \OMV\Exception(
			  "Failed to get S.M.A.R.T. extended information ".
			  "(device=%s, type=%s): %s", $smartInfo->getDeviceFile(),
			  $smartInfo->getType(), $smartInfo->getLastError());
		}
		return $result;
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the S.M.A.R.T. information.
		\OMV\System\Storage\StorageDevice::assertStorageDeviceExists(
		  $params['devicefile']);
		$sd = \OMV\System\Storage\StorageDevice::getStorageDevice(
		  $params['devicefile']);
		// Ensure that the device has S.M.A.R.T. support.
		$sd->assertHasSmartSupport();
		// Get the S.M.A.R.T. information.
		$smartInfo = $sd->getSmartInformation();
		if (FALSE === ($result = $smartInfo->getAttributes())) {
			throw new \OMV\Exception(
			  "Failed to get S.M.A.R.T. attributes for device '%s': %s",
			  $sd->getDeviceFile(), $smartInfo->getLastError());
		}
		return $result;
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the S.M.A.R.T. information.
		\OMV\System\Storage\StorageDevice::assertStorageDeviceExists(
		  $params['devicefile']);
		$sd = \OMV\System\Storage\StorageDevice::getStorageDevice(
		  $params['devicefile']);
		// Ensure that the device has S.M.A.R.T. support.
		$sd->assertHasSmartSupport();
		// Get the S.M.A.R.T. information.
		$smartInfo = $sd->getSmartInformation();
		if (FALSE === ($result = $smartInfo->getSelfTestLogs())) {
			throw new \OMV\Exception(
			  "Failed to get S.M.A.R.T. self-test logs for device '%s': %s",
			  $sd->getDeviceFile(), $smartInfo->getLastError());
		}
		return $result;
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Get the S.M.A.R.T. information.
		\OMV\System\Storage\StorageDevice::assertStorageDeviceExists(
		  $params['devicefile']);
		$sd = \OMV\System\Storage\StorageDevice::getStorageDevice(
		  $params['devicefile']);
		// Ensure that the device has S.M.A.R.T. support.
		$sd->assertHasSmartSupport();
		// Get the S.M.A.R.T. information.
		$smartInfo = $sd->getSmartInformation();
		if (FALSE === ($result = $smartInfo->getInformation())) {
			throw new \OMV\Exception(
			  "Failed to get S.M.A.R.T. information for device '%s': %s",
			  $sd->getDeviceFile(), $smartInfo->getLastError());
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
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.smartmontools.job");
		// Add additional share informations.
		$objectsAssoc = array();
		foreach ($objects as $objectk => &$objectv) {
			$objectAssoc = $objectv->getAssoc();
			// Append storage device information.
			$objectAssoc = array_merge($objectAssoc, [
				"devicefile" => gettext("n/a"),
				"devicefilebyid" => "",
				"model" => gettext("n/a"),
				"size" => "-1" // as string
			]);
			// Get the storage device object.
			$sd = \OMV\System\Storage\StorageDevice::getStorageDevice(
			  $objectv->get("devicefile"));
			if (!is_null($sd) && $sd->exists()) {
				// Update storage device information.
				$objectAssoc = array_merge($objectAssoc, [
					"devicefile" => $sd->getDeviceFile(),
					"devicefilebyid" => $sd->getDeviceFileById(),
					"model" => $sd->getModel(),
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.smart.setscheduledtest");
		// Check if the given device exists.
		\OMV\System\Storage\StorageDevice::assertStorageDeviceExists(
		  $params['devicefile']);
		$sd = \OMV\System\Storage\StorageDevice::getStorageDevice(
		  $params['devicefile']);
		// Ensure that the device has S.M.A.R.T. support.
		$sd->assertHasSmartSupport();
		// Prepare the configuration data.
		$object = new \OMV\Config\ConfigObject("conf.service.smartmontools.job");
		$object->setAssoc($params);
		$object->set("devicefile", $sd->getDeviceFileById());
		$object->set("devicetype", $sd->getSmartDeviceType());
		// Set the configuration object.
		$isNew = $object->isNew();
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify($isNew ? OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smart.scheduledtests.job",
		  $object->getAssoc());
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		return $this->deleteConfigObjectByUuid(
		  "conf.service.smartmontools.job", $params['uuid'],
		  "org.openmediavault.services.smart.scheduledtests.job");
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
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
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
			// Get the configuration object.
			$db = \OMV\Config\Database::getInstance();
			$object = $db->get("conf.service.smartmontools.job",
			  $params['uuid']);
			// Execute the command line in a shell.
			$testMap = array(
				  "S" => "short",
				  "L" => "long",
				  "C" => "conveyance",
				  "O" => "offline"
			  );
			$cmd = "/usr/sbin/smartctl";
			$cmdArgs = array();
			$cmdArgs[] = sprintf("-t %s", $testMap[$object->get("type")]);
			if (FALSE === $object->isEmpty("devicetype"))
				$cmdArgs[] = sprintf("-d %s", $object->get("devicetype"));
			$cmdArgs[] = escapeshellarg($object->get("devicefile"));
			if (FALSE === $this->execve($cmd, $cmdArgs, $bgOutputFilename))
				throw new \OMV\ExecException($cmd." ".implode(" ", $cmdArgs));
		} catch(\Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}
}
