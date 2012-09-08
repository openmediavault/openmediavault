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
require_once("openmediavault/functions.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class DiskMgmtRpc extends OMVRpc {
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
			"getHdParm" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"setHdParm" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"devicefile":{"type":["string","null"],"format":"regex",'.
					  '"pattern":"#^/dev(/disk/by-id)?/.+$#i"},
					"smart":{"type":"boolean"},
					"apm":{"type":"integer","minimum":1,"maximum":255},
					"aam":{"type":"integer","enum":[0,128,254]},
					"spindowntime":{"type":"integer","minimum":0,"maximum":254},
					"writecache":{"type":"boolean"}
				}
			}'),
			"wipe" => array('{
				"type":"object",
				"properties":{
					"devicefile":{"type":"string","format":"regex",'.
					  '"pattern":"#^/dev/.+$#i"},
					"secure":{"type":"boolean"}
				}
			}')
		);
	}

	/**
	 * Enumerate all disk devices on the system.
	 * @return An array containing physical disk device objects with the
	 *   fields \em devicename, \em devicefile, \em model, \em size,
	 *   \em description, \em vendor, \em serialnumber, \em israid and
	 *   \em isrootdevice.
	 */
	public function enumerateDevices() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get the device containing the operating system. Mark it as read-only
		// to deny wiping this device.
		$rootDevice = OMVSystem::getRootPartition();
		// Get all existing devices except software RAID devices.
		if (FALSE === ($devs = OMVStorageDevices::enumerate(
		  OMVStorageDevices::ALL & ~OMVStorageDevices::SOFTWARERAID))) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of hard disks and hardware RAID devices");
		}
		// Prepare result
		$objects = array();
		foreach ($devs as $devk => $devv) {
			$object = OMVStorageDeviceFactory::get($devv);
			if (!is_null($object) && $object->exists()) {
				// Prepare device object
				$objects[] = array(
					"devicename" => $object->getDeviceName(),
					"devicefile" => $object->getDeviceFile(),
					"devicefilebyid" => $object->getDeviceFileById(),
					"model" => $object->getModel(),
					"size" => $object->getSize(),
					"description" => $object->getDescription(),
					"vendor" => $object->getVendor(),
					"serialnumber" => $object->getSerialNumber(),
					"israid" => $object->isRaid(),
					"isrootdevice" => (0 === strpos($rootDevice,
					  $object->getDeviceFile())) ? TRUE : FALSE
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
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Enumerate all disk devices on the system.
		$objects = OMVRpc::exec("DiskMgmt", "enumerateDevices");
		foreach($objects as $objectk => &$objectv) {
			// Check if there exists a hdparm configuration object for the
			// given device. Note, we have to check for 'dev/xxx' and
			// 'dev/disk/by-id/xxx' entries.
			$xpath = sprintf("//system/storage/hdparm[devicefile='%s']/uuid",
			  $objectv['devicefilebyid']);
			$uuid = $xmlConfig->get($xpath);
			if(!is_uuid($uuid)) {
				$xpath = sprintf("//system/storage/hdparm[devicefile='%s']".
				  "/uuid", $objectv['devicefile']);
				$uuid = $xmlConfig->get($xpath);
			}
			// If found, then finally get the hdparm configuration object.
			if(is_uuid($uuid)) {
				$objectv['hdparm'] = OMVRpc::exec("DiskMgmt", "getHdParm",
				  array("uuid" => $uuid));
			}
		}
		// Filter result
		return $this->applyFilter($objects, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get a hard disk parameters config object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The requested configuration object.
	 */
	public function getHdParm($data) {
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
		$xpath = sprintf("//system/storage/hdparm[uuid='%s']", $data['uuid']);
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
			"smart" => array_boolval($data, 'smart'),
			"apm" => $data['apm'],
			"aam" => $data['aam'],
			"spindowntime" => $data['spindowntime'],
			"writecache" => array_boolval($data, 'writecache')
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness
			$xpath = sprintf("//system/storage/hdparm[devicefile='%s']",
			  $object['devicefile']);
			if (TRUE === $xmlConfig->exists($xpath)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				  sprintf(gettext("Settings already exist for device '%s'"),
				  $object['devicefile']));
			}
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

	/**
	 * Wipe the given device.
	 * @param data An array containing the following fields:
	 *   \em devicefile The device file to wipe, e.g. /dev/sdb.
	 *   \em secure Set to TRUE to secure wipe the device.
	 * @return The process id of the executed command.
	 */
	public function wipe($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Wipe the device.
		$outputFile = tempnam(sys_get_temp_dir(), "format");
		if (TRUE === boolval($data['secure'])) {
			// Secure wipe the device.
			$cmd = sprintf("export LANG=C; sudo shred -v -n 1 %s ".
			  ">%s 2>&1 & echo $!", $data['devicefile'], $outputFile);
		} else {
			// Quick wipe the device (see omv-initfs).
			$cmd = sprintf("export LANG=C; sudo sgdisk --zap-all %s ".
			  ">%s 2>&1 & echo $!", $data['devicefile'], $outputFile);
		}
		OMVUtil::exec($cmd, $output, $res);
		if ($res !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Remember pid and content filename in session
		$pid = intval($output[0]);
		$_SESSION['execjob'][$pid] = array(
			"pid" => $pid,
			"outputFile" => $outputFile
		);
		return $pid;
	}

	/**
	 * Rescan SCSI bus.
	 */
	public function rescan() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Execute shell script to rescan SCSI bus.
		$cmd = "[ -x /sys/class/scsi_host ] && for hostdir in ".
		  "/sys/class/scsi_host/host*; do echo \"- - -\" | ".
		  "sudo tee \${hostdir}/scan >/dev/null; done";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}
}
?>
