<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
		$this->registerMethod("getList");
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
		$this->registerMethod("getAttributes");
		$this->registerMethod("getSelfTestLogs");
		$this->registerMethod("getIdentityInfo");
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
	 * @return An array containing physical disk device objects with
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
		  OMV_STORAGE_DEVICE_TYPE_HDD))) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  "Failed to get list of hard disks devices");
		}
		// Prepare result
		$objects = array();
		foreach($devs as $devk => $devv) {
			$hdd = new OMVHardDiskDevice($devv);
			if($hdd->exists()) {
				// Try to get the device temperature via S.M.A.R.T.
				$temperature = $hdd->getTemperature();
				// Prepare device object
				$objects[] = array(
					  "devicename" => $hdd->getDeviceName(),
					  "devicefile" => $hdd->getDeviceFile(),
					  "devicefilebyid" => $hdd->getDeviceFileById(),
					  "model" => $hdd->getModel(),
					  "size" => $hdd->getSize(),
					  "temperature" => (FALSE === $temperature) ?
						"" : sprintf("%d°C", $temperature),
					  "description" => $hdd->getDescription(),
					  "vendor" => $hdd->getVendor(),
					  "serialnumber" => $hdd->getSerialNumber()
				  );
			}
		}
		return $objects;
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
		$object['enable'] = boolval($object['enable']);
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
	 * Get 'S.M.A.R.T. Attributes Data Structure' for the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file.
	 * @param context The context of the caller.
	 * @return The requested S.M.A.R.T. attributes.
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 * @throw E_MISC_FAILURE
	 */
	public function getAttributes($params, $context) {
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
		// Get the S.M.A.R.T. statistics.
		$hdd = new OMVHardDiskDevice($params['devicefile']);
		if(FALSE === $hdd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $hdd->getDeviceFile());
		}
		if(FALSE === ($stats = $hdd->getSmartStats())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to get S.M.A.R.T. statistics (device=%s)",
			  $hdd->getDeviceFile()));
		}
		return $stats['attributes'];
	}

	/**
	 * Get 'SMART Self-test log structure' for the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file.
	 * @param context The context of the caller.
	 * @return The requested Self-test logs.
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 * @throw E_MISC_FAILURE
	 */
	public function getSelfTestLogs($params, $context) {
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
		// Get the S.M.A.R.T. statistics.
		$hdd = new OMVHardDiskDevice($params['devicefile']);
		if(FALSE === $hdd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $hdd->getDeviceFile());
		}
		if(FALSE === ($stats = $hdd->getSmartStats())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to get S.M.A.R.T. statistics (device=%s)",
			  $hdd->getDeviceFile()));
		}
		return $stats['selftestlogs'];
	}

	/**
	 * Get all S.M.A.R.T. identity information for the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file.
	 * @param context The context of the caller.
	 * @return The requested S.M.A.R.T. identity information.
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 */
	public function getIdentityInfo($params, $context) {
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
		// Get the S.M.A.R.T. identity informations.
		$hdd = new OMVHardDiskDevice($params['devicefile']);
		if(FALSE === $hdd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $hdd->getDeviceFile());
		}
		return array(
			"devicemodel" => $hdd->getModel(),
			"serialnumber" => $hdd->getSerialNumber(),
			"firmwareversion" => $hdd->getFirmwareVersion()
		);
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
			$objectv['enable'] = boolval($objectv['enable']);
			// Append data.
			$objectv['volume'] = array(
				"devicefile" => gettext("n/a"),
				"model" => gettext("n/a"),
				"size" => "-1" // as string
			);
			$hdd = new OMVHardDiskDevice($objectv['devicefile']);
			if($hdd->exists()) {
				$objectv['volume'] = array(
					"devicefile" => $hdd->getDeviceFile(),
					"devicefilebyid" => $hdd->getDeviceFileById(),
					"model" => $hdd->getModel(),
					"size" => $hdd->getSize()
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
		$object['enable'] = boolval($object['enable']);
		return $object;
	}

	/**
	 * Set a scheduled test config object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_MISC_DEVICE_NOT_FOUND
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
