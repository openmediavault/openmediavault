<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
require_once("openmediavault/functions.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class DiskMgmtRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getList" => array(
				'{"type":"integer"}', // start
				'{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}' // sortDir
			),
			"getHdParm" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"setHdParm" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"devicefile":{"type":["string","null"],"format":"regex",'.
					  '"pattern":"#^/dev/.+$#i"},
					"smart":{"type":"boolean"},
					"apm":{"type":"integer","minimum":1,"maximum":255},
					"aam":{"type":"integer","enum":[0,128,254]},
					"spindowntime":{"type":"integer","minimum":0,"maximum":254},
					"writecache":{"type":"boolean"}
				}
			}')
		);
	}

	/**
	 * Enumerate all disk devices on the system.
	 * @return An array containing physical disk device objects with following
	 * fields: devicename, devicefile, model, size, capacity, temperature,
	 * description, vendor, serialnumber and temperature.
	 */
	public function enumerateDevices() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get all existing hard disks devices
		if (FALSE === ($devs = OMVHardDiskDevices::get())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of hard disks devices");
		}
		// Get the root disk volume
		$rootVolume = preg_replace("/\d+/", "", OMVSystem::getRootPartition());
		// Prepare result
		$objects = array();
		foreach ($devs as $devk => $devv) {
			$hdd = new OMVHardDiskDevice($devv);
			if ($hdd->exists()) {
				// Skip physical volume where the OS is installed
				if ($rootVolume === $hdd->getDeviceFile())
					continue;
				$object = array(
					"devicename" => $hdd->getDeviceName(),
					"devicefile" => $hdd->getDeviceFile(),
					"model" => $hdd->getModel(),
					"size" => $hdd->getSize(),
					"capacity" => $hdd->getCapacity(),
					"temperature" => gettext("n/a"),
					"description" => $hdd->getDescription(),
					"vendor" => $hdd->getVendor(),
					"serialnumber" => $hdd->getSerialNumber()
				);
				if (FALSE !== ($temperature = $hdd->getTemperature())) {
					$object['temperature'] = sprintf("%d°C", $temperature);
				}
				$objects[] = $object;
			}
		}
		return $objects;
	}

	/**
	 * Enumerate all disk devices on the system. The field \em hdparm will be
	 * added to the hard disk objects if there exists additional hard disk
	 * parameters (e.g. S.M.A.R.T. or AAM) that can be defined individually
	 * per hard disk.
	 * @param start The index where to start.
	 * @param count The number of objects to process.
	 * @param sortField The name of the column used to sort.
	 * @param sortDir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 * contains the total number of objects, \em data contains the object
	 * array. An exception will be thrown in case of an error.
	 */
	public function getList($start, $count, $sortField, $sortDir) {
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
		// Enumerate all disk devices on the system.
		$devs = OMVRpc::exec("DiskMgmt", "enumerateDevices");
		foreach ($devs as $devk => &$devv) {
			// Check if there exists a hard disk parameter configuration
			// object for the given device.
			$xpath = sprintf("//system/storage/hdparm[devicefile='%s']/uuid",
			  $devv['devicefile']);
			$uuid = $xmlConfig->get($xpath);
			if (is_uuid($uuid)) {
				$devv['hdparm'] = OMVRpc::exec("DiskMgmt", "getHdParm",
				  array($uuid));
			}
		}
		// Filter result
		return $this->applyFilter($devs, $start, $count, $sortField, $sortDir);
	}

	/**
	 * Get a hard disk parameters config object
	 */
	public function getHdParm($uuid) {
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
		$xpath = "//system/storage/hdparm[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['smart'] = boolval($object['smart']);
		$object['apm'] = intval($object['apm']);
		$object['aam'] = intval($object['aam']);
		$object['spindowntime'] = intval($object['spindowntime']);
		$object['writecache'] = boolval($object['writecache']);
		return $object;
	}

	/**
	 * Set (add/update) a hard disk parameters config object
	 */
	public function setHdParm($data) {
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
		// Prepare configuration data
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"devicefile" => $data['devicefile'],
			"smart" => array_boolval($data, 'smart'),
			"apm" => $data['apm'],
			"aam" => $data['aam'],
			"spindowntime" => $data['spindowntime'],
			"writecache" => array_boolval($data, 'writecache')
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration
			$success = $xmlConfig->set("//system/storage",
			  array("hdparm" => $object));
		} else {
			// Update existing configuration object
			$success = $xmlConfig->replace(
			  "//system/storage/hdparm[uuid='{$data['uuid']}']",
			  $object);
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
		  "org.openmediavault.system.storage.hdparm", $object);
	}
}
?>
