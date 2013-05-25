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
require_once("openmediavault/util.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceDiskMgmt extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "DiskMgmt";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("enumerateDevices");
		$this->registerMethod("getList");
		$this->registerMethod("getHdParm");
		$this->registerMethod("setHdParm");
		$this->registerMethod("wipe");
		$this->registerMethod("rescan");
	}

	/**
	 * Enumerate all disk devices on the system.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing physical disk device objects with the
	 *   fields \em devicename, \em devicefile, \em model, \em size,
	 *   \em description, \em vendor, \em serialnumber, \em israid and
	 *   \em isroot.
	 */
	public function enumerateDevices($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the device containing the operating system. Mark it as
		// read-only to deny wiping this device.
		$rootDeviceFile = OMVSystem::getRootDeviceFile();
		// Get all existing devices except software RAID devices.
		if(FALSE === ($devs = OMVStorageDevices::enumerate(
		  OMVStorageDevices::ALL & ~OMVStorageDevices::SOFTWARERAID))) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC, "Failed to ".
			  "get list of hard disks and hardware RAID devices");
		}
		// Prepare result list.
		$objects = array();
		foreach($devs as $devk => $devv) {
			$sd = OMVStorageDeviceFactory::get($devv);
			if(!is_null($sd) && $sd->exists()) {
				// Prepare device object
				$objects[] = array(
					"devicename" => $sd->getDeviceName(),
					"devicefile" => $sd->getDeviceFile(),
					"devicefilebyid" => $sd->getDeviceFileById(),
					"model" => $sd->getModel(),
					"size" => $sd->getSize(),
					"description" => $sd->getDescription(),
					"vendor" => $sd->getVendor(),
					"serialnumber" => $sd->getSerialNumber(),
					"israid" => $sd->isRaid(),
					"isroot" => (0 === strpos($rootDeviceFile,
					  $sd->getDeviceFile())) ? TRUE : FALSE
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
		// Enumerate all disk devices on the system.
		$objects = $this->callMethod("enumerateDevices", NULL, $context);
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
				$objectv['hdparm'] = $this->callMethod("getHdParm",
				  array("uuid" => $uuid), $context);
			}
		}
		// Filter result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a hard disk parameters config object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getHdParm($params, $context) {
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
		$xpath = sprintf("//system/storage/hdparm[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the result data.
		$object['smart'] = boolval($object['smart']['enable']);
		$object['apm'] = intval($object['apm']);
		$object['aam'] = intval($object['aam']);
		$object['spindowntime'] = intval($object['spindowntime']);
		$object['writecache'] = boolval($object['writecache']);
		return $object;
	}

	/**
	 * Set (add/update) a hard disk parameters config object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_MISC_DEVICE_NOT_FOUND
	 * @throw E_CONFIG_OBJECT_UNIQUENESS
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	public function setHdParm($params, $context) {
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
				  "devicefile":{"type":["string","null"],"format":"regex",'.
					'"pattern":"#^/dev(/disk/by-id)?/.+$#i"},
				  "smart":{"type":"boolean"},
				  "apm":{"type":"integer","minimum":0,"maximum":255},
				  "aam":{"type":"integer","enum":[0,128,254]},
				  "spindowntime":{"type":"integer","minimum":0,"maximum":254},
				  "writecache":{"type":"boolean"}
			  }
		  }');
		// Check if the given device exists.
		$sd = OMVStorageDeviceFactory::get($params['devicefile']);
		if(is_null($sd) || !$sd->exists()) {
			throw new OMVException(OMVErrorMsg::E_MISC_DEVICE_NOT_FOUND,
			  $params['devicefile']);
		}
		// Prepare the configuration data.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"devicefile" => $sd->getDeviceFileById(),
			"smart" => array(
				"enable" => array_boolval($params, 'smart'),
				"devicetype" => $sd->getSmartDeviceType()
			),
			"apm" => $params['apm'],
			"aam" => $params['aam'],
			"spindowntime" => $params['spindowntime'],
			"writecache" => array_boolval($params, 'writecache')
		);
		// Set the configuration data.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Check uniqueness
			$xpath = sprintf("//system/storage/hdparm[devicefile='%s']",
			  $object['devicefile']);
			if(TRUE === $xmlConfig->exists($xpath)) {
				throw new OMVException(
				  OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				  sprintf("Settings already exist for device '%s'"),
				  $object['devicefile']);
			}
			// Append object to configuration.
			$success = $xmlConfig->set("//system/storage",
			  array("hdparm" => $object));
		} else {
			// Update existing configuration object.
			$success = $xmlConfig->replace(
			  "//system/storage/hdparm[uuid='{$params['uuid']}']",
			  $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify the configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.storage.hdparm", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Wipe the given device.
	 * @param params An array containing the following fields:
	 *   \em devicefile The device file to wipe, e.g. /dev/sdb.
	 *   \em secure Set to TRUE to secure wipe the device.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw E_EXEC_FAILED
	 */
	public function wipe($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "devicefile":{"type":"string","format":"regex",'.
					'"pattern":"#^/dev/.+$#i"},
				  "secure":{"type":"boolean"}
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
			// Wipe the device.
			if(TRUE === boolval($params['secure'])) {
				// Secure wipe the device.
				$cmd = sprintf("export LANG=C; shred -v -n 1 %s 2>&1",
				  escapeshellarg($params['devicefile']));
				if(0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
			} else {
				// Quick wipe the device (see omv-initfs).
				$cmd = sprintf("export LANG=C; sgdisk --zap-all ".
				  "%s 2>&1", escapeshellarg($params['devicefile']));
				if(0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
				// To make sure really everything is wiped ...
				$cmd = sprintf("export LANG=C; dd if=/dev/zero of=%s bs=1M ".
				  "count=1 2>&1", escapeshellarg($params['devicefile']));
				if(0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
					throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
					  $cmd, implode("\n", $output));
				}
			}
			// Reread partition table.
			$cmd = sprintf("export LANG=C; blockdev --rereadpt %s",
			  escapeshellarg($params['devicefile']));
			if(0 !== $this->exec($cmd, $output, $bgOutputFilename)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			$this->finalizeBgProcStatus($bgStatusFilename, $output);
			exit(0);
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}

	/**
	 * Rescan SCSI bus.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_FAILED
	 */
	public function rescan($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Execute shell script to rescan SCSI bus.
		$cmd = "[ -x /sys/class/scsi_host ] && for hostdir in ".
		  "/sys/class/scsi_host/host*; do echo \"- - -\" | ".
		  "tee \${hostdir}/scan >/dev/null; done";
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceDiskMgmt());
?>
