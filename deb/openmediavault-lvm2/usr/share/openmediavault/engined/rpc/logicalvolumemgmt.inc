<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceLogicalVolumeMgmt extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "LogicalVolumeMgmt";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("enumeratePhysicalVolumes");
		$this->registerMethod("getPhysicalVolumesList");
		$this->registerMethod("getPhysicalVolumeCandidates");
		$this->registerMethod("createPhysicalVolume");
		$this->registerMethod("deletePhysicalVolume");
		$this->registerMethod("resizePhysicalVolume");
		$this->registerMethod("enumerateVolumeGroups");
		$this->registerMethod("getVolumeGroupsList");
		$this->registerMethod("getVolumeGroupCandidates");
		$this->registerMethod("createVolumeGroup");
		$this->registerMethod("getVolumeGroup");
		$this->registerMethod("deleteVolumeGroup");
		$this->registerMethod("getVolumeGroupPhysicalVolumes");
		$this->registerMethod("renameVolumeGroup");
		$this->registerMethod("extendVolumeGroup");
		$this->registerMethod("reduceVolumeGroup");
		$this->registerMethod("enumerateLogicalVolumes");
		$this->registerMethod("getLogicalVolumesList");
		$this->registerMethod("getLogicalVolumeCandidates");
		$this->registerMethod("createLogicalVolume");
		$this->registerMethod("renameLogicalVolume");
		$this->registerMethod("deleteLogicalVolume");
		$this->registerMethod("modifyLogicalVolume");
	}

	/**
	 * Enumerate all physical volumes on the system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects. Each object represents a physical volume
	 *   with the following properties: devicename, free, used, size, vguuid,
	 *   vgname and description.
	 * @throw E_EXEC_MISC
	 */
	public function enumeratePhysicalVolumes($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Enumerate all physical volumes on the system.
		if (FALSE === ($devs = OMVLvmPhysicalVolumes::enumerate())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  "Failed to get list of physical volumes");
		}
		$result = array();
		foreach($devs as $devk => $devv) {
			// Get the physical volume details.
			$pv = new OMVLvmPhysicalVolume($devv);
			if(!$pv->exists())
				continue;
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
	function getPhysicalVolumesList($params, $context) {
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
		// Enumerate all physical volumes on the system.
		$pvs = $this->callMethod("enumeratePhysicalVolumes", NULL, $context);
		foreach($pvs as $pvk => &$pvv) {
			$pvv['_used'] = !empty($pvv['vgname']);
		}
		// Filter result.
		return $this->applyFilter($pvs, $params['start'], $params['limit'],
		  $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get list of devices that can be used to create a physical volume.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing objects with the following fields:
	 *   devicefile, size and description.
	 * @throw E_EXEC_MISC
	 */
	public function getPhysicalVolumeCandidates($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get a list of all potential usable devices.
		if(FALSE === ($devs = OMVStorageDevices::enumerateUnused())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  "Failed to get list of devices");
		}
		// Prepare the result list.
		$result = array();
		foreach($devs as $devk => $devv) {
			// Get the object that implements the given storage device.
			$sd = OMVStorageDeviceFactory::get($devv);
			if(is_null($sd) || !$sd->exists())
				continue;
			// Check if device is referenced/used by a plugin.
			$xpath = sprintf("//services/devicefile[contains(.,'%s')]",
			  $sd->getDeviceFile());
			if(TRUE === $xmlConfig->exists($xpath))
				continue;
			// Does this device already contain a filesystem?
			if(FALSE !== OMVFilesystem::hasFileSystem($sd->getDeviceFile()))
				continue;
			// The device is a potential candidate to be used as a
			// physical volume.
			$result[] = array(
				"devicefile" => $sd->getDeviceFile(),
				"size" => $sd->getSize(),
				"description" => $sd->getDescription()
			);
		}
		return $result;
	}

	/**
	 * Create a physical volume.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_FAILED
	 * @throw E_EXEC_MISC
	 */
	public function createPhysicalVolume($params, $context) {
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
		// Make sure the disk device is clean, no partition table
		// should exist, otherwise pvcreate fails.
		$cmd = sprintf("dd if=/dev/zero of=%s bs=512 count=1 2>&1",
		  $params['devicefile']);
		if (0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Create the physical volume.
		$pv = new OMVLvmPhysicalVolume($params['devicefile']);
		if (!$pv->create()) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
			  gettext("Failed to create physical volume: %s"),
			  $pv->getLastError()));
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.lvm.physicalvolume", array(
			  "devicefile" => $params['devicefile']
		  ));
	}

	/**
	 * Delete a physical volume.
	 * @param params An array containing the following fields:
	 *   \em devicefile The devicefile of the physical volume.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 * @throw E_EXEC_MISC
	 */
	public function deletePhysicalVolume($params, $context) {
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
		// Check if the physical volume exists.
		$pv = new OMVLvmPhysicalVolume($params['devicefile']);
		if (!$pv->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("No physical volume found on '%s'"),
			  $params['devicefile']));
		}
		// Remove the physical volume.
		if (!$pv->remove()) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
			  gettext("Failed to remove physical volume: %s"),
			  $pv->getLastError()));
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.lvm.physicalvolume", array(
			  "devicefile" => $params['devicefile']
		  ));
	}

	/**
	 * Resize a physical volume.
	 * @param params An array containing the following fields:
	 *   \em devicefile The devicefile of the physical volume.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 * @throw E_EXEC_MISC
	 */
	public function resizePhysicalVolume($params, $context) {
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
		// Check if the physical volume exists.
		$pv = new OMVLvmPhysicalVolume($params['devicefile']);
		if (FALSE === $pv->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("No physical volume found on '%s'"),
			  $params['devicefile']));
		}
		// Resize the physical volume.
		if (!$pv->resize()) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
			  gettext("Failed to resize physical volume: %s"),
			  $pv->getLastError()));
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.lvm.physicalvolume", array(
			  "devicefile" => $params['devicefile']
		  ));
	}

	/**
	 * Enumerate all volume groups on the system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects. Each object represents a volume group
	 *   with the following properties: \em devicefile, \em uuid, \em name,
	 *   \em free, \em size, \em pvname, \em lvname, \em extentsize,
	 *   \em numextents, \em numfreeextents and \em description.
	 * @throw E_EXEC_MISC
	 */
	public function enumerateVolumeGroups($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Enumerate all volume groups on the system.
		if (FALSE === ($vgs = OMVLvmVolumeGroups::enumerate())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  "Failed to get list of volume groups");
		}
		// Prepare the result object.
		$result = array();
		foreach($vgs as $vgk => $vgv) {
			// Get the volume group details.
			$vg = new OMVLvmVolumeGroup($vgv);
			if(!$vg->exists())
				continue;
			$result[] = array(
				"devicefile" => $vg->getDeviceFile(),
				"uuid" => $vg->getUuid(),
				"name" => $vg->getName(),
				"free" => $vg->getFree(),
				"size" => $vg->getSize(),
				"pvname" => $vg->getPVName(),
				"lvname" => $vg->getLVName(),
				"extentsize" => $vg->getExtentSize(),
				"numextents" => $vg->getNumExtents(),
				"numfreeextents" => $vg->getNumFreeExtents(),
				"description" => $vg->getDescription()
			);
		}
		return $result;
	}

	/**
	 * Get a list of volume groups.
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
	function getVolumeGroupsList($params, $context) {
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
		// Enumerate all volume groups on the system.
		$vgs = $this->callMethod("enumerateVolumeGroups", NULL, $context);
		foreach($vgs as $vgk => &$vgv) {
			$vgv['_used'] = !empty($vgv['lvname']);
		}
		// Filter result.
		return $this->applyFilter($vgs, $params['start'], $params['limit'],
		  $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get list of physical volumes that can be used to create a volume group.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing objects with the following fields:
	 *   devicefile, size and description.
	 * @throw E_EXEC_MISC
	 */
	public function getVolumeGroupCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		$result = array();
		// Prepare list of used devices.
		$usedDevs = array();
		// Get volume groups to get the used physical volumes.
		if (FALSE === ($vgs = OMVLvmVolumeGroups::enumerate())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  "Failed to get list of volume groups");
		}
		foreach($vgs as $vgk => $vgv) {
			$vg = new OMVLvmVolumeGroup($vgv);
			// Add the physical volumes assigned to the volume group to the
			// list of already used physical volumes.
			$usedDevs = array_merge($usedDevs, $vg->getPVName());
		}
		// Get all physical physical volumes.
		if (FALSE === ($pvs = OMVLvmPhysicalVolumes::enumerate())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  "Failed to get list of physical volumes");
		}
		foreach($pvs as $pvk => $pvv) {
			$pv = new OMVLvmPhysicalVolume($pvv);
			if(!$pv->exists())
				continue;
			// Is this physical volume already used?
			if(in_array($pv->getDeviceFile(), $usedDevs))
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
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_MISC
	 */
	public function createVolumeGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "name":{"type":"string"},
				  "devices":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE_LIST'].'}
			  }
		  }');
		// Get the physical volumes to be used.
		$devices = preg_split("/[,;]/", $params['devices']);
		// Create the volume group.
		$vg = new OMVLvmVolumeGroup($params['name']);
		if (!$vg->create($devices)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
			  gettext("Failed to create volume group: %s"),
			  $vg->getLastError()));
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.lvm.volumegroup", array(
			  "name" => $params['name'],
			  "devices" => $devices
		  ));
	}

	/**
	 * Get volume group details.
	 * @param params An array containing the following fields:
	 *   \em uuid name The name of the volume group, e.g. vg0 or /dev/vg1.
	 * @param context The context of the caller.
	 * @return The volume group details containing the fields \em devicefile,
	 *   \em uuid, \em name, \em free, \em size, \em pvname, \em lvname,
	 *   \em extentsize, \em numextents, \em numfreeextents and
	 *   \em description.
	 * @throw E_MISC_FAILURE
	 */
	public function getVolumeGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "name":{"type":"string"}
			  }
		  }');
		// Get the volume group details.
		$vg = new OMVLvmVolumeGroup($params['name']);
		if(FALSE === $vg->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Volume group '%s' not found", $vg->getName()));
		}
		return array(
			"devicefile" => $vg->getDeviceFile(),
			"uuid" => $vg->getUuid(),
			"name" => $vg->getName(),
			"free" => $vg->getFree(),
			"size" => $vg->getSize(),
			"pvname" => $vg->getPVName(),
			"lvname" => $vg->getLVName(),
			"extentsize" => $vg->getExtentSize(),
			"numextents" => $vg->getNumExtents(),
			"numfreeextents" => $vg->getNumFreeExtents(),
			"description" => $vg->getDescription()
		);
	}

	/**
	 * Delete a volume group.
	 * @param params An array containing the following fields:
	 *   \em name The name of the volume group, e.g. vg0 or /dev/vg1.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 * @throw E_EXEC_MISC
	 */
	public function deleteVolumeGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "name":{"type":"string"}
			  }
		  }');
		// Check if the volume group exists.
		$vg = new OMVLvmVolumeGroup($params['name']);
		if (!$vg->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE, sprintf(
			  gettext("Volume group '%s' not found", $params['name'])));
		}
		// Remove the volume group.
		if (!$vg->remove()) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
			  gettext("Failed to remove volume group: %s"),
			  $vg->getLastError()));
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.lvm.volumegroup", array(
			  "name" => $params['name']
		  ));
	}

	/**
	 * Get the physical volumes assigned to a volume group.
	 * @param params An array containing the following fields:
	 *   \em name The name of the volume group, e.g. vg0 or /dev/vg1.
	 * @param context The context of the caller.
	 * @return An array containing the physical volumes assigned to the
	 *   given volume group. Each array item has the fields \em devicefile,
	 *   \em size, \em description and \em _used.
	 * @throw E_MISC_FAILURE
	 */
	public function getVolumeGroupPhysicalVolumes($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "name":{"type":"string"}
			  }
		  }');
		// Get the volume groups physical volumes.
		$vg = new OMVLvmVolumeGroup($params['name']);
		if(FALSE === $vg->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Volume group '%s' not found", $vg->getName()));
		}
		$result = array();
		foreach($vg->getPVName() as $pvk => $pvv) {
			$pv = new OMVLvmPhysicalVolume($pvv);
			if(!$pv->exists())
				continue;
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
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 * @throw E_EXEC_FAILED
	 * @throw E_EXEC_MISC
	 */
	public function renameVolumeGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
				  "name":{"type":"string"}
			  }
		  }');
		// Check if the volume group exists.
		$vg = new OMVLvmVolumeGroup($params['devicefile']);
		if (FALSE === $vg->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("No volume group found on '%s'",
			  $params['devicefile']));
		}
		// Rename the volume group.
		if ($params['name'] !== $vg->getName()) {
			if (!$vg->rename($params['name'])) {
				throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
				  gettext("Failed to rename volume group: %s"),
				  $vg->getLastError()));
			}
			// Notify configuration changes.
			$dispatcher = &OMVNotifyDispatcher::getInstance();
			$dispatcher->notify(OMV_NOTIFY_MODIFY,
			  "org.openmediavault.system.storage.lvm.volumegroup", array(
				  "devicefile" => $params['devicefile'],
				  "name" => $params['name']
			  ));
		}
	}

	/**
	 * Extend a volume group.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 * @throw E_EXEC_MISC
	 */
	public function extendVolumeGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
				  "devices":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE_LIST'].'}
			  }
		  }');
		// Check if the volume group exists.
		$vg = new OMVLvmVolumeGroup($params['devicefile']);
		if (FALSE === $vg->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("No volume group found on '%s'",
			  $params['devicefile']));
		}
		// Get the physical volumes assigned to the volume group.
		if (FALSE === ($pvs = $vg->getPVName())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  "Failed to get list of physical volumes");
		}
		// Get the new assigned physical volumes.
		$extendPV = preg_split("/[,;]/", $params['devices']);
		// Extend the volume group.
		if (!empty($extendPV)) {
			if (!$vg->extend($extendPV)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
				  gettext("Failed to extend volume group: %s"),
				  $vg->getLastError()));
			}
			// Notify configuration changes.
			$dispatcher = &OMVNotifyDispatcher::getInstance();
			$dispatcher->notify(OMV_NOTIFY_MODIFY,
			  "org.openmediavault.system.storage.lvm.volumegroup", array(
				  "devicefile" => $params['devicefile'],
				  "devices" => array_merge($extendPV, $pvs)
			  ));
		}
	}

	/**
	 * Reduce a volume group.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_MISC
	 */
	public function reduceVolumeGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
				  "devices":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE_LIST'].'}
			  }
		  }');
		// Check if the volume group exists.
		$vg = new OMVLvmVolumeGroup($params['devicefile']);
		if (FALSE === $vg->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("No volume group found on '%s'",
			  $params['devicefile']));
		}
		// Get the physical volumes assigned to the volume group.
		if (FALSE === ($pvs = $vg->getPVName())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  "Failed to get list of physical volumes");
		}
		// Get the removed physical volumes.
		$reducePV = preg_split("/[,;]/", $params['devices']);
		// Reduce the volume group.
		if (!empty($reducePV)) {
			if (!$vg->reduce($reducePV)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
				  gettext("Failed to reduce volume group: %s"),
				  $vg->getLastError()));
			}
			// Notify configuration changes.
			$dispatcher = &OMVNotifyDispatcher::getInstance();
			$dispatcher->notify(OMV_NOTIFY_MODIFY,
			  "org.openmediavault.system.storage.lvm.volumegroup", array(
				  "devicefile" => $params['devicefile'],
				  "devices" => array_diff($pvs, $reducePV)
			  ));
		}
	}

	/**
	 * Enumerate all logical volumes on the system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects. Each object represents a logical volume
	 *   with the following properties: devicefile, uuid, name, size and
	 *   vgname.
	 * @throw E_EXEC_MISC
	 */
	public function enumerateLogicalVolumes($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Enumerate all volume groups on the system.
		if (FALSE === ($lvs = OMVLvmLogicalVolumes::enumerate())) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  "Failed to get list of logical volumes");
		}
		$result = array();
		foreach($lvs as $lvk => $lvv) {
			// Get the physical volume details.
			$lv = new OMVLvmLogicalVolume($lvv);
			if($lv->exists()) {
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
	function getLogicalVolumesList($params, $context) {
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
		// Enumerate all volume groups on the system.
		$lvs = $this->callMethod("enumerateLogicalVolumes", NULL, $context);
		foreach($lvs as $lvk => &$lvv) {
			$used = FALSE;
			// Does the logical volume contain a filesystem and is it used?
			if(FALSE !== OMVRpc::exec("FsTab", "getByFsName", array(
			  "id" => $lvv['devicefile']), $context)) {
				$used = TRUE;
			}
			$lvv['_used'] = $used;
		}
		// Filter result.
		return $this->applyFilter($lvs, $params['start'], $params['limit'],
		  $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get list of volume groups that can be used to create a logical volume.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing objects with the following fields:
	 *   devicefile, size and description.
	 * @throw E_EXEC_MISC
	 */
	public function getLogicalVolumeCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Enumerate all volume groups on the system.
		return $this->callMethod("enumerateVolumeGroups", NULL, $context);
	}

	/**
	 * Create a logical volume.
	 * @param params An array containing the following fields:
	 *   \em name The name of the logical volume, e.g. lv0.
	 *   \em size The size of the logical volume in bytes. This is a string to
	 *      support volumes > 2GiB on 32bit systems.
	 *   \em vgname The name of the volume group, e.g. /dev/vg0.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_FAILED
	 */
	public function createLogicalVolume($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "name":{"type":"string"},
				  "size":{"type":"string"},
				  "vgname":{"type":"string"}
			  }
		  }');
		// Create the logical volume. Convert size to KiB to ensure it is
		// a multiple of 512.
		$lv = new OMVLvmLogicalVolume();
		if (!$lv->create($params['name'], $params['size'],
		  $params['vgname'])) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
			  gettext("Failed to create logical volume: %s"),
			  $lv->getLastError()));
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.lvm.logicalvolume", array(
			  "name" => $params['name'],
			  "size" => $params['size'],
			  "vgname" => $params['vgname']
		  ));
	}

	/**
	 * Rename a logical volume.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 * @throw E_EXEC_FAILED
	 */
	public function renameLogicalVolume($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
				  "name":{"type":"string"}
			  }
		  }');
		// Check if logical volume exists.
		$lv = new OMVLvmLogicalVolume($params['devicefile']);
		if (FALSE === $lv->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("No logical volume found on '%s'",
			  $params['devicefile']));
		}
		// Rename the logical volume.
		if ($params['name'] !== $lv->getName()) {
			if (!$lv->rename($params['name'])) {
				throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
				  gettext("Failed to rename logical volume: %s"),
				  $lv->getLastError()));
			}
			// Notify configuration changes.
			$dispatcher = &OMVNotifyDispatcher::getInstance();
			$dispatcher->notify(OMV_NOTIFY_MODIFY,
			  "org.openmediavault.system.storage.lvm.logicalvolume", array(
				  "devicefile" => $params['devicefile'],
				  "name" => $params['name']
			  ));
		}
	}

	/**
	 * Delete a logical volume.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file of the logical volume.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 * @throw E_EXEC_MISC
	 */
	public function deleteLogicalVolume($params, $context) {
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
		// Check if logical volume exists.
		$lv = new OMVLvmLogicalVolume($params['devicefile']);
		if (!$lv->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf(gettext("No logical volume found on '%s'"),
			  $params['devicefile']));
		}
		// Delete the logical volume.
		if (!$lv->remove()) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
			  gettext("Failed to remove logical volume: %s"),
			  $lv->getLastError()));
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.storage.lvm.logicalvolume", array(
			  "devicefile" => $params['devicefile']
		  ));
	}

	/**
	 * Modify a logical volume.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file of the logical volume to modify.
	 *   \em size The new size of the logical volume in bytes. This is a string
	 *      to support volumes > 2GiB on 32bit systems.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 * @throw E_EXEC_FAILED
	 */
	public function modifyLogicalVolume($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
				  "size":{"type":"string"}
			  }
		  }');
		// Check if logical volume exists.
		$lv = new OMVLvmLogicalVolume($params['devicefile']);
		if (FALSE === $lv->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("No logical volume found on '%s'",
			  $params['devicefile']));
		}
		// Calculate the new size of the logical volume to decide
		// whether it must be extended or reduced.
		$vg = new OMVLvmVolumeGroup($lv->getVGName());
		if (FALSE === $vg->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Volume group '%s' not found", $lv->getVGName()));
		}
		// Extend or reduce volume size?
		switch (bccomp($lv->getSize(), $params['size'])) {
		case -1: // Extend
			if (!$lv->extend($params['size'])) {
				throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
				  gettext("Failed to extend logical volume: %s"),
				  $lv->getLastError()));
			}
			break;
		case 1: // Reduce
			if (!$lv->reduce($params['size'])) {
				throw new OMVException(OMVErrorMsg::E_EXEC_MISC, sprintf(
				  gettext("Failed to reduce logical volume: %s"),
				  $lv->getLastError()));
			}
			break;
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.lvm.logicalvolume", array(
			  "devicefile" => $params['devicefile'],
			  "size" => $params['size']
		  ));
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceLogicalVolumeMgmt());
?>
