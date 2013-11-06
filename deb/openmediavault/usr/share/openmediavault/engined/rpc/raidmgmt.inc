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
require_once("openmediavault/functions.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceRaidMgmt extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "RaidMgmt";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("enumerateDevices");
		$this->registerMethod("getList");
		$this->registerMethod("getCandidates");
		$this->registerMethod("create");
		$this->registerMethod("grow");
		$this->registerMethod("add");
		$this->registerMethod("delete");
		$this->registerMethod("getDetail");
	}

	/**
	 * Enumerate all RAID devices on the system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing RAID device objects with following
	 *   fields: name, devicefile, uuid, level, numdevices, devices,
	 *   size, state, description and _used.
	 * @throw E_EXEC_MISC
	 */
	public function enumerateDevices($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get all existing software RAID devices.
		if(FALSE === ($devs = OMVStorageDevices::enumerate(
		  OMV_STORAGE_DEVICE_TYPE_SOFTWARERAID))) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  "Failed to get list of software RAID devices");
		}
		// Prepare the result object.
		$result = array();
		foreach($devs as $devk => $devv) {
			$raid = new OMVStorageDeviceMdadm($devv);
			if($raid->exists()) {
				$used = FALSE;
				// Is this RAID device used by another storage device?
				if(OMVStorageDevices::isUsed($raid->getDeviceFile())) {
					$used = TRUE;
				// Does the RAID device contain a filesystem and is it used?
				} else if(FALSE !== OMVRpc::exec("FsTab", "getByFsName",
				  array("id" => $raid->getDeviceFile()), $context)) {
					$used = TRUE;
				}
				$result[] = array(
					"_used" => $used,
					"name" => $raid->getName(),
					"devicefile" => $raid->getDeviceFile(),
					"uuid" => $raid->getUuid(),
					"level" => $raid->getLevel(),
					"numdevices" => $raid->getNumDevices(),
					"devices" => $raid->getSlaves(),
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
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "start":{"type":"integer"},
				  "limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
				  "sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
				  "sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
			  }
		  }');
		// Enumerate all RAID devices on the system.
		$devs = $this->callMethod("enumerateDevices", NULL, $context);
		// Filter the result list.
		return $this->applyFilter($devs, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get list of devices that can be used to create a RAID.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing objects with the following fields:
	 *   devicefile, size and description.
	 * @throw E_EXEC_MISC
	 */
	public function getCandidates($params, $context) {
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
			// The device is a potential candidate to be a member of a
			// Software RAID.
			$result[] = array(
				"devicefile" => $sd->getDeviceFile(),
				"size" => $sd->getSize(),
				"vendor" => $sd->getVendor(),
				"description" => $sd->getDescription()
			);
		}
		return $result;
	}

	/**
	 * Create a RAID device.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_FAILED
	 */
	public function create($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "devices":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE_LIST'].'},
				  "name":{"type":"string"},
				  "level":{"type":"string","enum":["stripe","mirror",'.
					'"linear","raid10","raid5","raid6"]}
			  }
		  }');
		// Generate the RAID device name.
		$deviceName = OMVSystem::getNextDevice("disk", "md");
		$devices = preg_split("/[,;]/", $params['devices']);
		// Create the RAID device.
		$cmdArgs = array();
		$cmdArgs[] = sprintf("/dev/%s", $deviceName);
		$cmdArgs[] = "-l";
		$cmdArgs[] = $params['level'];
		$cmdArgs[] = "-n";
		$cmdArgs[] = count($devices);
		if(!empty($params['name'])) {
			$cmdArgs[] = "-N";
			$cmdArgs[] = $params['name'];  
		}
		$cmdArgs[] = implode(" ", $devices);
		$cmd = sprintf("omv-mkraid %s 2>&1", implode(" ", $cmdArgs));
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_CREATE,
		  "org.openmediavault.system.storage.raid.device", array(
			  "name" => $params['name'],
			  "devicefile" => sprintf("/dev/%s", $deviceName),
			  "level" => $params['level'],
			  "numdevices" => count($devices),
			  "devices" => $devices
		  ));
	}

	/**
	 * Grow a RAID device.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 * @throw E_EXEC_FAILED
	 */
	public function grow($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
				  "devices":{"type":"string"}
			  }
		  }');
		// Get software RAID device details to create notification message.
		$raid = new OMVStorageDeviceMdadm($params['devicefile']);
		if(FALSE === $raid->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $raid->getDeviceFile());
		}
		$numDevices = $raid->getNumDevices();
		$devices = $raid->getSlaves();
		// Calculate new number of devices.
		$newDevices = explode(",", $params['devices']);
		$numDevices += count($newDevices);
		// Add new devices to the RAID device.
		$cmd = sprintf("export LANG=C; mdadm --manage %s --add %s 2>&1",
		  $raid->getDeviceFile(), implode(" ", $newDevices));
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Now grow the RAID.
		// See for more details:
		// https://raid.wiki.kernel.org/index.php/Growing
		// https://raid.wiki.kernel.org/index.php/Write-intent_bitmap
		$cmdArgs = array();
		$cmdArgs[] = "--grow";
		$cmdArgs[] = sprintf("--raid-devices=%d", $numDevices);
		$cmd = sprintf("export LANG=C; mdadm %s %s 2>&1", implode(" ",
		  $cmdArgs), $raid->getDeviceFile());
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.raid.device", array(
			  "name" => $raid->getName(),
			  "devicefile" => $raid->getDeviceFile(),
			  "level" => $raid->getLevel(),
			  "numdevices" => $numDevices,
			  "devices" => array_merge($devices, $newDevices)
		  ));
	}

	/**
	 * Add devices to a RAID device.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 * @throw E_EXEC_FAILED
	 */
	public function add($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "devicefile":{'.$GLOBALS['OMV_JSONSCHEMA_DEVICEFILE'].'},
				  "devices":{"type":"string"}
			  }
		  }');
		// Get software RAID device details to create notification message.
		$raid = new OMVStorageDeviceMdadm($params['devicefile']);
		if(FALSE === $raid->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $raid->getDeviceFile());
		}
		$numDevices = $raid->getNumDevices();
		$devices = $raid->getSlaves();
		// Calculate new number of devices.
		$newDevices = explode(",", $params['devices']);
		$numDevices += count($newDevices);
		// Add new devices to the RAID device.
		$cmd = sprintf("export LANG=C; mdadm --manage %s --add %s 2>&1",
		  $raid->getDeviceFile(), implode(" ", $newDevices));
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.raid.device", array(
			  "name" => $raid->getName(),
			  "devicefile" => $raid->getDeviceFile(),
			  "level" => $raid->getLevel(),
			  "numdevices" => $numDevices,
			  "devices" => array_merge($devices, $newDevices)
		  ));
	}

	/**
	 * Delete a RAID device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The RAID device file to be deleted.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 * @throw E_EXEC_FAILED
	 */
	public function delete($params, $context) {
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
		// Get software RAID device details to create notification message.
		$raid = new OMVStorageDeviceMdadm($params['devicefile']);
		if(FALSE === $raid->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $raid->getDeviceFile());
		}
		// Create the notification object.
		$object = array(
			"name" => $raid->getName(),
			"devicefile" => $raid->getDeviceFile(),
			"level" => $raid->getLevel(),
			"numdevices" => $raid->getNumDevices(),
			"devices" => $raid->getSlaves()
		);
		// Delete the RAID device.
		$cmd = sprintf("omv-rmraid %s", $raid->getDeviceFile());
		if(0 !== $this->exec($cmd, $output)) {
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
	 * @param params An array containing the following fields:
	 *   \em devicefile The RAID device file to get details from.
	 * @param context The context of the caller.
	 * @return The details of the given RAID device as string.
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 */
	public function getDetail($params, $context) {
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
		// Get software RAID device details.
		$raid = new OMVStorageDeviceMdadm($params['devicefile']);
		if(FALSE === $raid->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $raid->getDeviceFile());
		}
		return $raid->getDetail();
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceRaidMgmt());
?>
