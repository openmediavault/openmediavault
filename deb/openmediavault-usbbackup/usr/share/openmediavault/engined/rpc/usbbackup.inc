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
require_once("openmediavault/error.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceUsbBackup extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "UsbBackup";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getCandidates");
		$this->registerMethod("getList");
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("delete");
		$this->registerMethod("execute");
	}

	/**
	 * Get list of USB volumes that can be used for backup jobs.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing objects with the following fields:
	 *   \em uuid, \em devicefile and \em description.
	 *   The field \em uuid is the mount point configuration object UUID.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getCandidates($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get list of all detected filesystems. Note, the list may also
		// contain RAID and LVM volumes, so the list must be filtered.
		if(FALSE === ($objects = OMVFilesystems::get())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get filesystems"));
		}
		// Get the root partition (the filesystem containing the operation
		// system) to filter it out.
		$rootDeviceFile = OMVSystem::getRootDeviceFile();
		// Collect volumes that are candidates for a backup job.
		$result = array();
		foreach($objects as $objectk => $objectv) {
			// Skip filesystem we do not support?
			if(!in_array($objectv['type'], array("ext3", "ext4", "xfs",
			  "jfs", "msdos", "ntfs", "vfat", "hfsplus"))) {
				continue;
			}
			// Skip the device where the operating system is installed on.
			if($objectv['devicefile'] === $rootDeviceFile) {
				continue;
			}
			// Check if the volume is connected via USB.
			$fs = new OMVFilesystem($objectv['devicefile']);
			if(!$fs->exists()) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf(gettext("Filesystem '%s' not found"),
				  $objectv['devicefile']));
				continue;
			}
			$sd = OMVStorageDeviceFactory::get($fs->getParentDeviceFile());
			if(is_null($sd) || !$sd->exists()) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  sprintf(gettext("Device '%s' not found"),
				  $fs->getParentDeviceFile()));
			}
			if(!$sd->isUsb())
				continue;
			// Prepare result object.
			$result[] = array(
				"uuid" => $objectv['uuid'],
				"devicefile" => $objectv['devicefile'],
				"label" => $objectv['label'],
				"type" => $objectv['type'],
				"description" => $sd->getDescription()
			);
		}
		return $result;
	}

	/**
	 * Get list of USB backup job configuration objects.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
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
		// Get the configuration objects.
		$xpath = "//services/usbbackup/jobs/job";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional share informations.
		foreach($objects as $objectk => &$objectv) {
			$objectv['sharedfoldername'] = gettext("n/a");
			// Get the shared folder configuration object.
			$sfobject = $xmlConfig->get("//system/shares/sharedfolder[" .
			  "uuid='{$objectv['sharedfolderref']}']");
			if(!is_null($sfobject) && !empty($sfobject)) {
				$objectv['sharedfoldername'] = $sfobject['name'];
			}
			// Get job status.
			$objectv['running'] = file_exists(sprintf("/var/run/sync-%s",
			  $objectv['fsuuid']));
			// Modify result object.
			$objectv['enable'] = boolval($objectv['enable']);
			$objectv['sendemail'] = boolval($objectv['sendemail']);
			$objectv['usesubdir'] = boolval($objectv['usesubdir']);
			$objectv['recursive'] = boolval($objectv['recursive']);
			$objectv['times'] = boolval($objectv['times']);
			$objectv['compress'] = boolval($objectv['compress']);
			$objectv['archive'] = boolval($objectv['archive']);
			$objectv['delete'] = boolval($objectv['delete']);
			$objectv['quiet'] = boolval($objectv['quiet']);
			$objectv['perms'] = boolval($objectv['perms']);
			$objectv['acls'] = boolval($objectv['acls']);
			$objectv['xattrs'] = boolval($objectv['xattrs']);
		}
		// Filter the result.
		return $this->applyFilter($objects, $params['start'],
		  $params['limit'], $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a USB backup job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function get($params, $context) {
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
		$xpath = sprintf("//services/usbbackup/jobs/job[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result object.
		$object['enable'] = boolval($object['enable']);
		$object['sendemail'] = boolval($object['sendemail']);
		$object['usesubdir'] = boolval($object['usesubdir']);
		$object['recursive'] = boolval($object['recursive']);
		$object['times'] = boolval($object['times']);
		$object['compress'] = boolval($object['compress']);
		$object['archive'] = boolval($object['archive']);
		$object['delete'] = boolval($object['delete']);
		$object['quiet'] = boolval($object['quiet']);
		$object['perms'] = boolval($object['perms']);
		$object['acls'] = boolval($object['acls']);
		$object['xattrs'] = boolval($object['xattrs']);
		$object['partial'] = boolval($object['partial']);
		return $object;
	}

	/**
	 * Set a USB backup job configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_OBJECT_UNIQUENESS
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function set($params, $context) {
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
				  "mode":{"type":"string","enum":["push","pull"]},
				  "sendemail":{"type":"boolean"},
				  "comment":{"type":"string"},
				  "fsuuid":{"type":"string","optional":true},
				  "sharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
				  "usesubdir":{"type":"boolean"},
				  "recursive":{"type":"boolean"},
				  "times":{"type":"boolean"},
				  "compress":{"type":"boolean"},
				  "archive":{"type":"boolean"},
				  "delete":{"type":"boolean"},
				  "quiet":{"type":"boolean"},
				  "perms":{"type":"boolean"},
				  "acls":{"type":"boolean"},
				  "xattrs":{"type":"boolean"},
				  "partial":{"type":"boolean"},
				  "extraoptions":{"type":"string"}
			  }
		  }');
		// Prepare the configuration object.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"enable" => array_boolval($params, 'enable'),
			"mode" => $params['mode'],
			"sendemail" => array_boolval($params, 'sendemail'),
			"comment" => $params['comment'],
			"fsuuid" => $params['fsuuid'],
			"sharedfolderref" => $params['sharedfolderref'],
			"usesubdir" => array_boolval($params, 'usesubdir'),
			"recursive" => array_boolval($params, 'recursive'),
			"times" => array_boolval($params, 'times'),
			"compress" => array_boolval($params, 'compress'),
			"archive" => array_boolval($params, 'archive'),
			"delete" => array_boolval($params, 'delete'),
			"quiet" => array_boolval($params, 'quiet'),
			"perms" => array_boolval($params, 'perms'),
			"acls" => array_boolval($params, 'acls'),
			"xattrs" => array_boolval($params, 'xattrs'),
			"partial" => array_boolval($params, 'partial'),
			"extraoptions" => $params['extraoptions']
		);
		// Set the configuration object.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration.
			$success = $xmlConfig->set("//services/usbbackup/jobs",
			  array("job" => $object));
		} else {
			// Update existing configuration object.
			$success = $xmlConfig->replace("//services/usbbackup/jobs/job[" .
			  "uuid='{$params['uuid']}']", $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.usbbackup.jobs.job", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a backup job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
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
				  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
			  }
		  }');
		// Delete the configuration object.
		$xpath = sprintf("//services/usbbackup/jobs/job[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.usbbackup.jobs.job");
	}

	/**
	 * Execute a backup job.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the cron job to execute.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 * @throw E_CONFIG_DIRTY
	 */
	public function execute($params, $context) {
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
		// Check if the module is marked as dirty. This is an indication
		// that the rsync cron script has not been created or updated
		// until now.
		if($this->isModuleDirty("usbbackup")) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_DIRTY);
		}
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
			// Execute the backup script.
			$cmdArgs = array(sprintf("%s/rsync-%s",
			  $GLOBALS['OMV_USBBACKUP_SCRIPTS_DIR'], $params['uuid']));
			if(FALSE === $this->execve("/bin/sh", $cmdArgs,
			  $bgOutputFilename)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  "/bin/sh ".implode(" ", $cmdArgs), "");
			}
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceUsbBackup());
?>
