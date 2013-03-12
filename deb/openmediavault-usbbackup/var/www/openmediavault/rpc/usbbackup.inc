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
require_once("openmediavault/util.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class UsbBackupRpc extends OMVRpc {
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
			"get" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"set" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"enable":{"type":"boolean"},
					"mode":{"type":"string","enum":["push","pull"]},
					"sendemail":{"type":"boolean"},
					"comment":{"type":"string"},
					"fsuuid":{"type":"string","optional":true},
					"sharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
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
			}'),
			"delete" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"execute" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}')
		);
	}

	/**
	 * Get list of USB volumes that can be used for backup jobs.
	 * @return An array containing objects with the following fields:
	 *   \em uuid, \em devicefile and \em description.
	 *   The field \em uuid is the mount point configuration object UUID.
	 */
	public function getCandidates() {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions.
		$this->validateSession();
		if(!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get list of all detected filesystems. Note, the list may also
		// contain RAID and LVM volumes, so the list must be filtered.
		if(FALSE === ($objects = OMVFilesystems::get())) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  gettext("Failed to get filesystems"));
		}
		// Get the root partition (the filesystem containing the operation
		// system) to filter it out.
		$rootDeviceFile = OMVSystem::getRootPartition();
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
	 */
	public function getList($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions.
		$this->validateSession();
		if(!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters.
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object.
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
			// Modify result data.
			$objectv['enable'] = boolval($objectv['enable']);
			$objectv['sendemail'] = boolval($objectv['sendemail']);
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
		// Filter result.
		return $this->applyFilter($objects, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get a USB backup job configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The requested configuration object.
	 */
	function get($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions.
		$this->validateSession();
		if(!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Validate method parameters.
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object.
		$xpath = sprintf("//services/usbbackup/jobs/job[uuid='%s']",
		  $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data.
		$object['enable'] = boolval($object['enable']);
		$object['sendemail'] = boolval($object['sendemail']);
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
	 */
	function set($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions.
		$this->validateSession();
		if(!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters.
		$this->validateParams(__METHOD__, func_get_args());
		// Prepare configuration data.
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"enable" => array_boolval($data, 'enable'),
			"mode" => $data['mode'],
			"sendemail" => array_boolval($data, 'sendemail'),
			"comment" => $data['comment'],
			"fsuuid" => $data['fsuuid'],
			"sharedfolderref" => $data['sharedfolderref'],
			"recursive" => array_boolval($data, 'recursive'),
			"times" => array_boolval($data, 'times'),
			"compress" => array_boolval($data, 'compress'),
			"archive" => array_boolval($data, 'archive'),
			"delete" => array_boolval($data, 'delete'),
			"quiet" => array_boolval($data, 'quiet'),
			"perms" => array_boolval($data, 'perms'),
			"acls" => array_boolval($data, 'acls'),
			"xattrs" => array_boolval($data, 'xattrs'),
			"partial" => array_boolval($data, 'partial'),
			"extraoptions" => $data['extraoptions']
		);
		// Set configuration data.
		$success = FALSE;
		if($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration.
			$success = $xmlConfig->set("//services/usbbackup/jobs",
			  array("job" => $object));
		} else {
			// Update existing configuration object.
			$success = $xmlConfig->replace("//services/usbbackup/jobs/job[" .
			  "uuid='{$data['uuid']}']", $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration.
		if(FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.usbbackup.jobs.job", $object);
	}

	/**
	 * Delete a backup job configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return None
	 */
	public function delete($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions.
		$this->validateSession();
		if(!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters.
		$this->validateParams(__METHOD__, func_get_args());
		// Delete the configuration object.
		$xpath = sprintf("//services/usbbackup/jobs/job[uuid='%s']",
		  $data['uuid']);
		$this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.usbbackup.jobs.job");
	}

	/**
	 * Execute a backup job.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the backup job to execute.
	 * @return The process id of the executed command. This is required by
	 *   the 'Exec' RPC for identification.
	 */
	public function execute($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get the rsync job configuration object
		$object = OMVRpc::exec("UsbBackup", "get", array(
		  "uuid" => $data['uuid']));
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Execute the backup job.
		$outputFile = tempnam(sys_get_temp_dir(), "usbbackupjob");
		$cmd = sprintf("sudo sh '%s/rsync-%s' >%s 2>&1 & echo $!",
		  $GLOBALS['OMV_USBBACKUP_SCRIPTS_DIR'], $data['uuid'], $outputFile);
		OMVUtil::exec($cmd, $output, $res);
		if ($res !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Remember pid and content filename in session.
		$pid = intval($output[0]);
		$_SESSION['execjob'][$pid] = array(
			"pid" => $pid,
			"outputFile" => $outputFile
		);
		return $pid;
	}
}
?>
