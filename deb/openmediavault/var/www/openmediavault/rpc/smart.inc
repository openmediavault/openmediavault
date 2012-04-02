<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class SmartRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"setSettings" => array('{
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
			}'),
			"getAttributes" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'}
				}
			}'),
			"getSelfTestLogs" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'}
				}
			}'),
			"getIdentityInfo" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'}
				}
			}'),
			"getScheduleList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"getScheduledTest" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"setScheduledTest" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
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
			}'),
			"deleteScheduledTest" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}')
		);
	}

	/**
	 * Enumerate all devices supporting S.M.A.R.T.
	 * @return An array containing physical disk device objects with following
	 *   fields: devicename, devicefile, model, size, capacity, temperature,
	 *   description, vendor and serialnumber.
	 */
	public function enumerateDevices() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get all existing hard disks devices.
		if (FALSE === ($devs = OMVStorageDevices::enumerate(
		  OMVStorageDevices::HARDDISK))) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of hard disks devices");
		}
		// Prepare result
		$objects = array();
		foreach ($devs as $devk => $devv) {
			$hdd = new OMVHardDiskDevice($devv);
			if ($hdd->exists()) {
				// Try to get the device temperature via S.M.A.R.T.
				$temperature = $hdd->getTemperature();
				// Prepare device object
				$objects[] = array(
					"devicename" => $hdd->getDeviceName(),
					"devicefile" => $hdd->getDeviceFile(),
					"devicefilebyid" => $hdd->getDeviceFileById(),
					"model" => $hdd->getModel(),
					"size" => $hdd->getSize(),
					"capacity" => $hdd->getCapacity(),
					"temperature" => (FALSE === $temperature) ?
					  gettext("n/a") : sprintf("%d°C", $temperature),
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
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getList($data) {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Enumerate all devices supporting S.M.A.R.T.
		$objects = OMVRpc::exec("Smart", "enumerateDevices");
		// Filter result
		return $this->applyFilter($objects, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	public function getSettings() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/smart";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['interval'] = intval($object['interval']);
		$object['tempdiff'] = intval($object['tempdiff']);
		$object['tempinfo'] = intval($object['tempinfo']);
		$object['tempcrit'] = intval($object['tempcrit']);
		return $object;
	}

	public function setSettings($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get existing configuration object
		$xpath = "//services/smart";
		$oldObject = $xmlConfig->get($xpath);
		if (is_null($oldObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update configuration
		$object = array(
			"enable" => array_boolval($data, 'enable'),
			"interval" => $data['interval'],
			"powermode" => $data['powermode'],
			"tempdiff" => $data['tempdiff'],
			"tempinfo" => $data['tempinfo'],
			"tempcrit" => $data['tempcrit'],
			"scheduledtests" => $oldObject['scheduledtests']
		);
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smart", $object);
	}

	/**
	 * Get 'S.M.A.R.T. Attributes Data Structure' for the given device.
	 * @param data An array containing the following fields:
	 *   \em devicefile The device file.
	 * @return The requested S.M.A.R.T. attributes.
	 */
	public function getAttributes($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get S.M.A.R.T. statistics
		$hdd = new OMVHardDiskDevice($data['devicefile']);
		if (FALSE === $hdd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Device '%s' not found", $hdd->getDeviceFile()));
		}
		if (FALSE === ($stats = $hdd->getSmartStats())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to get S.M.A.R.T. statistics (device=%s)",
			  $hdd->getDeviceFile()));
		}
		return $stats['attributes'];
	}

	/**
	 * Get 'SMART Self-test log structure' for the given device.
	 * @param data An array containing the following fields:
	 *   \em devicefile The device file.
	 * @return The requested Self-test logs.
	 */
	public function getSelfTestLogs($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get S.M.A.R.T. statistics
		$hdd = new OMVHardDiskDevice($data['devicefile']);
		if (FALSE === $hdd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Device '%s' not found", $hdd->getDeviceFile()));
		}
		if (FALSE === ($stats = $hdd->getSmartStats())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to get S.M.A.R.T. statistics (device=%s)",
			  $hdd->getDeviceFile()));
		}
		return $stats['selftestlogs'];
	}

	/**
	 * Get all S.M.A.R.T. identity information for the given device.
	 * @param data An array containing the following fields:
	 *   \em devicefile The device file.
	 * @return The requested S.M.A.R.T. identity information.
	 */
	public function getIdentityInfo($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get S.M.A.R.T. identity informations
		$hdd = new OMVHardDiskDevice($data['devicefile']);
		if (FALSE === $hdd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Device '%s' not found", $hdd->getDeviceFile()));
		}
		return array(
			"devicemodel" => $hdd->getModel(),
			"serialnumber" => $hdd->getSerialNumber(),
			"firmwareversion" => $hdd->getFirmwareVersion()
		);
	}

	/**
	 * Get list of schedule configuration objects.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getScheduleList($data) {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object list
		$xpath = "//services/smart/scheduledtests/job";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional share informations
		foreach ($objects as $objectk => &$objectv) {
			$objectv['volume'] = array(
				"devicefile" => gettext("n/a"),
				"model" => gettext("n/a"),
				"size" => gettext("n/a"),
				"capacity" => gettext("n/a")
			);
			$hdd = new OMVHardDiskDevice($objectv['devicefile']);
			if ($hdd->exists()) {
				$objectv['volume'] = array(
					"devicefile" => $hdd->getDeviceFile(),
					"devicefilebyid" => $hdd->getDeviceFileById(),
					"model" => $hdd->getModel(),
					"size" => $hdd->getSize(),
					"capacity" => $hdd->getCapacity()
				);
			}
		}
		// Filter result
		return $this->applyFilter($objects, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get a scheduled test configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The requested configuration object.
	 */
	public function getScheduledTest($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath = sprintf("//services/smart/scheduledtests/job[uuid='%s']",
		  $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return $object;
	}

	/**
	 * Set a scheduled test config object.
	 */
	public function setScheduledTest($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Check if the given device exists.
		$sd = OMVStorageDeviceFactory::get($data['devicefile']);
		if (is_null($sd) || !$sd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("Device '%s' not found"), $data['devicefile']));
		}
		// Prepare configuration data
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"devicefile" => $sd->getDeviceFileById(),
			"type" => $data['type'],
			"month" => $data['month'],
			"dayofmonth" => $data['dayofmonth'],
			"dayofweek" => $data['dayofweek'],
			"hour" => $data['hour'],
			"comment" => $data['comment']
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration
			$success = $xmlConfig->set("//services/smart/scheduledtests",
			  array("job" => $object));
		} else {
			// Update existing configuration object
			$success = $xmlConfig->replace("//services/smart/scheduledtests/" .
			  "job[uuid='{$data['uuid']}']", $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.smart.scheduledtests.job", $object);
	}

	/**
	 * Delete a scheduled test configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return None
	 */
	public function deleteScheduledTest($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Delete configuration object
		$xpath = sprintf("//services/smart/scheduledtests/job[uuid='%s']",
		  $data['uuid']);
		if (FALSE === ($object = $xmlConfig->delete($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
			  $xpath);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.smart.scheduledtests.job", $object);
	}
}
?>
