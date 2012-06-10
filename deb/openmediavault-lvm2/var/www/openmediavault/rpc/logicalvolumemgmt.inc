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
require_once("openmediavault/error.inc");
require_once("openmediavault/system.inc");
require_once("openmediavault/lvm.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class LogicalVolumeMgmtRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getPhysicalVolumesList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"createPhysicalVolume" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'}
				}
			}'),
			"deletePhysicalVolume" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'}
				}
			}'),
			"getVolumeGroupsList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"getVolumeGroupPhysicalVolumes" => array('{
				"type":"object",
				"properties":{
					"name":{"type":"string"}
				}
			}'),
			"createVolumeGroup" => array('{
				"type":"object",
				"properties":{
					"name":{"type":"string"},
					"devices":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE_LIST'].'}
				}
			}'),
			"getVolumeGroup" => array('{
				"type":"object",
				"properties":{
					"name":{"type":"string"}
				}
			}'),
			"deleteVolumeGroup" => array('{
				"type":"object",
				"properties":{
					"name":{"type":"string"}
				}
			}'),
			"renameVolumeGroup" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
					"name":{"type":"string"}
				}
			}'),
			"extendVolumeGroup" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
					"devices":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE_LIST'].'}
				}
			}'),
			"reduceVolumeGroup" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
					"devices":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE_LIST'].'}
				}
			}'),
			"getLogicalVolumesList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"deleteLogicalVolume" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'}
				}
			}'),
			"createLogicalVolume" => array('{
				"type":"object",
				"properties":{
					"name":{"type":"string"},
					"size":{"type":"number"},
					"unit":{"type":"string","enum":["MiB","GiB","TiB"]},
					"vgname":{"type":"string"}
				}
			}'),
			"renameLogicalVolume" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
					"name":{"type":"string"}
				}
			}'),
			"modifyLogicalVolume" => array('{
				"type":"object",
				"properties":{
					"devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
					"size":{"type":"number"},
					"unit":{"type":"string","enum":["MiB","GiB","TiB"]}
				}
			}')
		);
	}

	/**
	 * Enumerate all physical volumes on the system.
	 * @return An array of objects. Each object represents a physical volume
	 *   with the following properties: devicename, free, used, size, vguuid,
	 *   vgname and description.
	 */
	public function enumeratePhysicalVolumes() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Enumerate all physical volumes on the system.
		if (FALSE === ($devs = OMVPhysicalVolumes::get())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of physical volumes");
		}
		$result = array();
		foreach ($devs as $devk => $devv) {
			// Get the physical volume details.
			$pv = new OMVPhysicalVolume($devv);
			$result[] = array(
				"devicefile" => $pv->getDeviceFile(),
				"free" => $pv->getFree(),
				"used" => $pv->getUsed(),
				"size" => $pv->getSize(),
				"vguuid" => $pv->getVGUuid(),
				"vgname" => $pv->getVGName(),
				"description" => $pv->getDescription()
			);
		}
		return $result;
	}

	/**
	 * Get a list of physical volumes.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	function getPhysicalVolumesList($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Enumerate all physical volumes on the system.
		$devs = $this->enumeratePhysicalVolumes();
		foreach ($devs as $devk => &$devv) {
			$devv['_used'] = !empty($devv['vgname']);
		}
		// Filter result
		return $this->applyFilter($devs, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get list of devices that can be used to create a physical volume.
	 * @return An array containing objects with the following fields:
	 *   devicefile, size and description.
	 */
	public function getPhysicalVolumeCandidates() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$result = array();
		// Prepare list of used devices
		$usedDevs = array();
		// Get all physical disks that are already used by a physical volume
		// and add them to the list of used physical disks.
		if (FALSE === ($devs = OMVPhysicalVolumes::get())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of physical volumes");
		}
		$usedDevs = array_merge($usedDevs, $devs);
		// Get all existing software RAID devices.
		if (FALSE === ($devs = OMVStorageDevices::enumerate(
		  OMVStorageDevices::SOFTWARERAID))) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of software RAID devices");
		}
		foreach ($devs as $devk => $devv) {
			$raid = new OMVSoftwareRaidDevice($devv);
			if (!$raid->exists())
				continue;
 			// Add the devices used by the RAID to the list of used devices.
			$usedDevs = array_merge($usedDevs, $raid->getDevices());
			// Is this RAID device already used as a physical volume?
			if (in_array($raid->getDeviceFile(), $usedDevs))
				continue;
			// Does this RAID already contain a filesystem?
			if (FALSE !== OMVFilesystem::hasFileSystem($raid->getDeviceFile()))
				continue;
			$result[] = array(
				"devicefile" => $raid->getDeviceFile(),
				"size" => $raid->getSize(),
				"description" => $raid->getDescription()
			);
		}

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
	 * Create a physical volume.
	 */
	public function createPhysicalVolume($data) {
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
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.lvm.physicalvolume", array(
			  "devicefile" => $data['devicefile']
		  ));
	}

	/**
	 * Delete a physical volume.
	 * @param data An array containing the following fields:
	 *   \em devicefile The devicefile of the physical volume.
	 * @return None
	 */
	public function deletePhysicalVolume($data) {
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
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.lvm.physicalvolume", array(
			  "devicefile" => $data['devicefile']
		  ));
	}

	/**
	 * Enumerate all volume groups on the system.
	 * @return An array of objects. Each object represents a volume group
	 *   with the following properties: devicefile, uuid, name, free, size,
	 *   pvName and lvName.
	 */
	public function enumerateVolumeGroups() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Enumerate all volume groups on the system.
		if (FALSE === ($vgs = OMVVolumeGroups::get())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of volume groups");
		}
		$result = array();
		foreach ($vgs as $vgk => $vgv) {
			// Get the volume group details.
			$vg = new OMVVolumeGroup($vgv);
			$result[] = array(
				"devicefile" => $vg->getDeviceFile(),
				"uuid" => $vg->getUuid(),
				"name" => $vg->getName(),
				"free" => $vg->getFree(),
				"size" => $vg->getSize(),
				"pvName" => $vg->getPVName(),
				"lvName" => $vg->getLVName(),
				"description" => $vg->getDescription()
			);
		}
		return $result;
	}

	/**
	 * Get a list of volume groups.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	function getVolumeGroupsList($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Enumerate all volume groups on the system.
		$vgs = $this->enumerateVolumeGroups();
		foreach ($vgs as $vgk => &$vgv) {
			$vgv['_used'] = !empty($vgv['lvName']);
		}
		// Filter result
		return $this->applyFilter($vgs, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get list of physical volumes that can be used to create a volume group.
	 * @return An array containing objects with the following fields:
	 *   devicefile, size and description.
	 */
	public function getVolumeGroupCandidates() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$result = array();
		// Prepare list of used devices
		$usedDevs = array();
		// Get volume groups to get the used physical volumes.
		if (FALSE === ($vgs = OMVVolumeGroups::get())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of volume groups");
		}
		foreach ($vgs as $vgk => $vgv) {
			$vg = new OMVVolumeGroup($vgv);
			// Add the physical volumes assigned to the volume group to the
			// list of already used physical volumes.
			$usedDevs = array_merge($usedDevs, $vg->getPVName());
		}
		// Get all physical physical volumes.
		if (FALSE === ($pvs = OMVPhysicalVolumes::get())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of physical volumes");
		}
		foreach ($pvs as $pvk => $pvv) {
			$pv = new OMVPhysicalVolume($pvv);
			// Is this physical volume already used?
			if (in_array($pv->getDeviceFile(), $usedDevs))
				continue;
			$result[] = array(
				"devicefile" => $pv->getDeviceFile(),
				"size" => $pv->getSize(),
				"description" => $pv->getDescription()
			);
		}
		return $result;
	}

	/**
	 * Create a volume group.
	 */
	public function createVolumeGroup($data) {
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
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.lvm.volumegroup", array(
			  "name" => $data['name'],
			  "devices" => preg_split("/[,;]/", $data['devices'])
		  ));
	}

	/**
	 * Get volume group details.
	 * @param data An array containing the following fields:
	 *   \em uuid name The name of the volume group, e.g. vg0 or /dev/vg1.
	 * @return The volume group details.
	 */
	public function getVolumeGroup($data) {
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
		// Get the volume group details.
		$vg = new OMVVolumeGroup($data['name']);
		if (FALSE === $vg->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Volume group '%s' not found", $vg->getName()));
		}
		return array(
			"devicefile" => $vg->getDeviceFile(),
			"uuid" => $vg->getUuid(),
			"name" => $vg->getName(),
			"free" => $vg->getFree(),
			"size" => $vg->getSize(),
			"pvName" => $vg->getPVName(),
			"lvName" => $vg->getLVName(),
			"description" => $vg->getDescription()
		);
	}

	/**
	 * Delete a volume group.
	 * @param data An array containing the following fields:
	 *   \em name The name of the volume group, e.g. vg0 or /dev/vg1.
	 * @return None
	 */
	public function deleteVolumeGroup($data) {
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
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.lvm.volumegroup", array(
			  "name" => $data['name']
		  ));
	}

	/**
	 * Get the physical volumes assigned to a volume group.
	 * @param data An array containing the following fields:
	 *   \em name The name of the volume group, e.g. vg0 or /dev/vg1.
	 * @return An array containing the physical volumes assigned to the
	 *   given volume group. Each array item has the fields \em devicefile,
	 *   \em size, \em description and \em _used.
	 */
	public function getVolumeGroupPhysicalVolumes($data) {
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
		// Get the volume groups physical volumes.
		$vg = new OMVVolumeGroup($data['name']);
		$result = array();
		foreach ($vg->getPVName() as $pvk => $pvv) {
			$pv = new OMVPhysicalVolume($pvv);
			$result[] = array(
				"devicefile" => $pv->getDeviceFile(),
				"size" => $pv->getSize(),
				"description" => $pv->getDescription(),
				"_used" => ($pv->getAllocatedPhysicalExtents() > 0) ?
				  TRUE : FALSE
			);
		}
		return $result;
	}

	/**
	 * Rename a volume group.
	 */
	public function renameVolumeGroup($data) {
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
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.lvm.volumegroup", array(
			  "devicefile" => $data['devicefile'],
			  "name" => $data['name']
		  ));
	}

	/**
	 * Extend a volume group.
	 */
	public function extendVolumeGroup($data) {
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
		// Get the physical volumes assigned to the volume group.
		$vg = new OMVVolumeGroup($data['devicefile']);
		if (FALSE === ($pvs = $vg->getPVName())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of physical volumes");
		}
		// Compose the new list of devices.
		$devices = array_merge(preg_split("/[,;]/", $data['devices']), $pvs);
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.lvm.volumegroup", array(
			  "devicefile" => $data['devicefile'],
			  "devices" => $devices
		  ));
	}

	/**
	 * Reduce a volume group.
	 */
	public function reduceVolumeGroup($data) {
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
		// Get the physical volumes assigned to the volume group.
		$vg = new OMVVolumeGroup($data['devicefile']);
		if (FALSE === ($pvs = $vg->getPVName())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of physical volumes");
		}
		// Compose the new list of devices.
		$devices = array_diff($pvs, preg_split("/[,;]/", $data['devices']));
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.lvm.volumegroup", array(
			  "devicefile" => $data['devicefile'],
			  "devices" => $devices
		  ));
	}

	/**
	 * Enumerate all logical volumes on the system.
	 * @return An array of objects. Each object represents a logical volume
	 *   with the following properties: devicefile, uuid, name, size and
	 *   vgname.
	 */
	public function enumerateLogicalVolumes() {
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Enumerate all volume groups on the system.
		if (FALSE === ($lvs = OMVLogicalVolumes::get())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to " .
			  "get list of logical volumes");
		}
		$result = array();
		foreach ($lvs as $lvk => $lvv) {
			// Get the physical volume details.
			$lv = new OMVLogicalVolume($lvv);
			if ($lv->exists()) {
				$result[] = array(
					"devicefile" => $lv->getDeviceFile(),
					"uuid" => $lv->getUuid(),
					"name" => $lv->getName(),
					"size" => $lv->getSize(),
					"vgname" => $lv->getVGName()
				);
			}
		}
		return $result;
	}

	/**
	 * Get a list of logical volumes.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	function getLogicalVolumesList($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Enumerate all volume groups on the system.
		$lvs = $this->enumerateLogicalVolumes();
		foreach ($lvs as $lvk => &$lvv) {
			$used = FALSE;
			// Does the logical volume contain a filesystem and is it used?
			if (FALSE !== OMVRpc::exec("FsTab", "getByFsName", array(
			  "id" => $lvv['devicefile']))) {
				$used = TRUE;
			}
			$lvv['_used'] = $used;
		}
		// Filter result
		return $this->applyFilter($lvs, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Create a logical volume.
	 */
	public function createLogicalVolume($data) {
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
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.lvm.logicalvolume", array(
			  "name" => $data['name'],
			  "size" => $data['size'],
			  "unit" => $data['unit'],
			  "vgname" => $data['vgname']
		  ));
	}

	/**
	 * Rename a logical volume.
	 */
	public function renameLogicalVolume($data) {
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
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.lvm.logicalvolume", array(
			  "devicefile" => $data['devicefile'],
			  "name" => $data['name']
		  ));
	}

	/**
	 * Delete a logical volume.
	 * @param data An array containing the following fields:
	 *   \em devicefile The device file of the logical volume.
	 * @return None
	 */
	public function deleteLogicalVolume($data) {
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
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.lvm.logicalvolume", array(
			  "devicefile" => $data['devicefile']
		  ));
	}

	/**
	 * Modify a logical volume.
	 * @param data An array containing the following fields:
	 *   \em devicefile The device file of the logical volume to modify.
	 *   \em size The new absolute size.
	 *   \em unit The binary unit of the new volume size.
	 * @return None
	 */
	public function modifyLogicalVolume($data) {
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
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.lvm.logicalvolume", array(
			  "devicefile" => $data['devicefile'],
			  "size" => $data['size'],
			  "unit" => $data['unit']
		  ));
	}
}
?>
