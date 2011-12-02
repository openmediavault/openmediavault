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
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class RsyncRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getList" => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}' // sortDir
			),
			"get" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"set" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"enable":{"type":"boolean"},
					"comment":{"type":"string"},
					"type":{"type":"string","enum":["local","remote"]},
					"srcsharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].
					  ',"optional":true},
					"srcuri":{"type":"string","optional":true},
					"destsharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].
					  ',"optional":true},
					"desturi":{"type":"string","optional":true},
					"minute":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[0-9]|1[0-9]|2[0-9]|3[0-9]|4[0-9]|'.
					  '5[0-9]|[*]$\/"},
					"hour":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[0-9]|1[0-9]|2[0-3]|[*]$\/"},
					"month":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[1-9]|1[0-2]|[*]$\/"},
					"dayofmonth":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[1-9]|1[0-9]|2[0-9]|3[0-1]|[*]$\/"},
					"dayofweek":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[1-7]|[*]$\/"},
					"recursive":{"type":"boolean"},
					"times":{"type":"boolean"},
					"compress":{"type":"boolean"},
					"archive":{"type":"boolean"},
					"delete":{"type":"boolean"},
					"quiet":{"type":"boolean"},
					"perms":{"type":"boolean"},
					"acls":{"type":"boolean"},
					"xattrs":{"type":"boolean"},
					"extraoptions":{"type":"string"},
					"mode":{"type":"string","enum":["push","pull"],'.
					  '"optional":true}
				}
			}'),
			"delete" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"execute" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			)
		);
	}

	/**
	 * Get list of rsync job configuration objects.
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
		// Get configuration object
		$xpath = "//services/rsync/jobs/job";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional informations
		foreach ($objects as $objectk => &$objectv) {
			$objectv['srcname'] = gettext("n/a");
			$objectv['destname'] = gettext("n/a");
			switch ($objectv['type']) {
			case "local":
				$sfobject = $xmlConfig->get("//system/shares/sharedfolder[" .
				  "uuid='{$objectv['src']}']");
				if (!is_null($sfobject) && !empty($sfobject)) {
					$objectv['srcname'] = $sfobject['name'];
				}
				$sfobject = $xmlConfig->get("//system/shares/sharedfolder[" .
				  "uuid='{$objectv['dest']}']");
				if (!is_null($sfobject) && !empty($sfobject)) {
					$objectv['destname'] = $sfobject['name'];
				}
				break;
			case "remote":
				switch ($objectv['mode']) {
				case "push":
					$sfobject = $xmlConfig->get("//system/shares/" .
					  "sharedfolder[uuid='{$objectv['src']}']");
					if (!is_null($sfobject) && !empty($sfobject)) {
						$objectv['srcname'] = $sfobject['name'];
					}
					$objectv['destname'] = $objectv['dest'];
					break;
				case "pull":
					$objectv['srcname'] = $objectv['src'];
					$sfobject = $xmlConfig->get("//system/shares/" .
					  "sharedfolder[uuid='{$objectv['dest']}']");
					if (!is_null($sfobject) && !empty($sfobject)) {
						$objectv['destname'] = $sfobject['name'];
					}
					break;
				}
				break;
			}
			// Modify result data
			$objectv['enable'] = boolval($objectv['enable']);
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
		// Filter result
		return $this->applyFilter($objects, $start, $count, $sortField,
		  $sortDir);
	}

	/**
	 * Get a rsync job configuration object.
	 */
	function get($uuid) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object
		$xpath = "//services/rsync/jobs/job[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional informations
		switch ($object['type']) {
		case "local":
			$object['srcsharedfolderref'] = $object['src'];
			$object['destsharedfolderref'] = $object['dest'];
		case "remote":
			switch ($object['mode']) {
			case "push":
				$object['srcsharedfolderref'] = $object['src'];
				$object['desturi'] = $object['dest'];
				break;
			case "pull":
				$object['srcuri'] = $object['src'];
				$object['destsharedfolderref'] = $object['dest'];
				break;
			}
			break;
		}
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['recursive'] = boolval($object['recursive']);
		$object['times'] = boolval($object['times']);
		$object['compress'] = boolval($object['compress']);
		$object['archive'] = boolval($object['archive']);
		$object['delete'] = boolval($object['delete']);
		$object['quiet'] = boolval($object['quiet']);
		$object['perms'] = boolval($object['perms']);
		$object['acls'] = boolval($object['acls']);
		$object['xattrs'] = boolval($object['xattrs']);
		return $object;
	}

	/**
	 * Set a rsync job configuration object.
	 */
	function set($data) {
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
			"enable" => array_boolval($data, 'enable'),
			"comment" => $data['comment'],
			"type" => $data['type'],
			"src" => ($data['type'] === "local") ?
			  $data['srcsharedfolderref'] : (($data['mode'] === "push") ?
				$data['srcsharedfolderref'] : $data['srcuri']),
			"dest" => ($data['type'] === "local") ?
			  $data['destsharedfolderref'] : (($data['mode'] === "push") ?
				$data['desturi'] : $data['destsharedfolderref']),
			"minute" => $data['minute'],
			"hour" => $data['hour'],
			"dayofmonth" => $data['dayofmonth'],
			"month" => $data['month'],
			"dayofweek" => $data['dayofweek'],
			"recursive" => array_boolval($data, 'recursive'),
			"times" => array_boolval($data, 'times'),
			"compress" => array_boolval($data, 'compress'),
			"archive" => array_boolval($data, 'archive'),
			"delete" => array_boolval($data, 'delete'),
			"quiet" => array_boolval($data, 'quiet'),
			"perms" => array_boolval($data, 'perms'),
			"acls" => array_boolval($data, 'acls'),
			"xattrs" => array_boolval($data, 'xattrs'),
			"extraoptions" => $data['extraoptions'],
			"mode" => isset($data['mode']) ? $data['mode'] : ""
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration
			$success = $xmlConfig->set("//services/rsync/jobs",
			  array("job" => $object));
		} else {
			// Update existing configuration object
			$success = $xmlConfig->replace("//services/rsync/jobs/job[" .
			  "uuid='{$data['uuid']}']", $object);
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
		  "org.openmediavault.services.rsync.jobs.job", $object);
	}

	/**
	 * Delete a rsync job configuration object.
	 */
	public function delete($uuid) {
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
		// Delete configuration object
		$xpath = "//services/rsync/jobs/job[uuid='{$uuid}']";
		if (FALSE === ($object = $xmlConfig->delete($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
			  $xpath);
		}
		// Save configuration
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.rsync.jobs.job", $object);
	}

	/**
	 * Execute a rsync job.
	 * @param uuid The UUID of the cron job to execute.
	 * @return The process id of the executed command. This is required by the
	 * 'Exec' RPC for identification.
	 */
	public function execute($uuid) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get the rsync job configuration object
		$object = OMVRpc::exec("Rsync", "get", array($uuid));
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}
		// Execute the cron command line
		$outputFile = tempnam(sys_get_temp_dir(), "rsyncjob");
		$cmd = sprintf("sudo sh '%s/rsync-%s' >%s 2>&1 & echo $!",
		  $GLOBALS['OMV_RSYNC_CRONSCRIPTS_DIR'], $uuid, $outputFile);
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
}
?>
