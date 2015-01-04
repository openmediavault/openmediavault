<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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
require_once("openmediavault/error.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceSmart extends OMVRpcServiceAbstract {
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
	 * @throw E_MISC_FAILURE
	 */
	public function enumerateDevices($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get all existing hard disks devices.
		if(FALSE === ($devs = OMVStorageDevices::enumerate(
		  OMV_STORAGE_DEVICE_TYPE_ALL & ~(OMV_STORAGE_DEVICE_TYPE_LOOPDEVICE |
		  OMV_STORAGE_DEVICE_TYPE_SOFTWARERAID)))) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  "Failed to get list of storage devices");
		}
		// Prepare result.
		$objects = array();
		foreach($devs as $devk => $devv) {
			// Get the storage device object.
			$sd = OMVStorageDeviceFactory::get($devv);
			if(is_null($sd) || !$sd->exists()) {
				throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
				  $devv);
			}
			// Skip devices that do not support S.M.A.R.T.
			if(FALSE === $sd->hasSMART())
				continue;
			// Get the S.M.A.R.T. information about the given device.
			if(is_null($smartInfo = $sd->getSMARTInformation())) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf("Failed to get S.M.A.R.T. interface (device=%s)",
				  $sd->getDeviceFile()));
			}
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
				  "serialnumber" => $sd->getSerialNumber()
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
	 * @throw E_MISC_FAILURE
	 */
	public function enumerateMonitoredDevices($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Enumerate all devices supporting S.M.A.R.T.
		$objects = $this->callMethod("enumerateDevices", NULL, $context);
		// Prepare the result list, only return devices that are monitored.
		$result = array();
		foreach($objects as $objectk => &$objectv) {
			// Check if there exists a device configuration object for the
			// given device. Note, we have to check for 'dev/xxx' and
			// 'dev/disk/by-id/xxx' entries.
			$xpath = sprintf("//services/smart/monitor/device[".
			  "devicefile='%s' or devicefile='%s']",
			  $objectv['devicefilebyid'], $objectv['devicefile']);
			$object = $xmlConfig->get($xpath);
			// If found, then finally get the S.M.A.R.T. device settings
			// configuration object.
			if(is_null($object))
				continue;
			if (TRUE !== boolvalEx($object['enable']))
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
	 * @throw E_CONFIG_OBJECT_INVALID
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
		// Enumerate all devices supporting S.M.A.R.T.
		$objects = $this->callMethod("enumerateDevices", NULL, $context);
		// Append additional configuration data.
		foreach ($objects as $objectk => &$objectv) {
			$objectv['uuid'] = $GLOBALS['OMV_UUID_UNDEFINED'];
			$objectv['monitor'] = FALSE;
			// Check if there exists a device configuration object for the
			// given device. Note, we have to check for 'dev/xxx' and
			// 'dev/disk/by-id/xxx' entries.
			$xpath = sprintf("//services/smart/monitor/device[".
			  "devicefile='%s' or devicefile='%s']",
			  $objectv['devicefilebyid'], $objectv['devicefile']);
			// If found, then finally get the S.M.A.R.T. device settings
			// configuration object.
			if (!is_null($object = $xmlConfig->get($xpath))) {
				// Ensure the retured value is an object, otherwise throw
				// an error.
				if (is_multiarray($object)) {
					throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_INVALID,
					  "A single object is expected");
				}
				// Get the S.M.A.R.T. settings of this device.
				$object = $this->callMethod("getDeviceSettings", array(
					  "devicefile" => $object['devicefile']
				  ), $context);
				$objectv['uuid'] = $object['uuid'];
				$objectv['monitor'] = $object['enable'];
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
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$xpath = "//services/smart";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data.
		$object['enable'] = boolvalEx($object['enable']);
		$object['interval'] = intval($object['interval']);
		$object['tempdiff'] = intval($object['tempdiff']);
		$object['tempinfo'] = intval($object['tempinfo']);
		$object['tempcrit'] = intval($object['tempcrit']);
		return $object;
	}

	/**
	 * Set the settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	public function setSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "enable":{"type":"boolean"},
				  "interval":{"type":"integer","minimum":10},
				  "powermode":{"type":"string","enum":["never","sleep",'.
					'"standby","idle"]},
				  "tempdiff":{"type":"integer","minimum":0},
				  "tempinfo":{"type":"integer","minimum":0},
				  "tempcrit":{"type":"integer","minimum":0}
			  }
		  }');
		// Get the existing configuration object.
		$xpath = "//services/smart";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update the configuration object.
		$object = array_merge($object, array(
			  "enable" => array_boolval($params, 'enable'),
			  "interval" => $params['interval'],
			  "powermode" => $params['powermode'],
			  "tempdiff" => $params['tempdiff'],
			  "tempinfo" => $params['tempinfo'],
			  "tempcrit" => $params['tempcrit']
		  ));
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smart", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Get the device settings.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file, e.g.
	 *     \li /dev/sdb
	 *     \li /dev/disk/by-id/ata-ST31000528AS_8VI5C2AZ
	 * @param context The context of the caller.
	 * @return The configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getDeviceSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "devicefile":{"type":"string","format":"regex",'.
					'"pattern":"#^/dev(/disk/by-id)?/.+$#i"}
			  }
		  }');
		// Get the configuration object.
		$xpath = sprintf("//services/smart/monitor/device[devicefile='%s']",
		  $params['devicefile']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data.
		$object['enable'] = boolvalEx($object['enable']);
		return $object;
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
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 * @throw E_MISC_FAILURE
	 * @throw E_CONFIG_OBJECT_UNIQUENESS
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	public function setDeviceSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
				  "devicefile":{"type":"string","format":"regex",'.
					'"pattern":"#^/dev(/disk/by-id)?/.+$#i"},
				  "enable":{"type":"boolean"}
			  }
		  }');
		// Check if the given device exists and supports S.M.A.R.T.
		$sd = OMVStorageDeviceFactory::get($params['devicefile']);
		if(is_null($sd) || !$sd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $params['devicefile']);
		}
		if(FALSE === $sd->hasSMART()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Device does not support S.M.A.R.T. (device=%s)",
			  $sd->getDeviceFile()));
		}
		// Prepare the configuration data.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"devicefile" => $sd->getDeviceFileById(),
			"enable" => array_boolval($params, 'enable'),
			"type" => $sd->getSMARTDeviceType()
		);
		// Set the configuration data.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness.
			$xpath = sprintf("//services/smart/monitor/device[devicefile='%s']",
			  $object['devicefile']);
			if(TRUE === $xmlConfig->exists($xpath)) {
				throw new OMVException(
				  OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				  sprintf("Settings already exist for device '%s'"),
				  $object['devicefile']);
			}
			// Append object to configuration.
			$success = $xmlConfig->set("//services/smart/monitor",
			  array("device" => $object));
		} else {
			// Update existing configuration object.
			$xpath = sprintf("//services/smart/monitor/device[uuid='%s']",
			  $object['uuid']);
			$success = $xmlConfig->replace($xpath, $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify the configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smart.monitor.device", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Get the extended S.M.A.R.T. information about the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file, e.g. /dev/sdb.
	 * @param context The context of the caller.
	 * @return The requested extended S.M.A.R.T. information as plain text.
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 * @throw E_MISC_FAILURE
	 */
	public function getExtendedInformation($params, $context) {
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
		// Get the S.M.A.R.T. information.
		$sd = OMVStorageDeviceFactory::get($params['devicefile']);
		if(is_null($sd) || !$sd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $params['devicefile']);
		}
		if(FALSE === $sd->hasSMART()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Device does not support S.M.A.R.T. (device=%s)",
			  $sd->getDeviceFile()));
		}
		if(is_null($smartInfo = $sd->getSMARTInformation())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to get S.M.A.R.T. interface (device=%s)",
			  $sd->getDeviceFile()));
		}
		if(FALSE === ($result = $smartInfo->getExtendedInformation())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to get S.M.A.R.T. extended information ".
			  "(device=%s): %s", $sd->getDeviceFile(),
			  $smartInfo->getLastError()));
		}
		return $result;
	}

	/**
	 * Get 'S.M.A.R.T. Attributes Data Structure' about the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file, e.g. /dev/sdb.
	 * @param context The context of the caller.
	 * @return The requested S.M.A.R.T. attributes.
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 * @throw E_MISC_FAILURE
	 */
	public function getAttributes($params, $context) {
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
		// Get the S.M.A.R.T. information.
		$sd = OMVStorageDeviceFactory::get($params['devicefile']);
		if(is_null($sd) || !$sd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $params['devicefile']);
		}
		if(FALSE === $sd->hasSMART()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Device does not support S.M.A.R.T. (device=%s)",
			  $sd->getDeviceFile()));
		}
		if(is_null($smartInfo = $sd->getSMARTInformation())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to get S.M.A.R.T. interface (device=%s)",
			  $sd->getDeviceFile()));
		}
		if(FALSE === ($result = $smartInfo->getAttributes())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to get S.M.A.R.T. attributes (device=%s): %s",
			  $sd->getDeviceFile(), $smartInfo->getLastError()));
		}
		return $result;
	}

	/**
	 * Get 'SMART Self-test log structure' about the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file, e.g. /dev/sdb.
	 * @param context The context of the caller.
	 * @return The requested Self-test logs.
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 * @throw E_MISC_FAILURE
	 */
	public function getSelfTestLogs($params, $context) {
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
		// Get the S.M.A.R.T. information.
		$sd = OMVStorageDeviceFactory::get($params['devicefile']);
		if(is_null($sd) || !$sd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $params['devicefile']);
		}
		if(FALSE === $sd->hasSMART()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Device does not support S.M.A.R.T. (device=%s)",
			  $sd->getDeviceFile()));
		}
		if(is_null($smartInfo = $sd->getSMARTInformation())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to get S.M.A.R.T. interface (device=%s)",
			  $sd->getDeviceFile()));
		}
		if(FALSE === ($result = $smartInfo->getSelfTestLogs())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to get S.M.A.R.T. self-test logs ".
			  "(device=%s): %s", $sd->getDeviceFile(),
			  $smartInfo->getLastError()));
		}
		return $result;
	}

	/**
	 * Get all S.M.A.R.T. identity information for the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file, e.g. /dev/sdb.
	 * @param context The context of the caller.
	 * @return The requested S.M.A.R.T. identity information.
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 */
	public function getInformation($params, $context) {
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
		// Get the S.M.A.R.T. information.
		$sd = OMVStorageDeviceFactory::get($params['devicefile']);
		if(is_null($sd) || !$sd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $params['devicefile']);
		}
		if(FALSE === $sd->hasSMART()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Device does not support S.M.A.R.T. (device=%s)",
			  $sd->getDeviceFile()));
		}
		if(is_null($smartInfo = $sd->getSMARTInformation())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to get S.M.A.R.T. interface (device=%s)",
			  $sd->getDeviceFile()));
		}
		if(FALSE === ($result = $smartInfo->getInformation())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to get S.M.A.R.T. information (device=%s): %s",
			  $sd->getDeviceFile(), $smartInfo->getLastError()));
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
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getScheduleList($params, $context) {
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
		// Get the configuration object list.
		$xpath = "//services/smart/scheduledtests/job";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional share informations.
		foreach($objects as $objectk => &$objectv) {
			// Modify result data.
			$objectv['enable'] = boolvalEx($objectv['enable']);
			// Append storage device information.
			$objectv['volume'] = array(
				"devicefile" => gettext("n/a"),
				"model" => gettext("n/a"),
				"size" => "-1" // as string
			);
			// Get the storage device object.
			$sd = OMVStorageDeviceFactory::get($objectv['devicefile']);
			if (!is_null($sd) && $sd->exists()) {
				// Update storage device information.
				$objectv['volume'] = array(
					"devicefile" => $sd->getDeviceFile(),
					"devicefilebyid" => $sd->getDeviceFileById(),
					"model" => $sd->getModel(),
					"size" => $sd->getSize()
				);
			}
		}
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a scheduled test configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getScheduledTest($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Get the configuration object.
		$xpath = sprintf("//services/smart/scheduledtests/job[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the result object.
		$object['enable'] = boolvalEx($object['enable']);
		return $object;
	}

	/**
	 * Set a scheduled test config object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 * @throw E_MISC_FAILURE
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	public function setScheduledTest($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
				  "enable":{"type":"boolean"},
				  "devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
				  "type":{"type":"string","enum":["S","L","C","O"]},
				  "hour":{"type":"string","format":"regex",'.
					'"pattern":"\/^0[0-9]|1[0-9]|2[0-3]|[*]$\/"},
				  "month":{"type":"string","format":"regex",'.
					'"pattern":"\/^0[1-9]|1[0-2]|[*]$\/"},
				  "dayofmonth":{"type":"string","format":"regex",'.
					'"pattern":"\/^0[1-9]|1[0-9]|2[0-9]|3[0-1]|[*]$\/"},
				  "dayofweek":{"type":"string","format":"regex",'.
					'"pattern":"\/^[1-7]|[*]$\/"},
				  "comment":{"type":"string"}
			  }
		  }');
		// Check if the given device exists.
		$sd = OMVStorageDeviceFactory::get($params['devicefile']);
		if(is_null($sd) || !$sd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $params['devicefile']);
		}
		// Check if the given device exists and supports S.M.A.R.T.
		if(FALSE === $sd->hasSMART()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Device does not support S.M.A.R.T. (device=%s)",
			  $sd->getDeviceFile()));
		}
		// Prepare configuration data
		$object = array(
			  "uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
				OMVUtil::uuid() : $params['uuid'],
			  "enable" => array_boolval($params, 'enable'),
			  "devicefile" => $sd->getDeviceFileById(),
			  "type" => $params['type'],
			  "month" => $params['month'],
			  "dayofmonth" => $params['dayofmonth'],
			  "dayofweek" => $params['dayofweek'],
			  "hour" => $params['hour'],
			  "comment" => $params['comment']
		  );
		// Set the configuration data.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration.
			$success = $xmlConfig->set("//services/smart/scheduledtests",
			  array("job" => $object));
		} else {
			// Update existing configuration object.
			$success = $xmlConfig->replace("//services/smart/scheduledtests/".
			  "job[uuid='{$params['uuid']}']", $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smart.scheduledtests.job", $object);
		// Return the configuration object.
		return $object;
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
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Delete the configuration object.
		$xpath = sprintf("//services/smart/scheduledtests/job[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.smart.scheduledtests.job");
	}

	/**
	 * Execute a scheduled test.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw E_CONFIG_DIRTY
	 */
	public function executeScheduledTest($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Create a background process.
		$bgStatusFilename = $this->createBgProcStatus();
		$pid = $this->fork();
		if($pid > 0) { // Parent process.
			$this->initializeBgProcStatus($bgStatusFilename, $pid);
			return $bgStatusFilename;
		}
		// Child process.
		try {
			$bgOutputFilename = $this->createBgProcOutput();
			$this->updateBgProcStatus($bgStatusFilename, "outputfilename",
			  $bgOutputFilename);
			// Get the scheduled test configuration object.
			$object = $this->callMethod("getScheduledTest", array(
			  "uuid" => $params['uuid']), $context);
			// Execute the command line in a shell.
			$test = array(
				  "S" => "short",
				  "L" => "long",
				  "C" => "conveyance",
				  "O" => "offline"
			  );
			$cmd = "/usr/sbin/smartctl";
			$cmdArgs = array("-t", $test[$object['type']],
			  $object['devicefile']);
			if(FALSE === $this->execve($cmd, $cmdArgs, $bgOutputFilename)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd." ".implode(" ", $cmdArgs), "");
			}
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceSmart());
?>
