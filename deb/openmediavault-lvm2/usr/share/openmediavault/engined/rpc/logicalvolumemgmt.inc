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
class OMVRpcServiceLogicalVolumeMgmt extends \OMV\Rpc\ServiceAbstract {
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
		$this->registerMethod("getLogicalVolume");
		$this->registerMethod("renameLogicalVolume");
		$this->registerMethod("deleteLogicalVolume");
		$this->registerMethod("extendLogicalVolume");
		$this->registerMethod("reduceLogicalVolume");
		$this->registerMethod("createLogicalVolumeSnapshot");
	}

	/**
	 * Enumerate all physical volumes on the system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects. Each object represents a physical volume
	 *   with the following properties: devicename, free, used, size, vguuid,
	 *   vgname and description.
	 */
	public function enumeratePhysicalVolumes($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Enumerate all physical volumes on the system.
		$devs = \OMV\System\Storage\Lvm\PhysicalVolume::enumerate();
		$result = [];
		foreach ($devs as $devk => $devv) {
			// Get the physical volume details.
			$pv = new \OMV\System\Storage\Lvm\PhysicalVolume($devv);
			if (!$pv->exists())
				continue;
			$result[] = [
				"devicefile" => $pv->getPreferredDeviceFile(),
				"free" => $pv->getFree(),
				"used" => $pv->getUsed(),
				"size" => $pv->getSize(),
				"vguuid" => $pv->getVGUuid(),
				"vgname" => $pv->getVGName(),
				"description" => $pv->getDescription()
			];
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Enumerate all physical volumes on the system.
		$pvs = $this->callMethod("enumeratePhysicalVolumes", NULL, $context);
		foreach ($pvs as $pvk => &$pvv) {
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
	 */
	public function getPhysicalVolumeCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get a list of all potential usable devices.
		$devs = \OMV\System\Storage\StorageDevice::enumerateUnused();
		if (FALSE === $devs)
			throw new \OMV\Exception("Failed to get list of devices.");
		// Prepare the result list.
		$result = [];
		foreach ($devs as $devk => $devv) {
			// Get the object that implements the given storage device.
			$sd = \OMV\System\Storage\StorageDevice::getStorageDevice($devv);
			if (is_null($sd) || !$sd->exists())
				continue;
			// Skip read-only devices like CDROM.
			if (TRUE === $sd->isReadOnly())
				continue;
			// Check if the device is referenced/used by a plugin.
			$db = \OMV\Config\Database::getInstance();
			if (TRUE === $db->exists("conf.service", [
					"operator" => "stringContains",
					"arg0" => "devicefile",
					"arg1" => $sd->getDeviceFile()
				]))
				continue;
			// Does this device already contain a filesystem?
			if (FALSE !== \OMV\System\Filesystem\Filesystem::hasFileSystem(
				$sd->getDeviceFile()))
				continue;
			// The device is a potential candidate to be used as a
			// physical volume.
			$result[] = [
				"devicefile" => $sd->getDeviceFile(),
				"size" => $sd->getSize(),
				"description" => $sd->getDescription()
			];
		}
		return $result;
	}

	/**
	 * Create a physical volume.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function createPhysicalVolume($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Make sure the disk device is clean, no partition table
		// should exist, otherwise pvcreate fails.
		$sd = \OMV\System\Storage\StorageDevice::assertGetStorageDevice(
			$params['devicefile']);
		$sd->wipe();
		// Create the physical volume.
		$pv = new \OMV\System\Storage\Lvm\PhysicalVolume($sd->getDeviceFile());
		$pv->create();
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
		  "org.openmediavault.conf.system.storage.lvm.physicalvolume", [
			  "devicefile" => $sd->getDeviceFile()
		  ]);
	}

	/**
	 * Delete a physical volume.
	 * @param params An array containing the following fields:
	 *   \em devicefile The devicefile of the physical volume.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function deletePhysicalVolume($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Check if the physical volume exists.
		$pv = new \OMV\System\Storage\Lvm\PhysicalVolume($params['devicefile']);
		$pv->assertExists();
		// Remove the physical volume.
		$pv->remove();
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.conf.system.storage.lvm.physicalvolume", [
			  "devicefile" => $params['devicefile']
		  ]);
	}

	/**
	 * Resize a physical volume.
	 * @param params An array containing the following fields:
	 *   \em devicefile The devicefile of the physical volume.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function resizePhysicalVolume($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Check if the physical volume exists.
		$pv = new \OMV\System\Storage\Lvm\PhysicalVolume($params['devicefile']);
		$pv->assertExists();
		// Resize the physical volume.
		$pv->resize();
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.system.storage.lvm.physicalvolume", [
			  "devicefile" => $params['devicefile']
		  ]);
	}

	/**
	 * Enumerate all volume groups on the system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects. Each object represents a volume group
	 *   with the following properties: \em devicefile, \em uuid, \em name,
	 *   \em free, \em size, \em pvname, \em lvname, \em extentsize,
	 *   \em numextents, \em numfreeextents and \em description.
	 */
	public function enumerateVolumeGroups($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Enumerate all volume groups on the system.
		$vgs = \OMV\System\Storage\Lvm\VolumeGroup::enumerate();
		// Prepare the result object.
		$result = [];
		foreach ($vgs as $vgk => $vgv) {
			// Get the volume group details.
			$vg = new \OMV\System\Storage\Lvm\VolumeGroup($vgv);
			if (!$vg->exists())
				continue;
			$result[] = [
				"devicefile" => $vg->getPreferredDeviceFile(),
				"uuid" => $vg->getUuid(),
				"name" => $vg->getName(),
				"free" => $vg->getFree(),
				"size" => $vg->getSize(),
				"pvname" => $vg->getPVName(),
				"lvname" => $vg->getLVName(),
				"extentsize" => $vg->getExtentSize(),
				"numextents" => $vg->getNumExtents(),
				"numfreeextents" => $vg->getNumFreeExtents(),
				"attributes" => $vg->getAttributes(),
				"description" => $vg->getDescription()
			];
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Enumerate all volume groups on the system.
		$vgs = $this->callMethod("enumerateVolumeGroups", NULL, $context);
		foreach ($vgs as $vgk => &$vgv) {
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
	 */
	public function getVolumeGroupCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$result = [];
		// Prepare list of used devices.
		$usedDevs = [];
		// Get volume groups to get the used physical volumes.
		$vgs = \OMV\System\Storage\Lvm\VolumeGroup::enumerate();
		foreach ($vgs as $vgk => $vgv) {
			$vg = new \OMV\System\Storage\Lvm\VolumeGroup($vgv);
			// Add the physical volumes assigned to the volume group to the
			// list of already used physical volumes.
			$usedDevs = array_merge($usedDevs, $vg->getPVName());
		}
		// Get all physical physical volumes.
		$pvs = \OMV\System\Storage\Lvm\PhysicalVolume::enumerate();
		foreach ($pvs as $pvk => $pvv) {
			$pv = new \OMV\System\Storage\Lvm\PhysicalVolume($pvv);
			if (FALSE === $pv->exists())
				continue;
			// Is this physical volume already used?
			if (in_array($pv->getDeviceFile(), $usedDevs))
				continue;
			$result[] = [
				"devicefile" => $pv->getDeviceFile(),
				"size" => $pv->getSize(),
				"description" => $pv->getDescription()
			];
		}
		return $result;
	}

	/**
	 * Create a volume group.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function createVolumeGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
			"rpc.logicalvolumemgmt.createvolumegroup");
		// Create the volume group.
		$vg = new \OMV\System\Storage\Lvm\VolumeGroup($params['name']);
		$vg->create($params['devices']);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
			"org.openmediavault.conf.system.storage.lvm.volumegroup", [
				"name" => $params['name'],
				"devices" => $params['devices']
			]);
	}

	/**
	 * Get volume group details.
	 * @param params An array containing the following fields:
	 *   \em name The name of the volume group, e.g. vg0 or /dev/vg1.
	 * @param context The context of the caller.
	 * @return The volume group details containing the fields \em devicefile,
	 *   \em uuid, \em name, \em free, \em size, \em pvname, \em lvname,
	 *   \em extentsize, \em numextents, \em numfreeextents and
	 *   \em description.
	 */
	public function getVolumeGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
		  "rpc.logicalvolumemgmt.getvolumegroup");
		// Get the volume group details.
		$vg = new \OMV\System\Storage\Lvm\VolumeGroup($params['name']);
		$vg->assertExists();
		return [
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
		];
	}

	/**
	 * Delete a volume group.
	 * @param params An array containing the following fields:
	 *   \em name The name of the volume group, e.g. vg0 or /dev/vg1.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function deleteVolumeGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
		  "rpc.logicalvolumemgmt.deletevolumegroup");
		// Check if the volume group exists.
		$vg = new \OMV\System\Storage\Lvm\VolumeGroup($params['name']);
		$vg->assertExists();
		// Remove the volume group.
		$vg->remove();
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.conf.system.storage.lvm.volumegroup", [
			  "name" => $params['name']
		  ]);
	}

	/**
	 * Get the physical volumes assigned to a volume group.
	 * @param params An array containing the following fields:
	 *   \em name The name of the volume group, e.g. vg0 or /dev/vg1.
	 * @param context The context of the caller.
	 * @return An array containing the physical volumes assigned to the
	 *   given volume group. Each array item has the fields \em devicefile,
	 *   \em size, \em description and \em _used.
	 */
	public function getVolumeGroupPhysicalVolumes($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
		  "rpc.logicalvolumemgmt.getvolumegroupphysicalvolumes");
		// Get the volume groups physical volumes.
		$vg = new \OMV\System\Storage\Lvm\VolumeGroup($params['name']);
		$vg->assertExists();
		$result = [];
		foreach ($vg->getPVName() as $pvk => $pvv) {
			$pv = new \OMV\System\Storage\Lvm\PhysicalVolume($pvv);
			if (FALSE === $pv->exists())
				continue;
			$result[] = [
				"devicefile" => $pv->getDeviceFile(),
				"size" => $pv->getSize(),
				"description" => $pv->getDescription(),
				"_used" => ($pv->getAllocatedPhysicalExtents() > 0) ?
				  TRUE : FALSE
			];
		}
		return $result;
	}

	/**
	 * Rename a volume group.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function renameVolumeGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
		  "rpc.logicalvolumemgmt.renamevolumegroup");
		// Check if the volume group exists.
		$vg = new \OMV\System\Storage\Lvm\VolumeGroup($params['devicefile']);
		$vg->assertExists();
		// Rename the volume group.
		if ($params['name'] !== $vg->getName()) {
			$vg->rename($params['name']);
			// Notify configuration changes.
			$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
			$dispatcher->notify(OMV_NOTIFY_MODIFY,
			  "org.openmediavault.conf.system.storage.lvm.volumegroup", [
				  "devicefile" => $params['devicefile'],
				  "name" => $params['name']
			  ]);
		}
	}

	/**
	 * Extend a volume group.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function extendVolumeGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
			"rpc.logicalvolumemgmt.extendvolumegroup");
		// Check if the volume group exists.
		$vg = new \OMV\System\Storage\Lvm\VolumeGroup($params['devicefile']);
		$vg->assertExists();
		// Get the physical volumes assigned to the volume group.
		$pvs = $vg->getPVName();
		// Extend the volume group.
		if (!empty($params['devices'])) {
			$vg->extend($params['devices']);
			// Notify configuration changes.
			$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
			$dispatcher->notify(OMV_NOTIFY_MODIFY,
			  "org.openmediavault.conf.system.storage.lvm.volumegroup", [
				  "devicefile" => $params['devicefile'],
				  "devices" => array_merge($params['devices'], $pvs)
			  ]);
		}
	}

	/**
	 * Reduce a volume group.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function reduceVolumeGroup($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
			"rpc.logicalvolumemgmt.reducevolumegroup");
		// Check if the volume group exists.
		$vg = new \OMV\System\Storage\Lvm\VolumeGroup($params['devicefile']);
		$vg->assertExists();
		// Get the physical volumes assigned to the volume group.
		$pvs = $vg->getPVName();
		// Reduce the volume group.
		if (!empty($params['devices'])) {
			$vg->reduce($params['devices']);
			// Notify configuration changes.
			$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
			$dispatcher->notify(OMV_NOTIFY_MODIFY,
			  "org.openmediavault.conf.system.storage.lvm.volumegroup", [
				  "devicefile" => $params['devicefile'],
				  "devices" => array_diff($pvs, $params['devices'])
			  ]);
		}
	}

	/**
	 * Enumerate all logical volumes on the system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of objects. Each object represents a logical volume
	 *   with the following properties: devicefile, uuid, name, size and
	 *   vgname.
	 */
	public function enumerateLogicalVolumes($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Enumerate all volume groups on the system.
		$lvs = \OMV\System\Storage\Lvm\LogicalVolume::enumerate();
		$result = [];
		foreach ($lvs as $lvk => $lvv) {
			// Get the physical volume details.
			$lv = new \OMV\System\Storage\Lvm\LogicalVolume($lvv);
			if (FALSE === $lv->exists())
				continue;
			$result[] = [
				"devicefile" => $lv->getPreferredDeviceFile(),
				"uuid" => $lv->getUuid(),
				"name" => $lv->getName(),
				"size" => $lv->getSize(),
				"vgname" => $lv->getVGName(),
				"attributes" => $lv->getAttributes(),
				"vgattributes" => $lv->getVGAttributes(),
				"isavailable" => $lv->IsMediaAvailable()
			];
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.getlist");
		// Enumerate all volume groups on the system.
		$lvs = $this->callMethod("enumerateLogicalVolumes", NULL, $context);
		foreach($lvs as $lvk => &$lvv) {
			$lvv['_used'] = FALSE;
			// Does the logical volume contain a filesystem and is it used?
			if(FALSE !== \OMV\Rpc\Rpc::call("FsTab", "getByFsName", [
				"fsname" => $lvv['devicefile']
			], $context)) {
				$lvv['_used'] = TRUE;
			}
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
	 */
	public function getLogicalVolumeCandidates($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Enumerate all volume groups on the system.
		return $this->callMethod("enumerateVolumeGroups", NULL, $context);
	}

	/**
	 * Create a logical volume.
	 * @param params An array containing the following fields:
	 *   \em name The name of the logical volume, e.g. lv0.
	 *   \em size The size of the logical volume in percent.
	 *   \em vgname The name of the volume group, e.g. /dev/vg0.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function createLogicalVolume($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
		  "rpc.logicalvolumemgmt.createlogicalvolume");
		// Create the logical volume.
		\OMV\System\Storage\Lvm\LogicalVolume::create(
			$params['name'], $params['size'], $params['vgname']);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
		  "org.openmediavault.conf.system.storage.lvm.logicalvolume", [
			  "name" => $params['name'],
			  "size" => $params['size'],
			  "vgname" => $params['vgname']
		  ]);
	}

	/**
	 * Get logical volume details.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file of the logical volume.
	 * @param context The context of the caller.
	 * @return The logical volume details containing the fields
	 *   \em devicefile, \em uuid, \em name, \em size, \em vgname,
	 *   \em attributes, \em vgattributes and \em isavailable.
	 */
	public function getLogicalVolume($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
		  "rpc.logicalvolumemgmt.getlogicalvolume");
		// Get the logical volume details.
		$lv = new \OMV\System\Storage\Lvm\LogicalVolume($params['name']);
		$lv->assertExists();
		return [
			"devicefile" => $lv->getPreferredDeviceFile(),
			"uuid" => $lv->getUuid(),
			"name" => $lv->getName(),
			"size" => $lv->getSize(),
			"vgname" => $lv->getVGName(),
			"attributes" => $lv->getAttributes(),
			"vgattributes" => $lv->getVGAttributes(),
			"isavailable" => $lv->IsMediaAvailable()
		];
	}

	/**
	 * Rename a logical volume.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file of the logical volume.
	 *   \em name The new name of the logical volume.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function renameLogicalVolume($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
		  "rpc.logicalvolumemgmt.renamelogicalvolume");
		// Check if logical volume exists.
		$lv = new \OMV\System\Storage\Lvm\LogicalVolume($params['devicefile']);
		$lv->assertExists();
		// Rename the logical volume.
		if ($params['name'] !== $lv->getName()) {
			$lv->rename($params['name']);
			// Notify configuration changes.
			$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
			$dispatcher->notify(OMV_NOTIFY_MODIFY,
			  "org.openmediavault.conf.system.storage.lvm.logicalvolume", [
				  "devicefile" => $params['devicefile'],
				  "name" => $params['name']
			  ]);
		}
	}

	/**
	 * Delete a logical volume.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file of the logical volume.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function deleteLogicalVolume($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.devicefile");
		// Check if logical volume exists.
		$lv = new \OMV\System\Storage\Lvm\LogicalVolume($params['devicefile']);
		$lv->assertExists();
		// Delete the logical volume.
		$lv->remove();
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.conf.system.storage.lvm.logicalvolume", [
			  "devicefile" => $params['devicefile']
		  ]);
	}

	/**
	 * Extend a logical volume.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file of the logical volume to modify.
	 *   \em size The percentage of the total space in the volume group.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function extendLogicalVolume($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
		  "rpc.logicalvolumemgmt.extendlogicalvolume");
		// Check if logical volume exists.
		$lv = new \OMV\System\Storage\Lvm\LogicalVolume($params['devicefile']);
		$lv->assertExists();
		$lv->extend($params['size']);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.system.storage.lvm.logicalvolume", [
			  "devicefile" => $params['devicefile'],
			  "size" => $params['size']
		  ]);
	}

	/**
	 * Reduce a logical volume.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file of the logical volume to modify.
	 *   \em size The percentage of the total space in the volume group.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function reduceLogicalVolume($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
		  "rpc.logicalvolumemgmt.reducelogicalvolume");
		// Check if logical volume exists.
		$lv = new \OMV\System\Storage\Lvm\LogicalVolume($params['devicefile']);
		$lv->assertExists();
		$lv->reduce($params['size']);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.system.storage.lvm.logicalvolume", [
			  "devicefile" => $params['devicefile'],
			  "size" => $params['size']
		  ]);
	}

	/**
	 * Create a logical volume snapshot.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file of the logical volume.
	 *   \em size The size of the logical volume in bytes. This is a string to
	 *      support volumes > 2GiB on 32bit systems. If this parameter is not
	 *      set, then the size of the specified \em devicefile is used.
	 * @param context The context of the caller.
	 * @return void
	 */
	public function createLogicalVolumeSnapshot($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params,
			"rpc.logicalvolumemgmt.createlogicalvolumesnapshot");
		// Get the logical volume.
		$lv = new \OMV\System\Storage\Lvm\LogicalVolume($params['devicefile']);
		$lv->assertExists();
		// Create the snapshot.
		$name = sprintf("%s_snapshot_%s", $lv->getName(), date("Ymd-Hms"));
		$size = array_value($params, "size", $lv->getSize());
		$lv->createSnapshot($name, $size);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
			"org.openmediavault.conf.system.storage.lvm.logicalvolume", [
				"name" => $name,
				"size" => $size,
				"vgname" => $lv->getVGName()
			]);
	}
}
