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

class RaidMgmtRpc extends OMVRpc {
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
			"create" => array('{
				"type":"object",
				"properties":{
					"devices":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE_LIST'].'},
					"name":{"type":"string"},
					"level":{"type":"string","enum":["stripe","mirror",'.
					  '"linear","raid10","raid5","raid6"]}
				}
			}'),
			"grow" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
					"devices":{"type":"string"}
				}
			}'),
			"add" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
					"devices":{"type":"string"}
				}
			}'),
			"delete" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'}
				}
			}'),
			"getDetail" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'}
				}
			}')
		);
	}

	/**
	 * Enumerate all RAID devices on the system.
	 * @return An array containing RAID device objects with following fields:
	 *   name, devicefile, uuid, level, numdevices, devices, size, state,
	 *   description and _used.
	 */
	public function enumerateDevices() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get all existing software RAID devices.
		if (FALSE === ($devs = OMVStorageDevices::enumerate(
		  OMVStorageDevices::SOFTWARERAID))) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of software RAID devices");
		}
		// Get list of physical volumes to check whether a RAID device
		// is used as a physical volume.
		if (FALSE === ($pvs = OMVPhysicalVolumes::get())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to get ".
			  "list of physical volumes");
		}
		// Prepare result
		$result = array();
		foreach ($devs as $devk => $devv) {
			$raid = new OMVSoftwareRaidDevice($devv);
			if ($raid->exists()) {
				$used = FALSE;
				// Is this RAID device already used as a physical volume?
				if (in_array($raid->getDeviceFile(), $pvs)) {
					$used = TRUE;
				// Does the RAID device contain a filesystem and is it used?
				} else if (FALSE !== OMVRpc::exec("FsTab", "getByFsName",
				  array("id" => $raid->getDeviceFile()))) {
					$used = TRUE;
				}
				$result[] = array(
					"_used" => $used,
					"name" => $raid->getName(),
					"devicefile" => $raid->getDeviceFile(),
					"uuid" => $raid->getUuid(),
					"level" => $raid->getLevel(),
					"numdevices" => $raid->getNumDevices(),
					"devices" => $raid->getDevices(),
					"size" => $raid->getSize(),
					"state" => $raid->getState(),
					"description" => $raid->getDescription()
				);
			}
		}
		return $result;
	}

	/**
	 * Get list of available RAID devices.
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
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Enumerate all RAID devices on the system.
		$devs = OMVRpc::exec("RaidMgmt", "enumerateDevices");
		// Filter result
		return $this->applyFilter($devs, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get list of devices that can be used to create a RAID.
	 * @return An array containing objects with the following fields:
	 * devicefile, size and description.
	 */
	public function getCandidates() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$result = array();
		// Prepare list of used devices. Append device where the OS is
		// installed on.
		$rootVolume = preg_replace("/\d+/", "", OMVSystem::getRootPartition());
		$usedDevs = array($rootVolume);
		// Get all existing software RAID devices.
		if (FALSE === ($devs = OMVStorageDevices::enumerate(
		  OMVStorageDevices::SOFTWARERAID))) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of software RAID devices");
		}
		foreach ($devs as $devk => $devv) {
			$raid = new OMVSoftwareRaidDevice($devv);
			if ($raid->exists()) {
				$usedDevs = array_merge($usedDevs, $raid->getDevices());
			}
		}
		// Get all physical disks that are already used by a physical volume
		// and add them to the list of used physical disks.
		if (FALSE === ($devs = OMVPhysicalVolumes::get())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of physical volumes");
		}
		$usedDevs = array_merge($usedDevs, $devs);
		// Get all physical disk devices on the system.
		unset($devs);
		$devs = OMVRpc::exec("DiskMgmt", "enumerateDevices");
		// Prepare result
		foreach ($devs as $devk => $devv) {
			// Is this hard disk device used in a RAID device?
			if (in_array($devv['devicefile'], $usedDevs))
				continue;
			// Check if device is referenced/used by a plugin.
			if (TRUE === $xmlConfig->exists("//services/devicefile[".
			  "contains(.,'{$devv['devicefile']}')]"))
				continue;
			// Does this device already contain a filesystem?
			if (FALSE !== OMVFilesystem::hasFileSystem($devv['devicefile']))
				continue;
			$result[] = array(
				"devicefile" => $devv['devicefile'],
				"size" => $devv['size'],
				"description" => $devv['description']
			);
		}
		return $result;
	}

	/**
	 * Create a RAID device.
	 */
	public function create($data) {
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
		// Generate the RAID device name.
		$deviceName = OMVSystem::getNextDevice("disk", "md");
		$devices = preg_split("/[,;]/", $data['devices']);
		// Create the notification object.
		$object = array(
			"name" => $data['name'],
			"devicefile" => sprintf("/dev/%s", $deviceName),
			"level" => $data['level'],
			"numdevices" => count($devices),
			"devices" => $devices
		);
		// Create the RAID device.
		$cmdArgs = sprintf("%s -l %s -n %d", $object['devicefile'],
		  $object['level'], $object['numdevices']);
		if (!empty($object['name'])) {
			$cmdArgs .= sprintf(" -N %s", $object['name']);
		}
		$cmdArgs .= " " . implode(" ", $object['devices']);
		$cmd = sprintf("sudo omv-mkraid %s 2>&1", $cmdArgs);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.raid.device", $object);
	}

	/**
	 * Grow a RAID device.
	 */
	public function grow($data) {
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
		// Get software RAID device details to create notification message.
		$raid = new OMVSoftwareRaidDevice($data['devicefile']);
		if (FALSE === $raid->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Device '%s' not found", $raid->getDeviceFile()));
		}
		$numDevices = $raid->getNumDevices();
		$devices = $raid->getDevices();
		// Calculate new number of devices.
		$newDevices = explode(",", $data['devices']);
		$numDevices += count($newDevices);
		// Add new devices to the RAID device.
		$cmd = sprintf("sudo mdadm --manage %s --add %s 2>&1",
		  $raid->getDeviceFile(), implode(" ", $newDevices));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Now grow the RAID.
		// See https://raid.wiki.kernel.org/index.php/Growing
		$cmd = sprintf("sudo mdadm --grow --raid-devices=%d %s 2>&1",
		  $numDevices, $raid->getDeviceFile());
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Create the notification object.
		$object = array(
			"name" => $raid->getName(),
			"devicefile" => $raid->getDeviceFile(),
			"level" => $raid->getLevel(),
			"numdevices" => $numDevices,
			"devices" => array_merge($devices, $newDevices)
		);
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.raid.device", $object);
	}

	/**
	 * Add devices to a RAID device.
	 */
	public function add($data) {
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
		// Get software RAID device details to create notification message.
		$raid = new OMVSoftwareRaidDevice($data['devicefile']);
		if (FALSE === $raid->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Device '%s' not found", $raid->getDeviceFile()));
		}
		$numDevices = $raid->getNumDevices();
		$devices = $raid->getDevices();
		// Calculate new number of devices.
		$newDevices = explode(",", $data['devices']);
		$numDevices += count($newDevices);
		// Add new devices to the RAID device.
		$cmd = sprintf("sudo mdadm --manage %s --add %s 2>&1",
		  $raid->getDeviceFile(), implode(" ", $newDevices));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Create the notification object.
		$object = array(
			"name" => $raid->getName(),
			"devicefile" => $raid->getDeviceFile(),
			"level" => $raid->getLevel(),
			"numdevices" => $numDevices,
			"devices" => array_merge($devices, $newDevices)
		);
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.raid.device", $object);
	}

	/**
	 * Delete a RAID device.
	 * @param data An array containing the following fields:
	 *   \em devicefile The RAID device file to be deleted.
	 * @return None
	 */
	public function delete($data) {
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
		// Get software RAID device details to create notification message.
		$raid = new OMVSoftwareRaidDevice($data['devicefile']);
		if (FALSE === $raid->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Device '%s' not found", $raid->getDeviceFile()));
		}
		// Create the notification object.
		$object = array(
			"name" => $raid->getName(),
			"devicefile" => $raid->getDeviceFile(),
			"level" => $raid->getLevel(),
			"numdevices" => $raid->getNumDevices(),
			"devices" => $raid->getDevices()
		);
		// Delete the RAID device.
		$cmd = sprintf("sudo omv-rmraid %s", $raid->getDeviceFile());
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.raid.device", $object);
	}

	/**
	 * Get detail about a RAID device.
	 * @param data An array containing the following fields:
	 *   \em devicefile The RAID device file to get details from.
	 * @return An object with the requested details.
	 */
	public function getDetail($data) {
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
		// Get software RAID device details.
		$raid = new OMVSoftwareRaidDevice($data['devicefile']);
		if (FALSE === $raid->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Device '%s' not found", $raid->getDeviceFile()));
		}
		return $raid->getDetail();
	}
}
?>
