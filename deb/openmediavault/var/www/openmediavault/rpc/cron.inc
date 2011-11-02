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
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class CronRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getListByType" => array(
				'{"type":"integer"}', // start
				'{"type":"integer"}', // count
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'}', // sortField
				'{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}', // sortDir
				'{"type":"array","items":{"type":"string"}}' // types
			),
			"get" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"set" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"type":{"type":["string","null"],"format":"regex",'.
					  '"pattern":"\/^reboot|shutdown|userdefined$\/i"},
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
					"username":{"type":"string"},
					"command":{"type":"string"},
					"comment":{"type":"string"}
				}
			}'),
			"setRebootShutdown" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"type":{"type":["string","null"],"format":"regex",'.
					  '"pattern":"\/^reboot|shutdown$\/i"},
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
					"comment":{"type":"string"}
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
	 * Get the cron jobs of the requested types.
	 * @param types Array of cron jobs types which are reboot, shutdown
	 * or userdefined.
	 * @return An array of cron jobs of the requested types.
	 */
	public function getListByType($start, $count, $sortField, $sortDir,
	  $types) {
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
		// Get configuration data
		$xpath = "//system/crontab/job";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Get cron jobs that match the given type
		$jobs = array();
		foreach ($objects as $objectk => $objectv) {
			if (in_array($objectv['type'], $types)) {
				$jobs[] = $objectv;
			}
		}
		// Filter result
		return $this->applyFilter($jobs, $start, $count, $sortField, $sortDir);
	}

	/**
	 * Get a cron job
	 */
	public function get($uuid) {
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
		$xpath = "//system/crontab/job[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return $object;
	}

	/**
	 * Set (add/update) a cron job
	 */
	public function set($data) {
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
			"type" => $data['type'],
			"minute" => $data['minute'],
			"hour" => $data['hour'],
			"dayofmonth" => $data['dayofmonth'],
			"month" => $data['month'],
			"dayofweek" => $data['dayofweek'],
			"username" => $data['username'],
			"command" => $data['command'],
			"comment" => $data['comment']
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration
			$success = $xmlConfig->set("//system/crontab",
			  array("job" => $object));
		} else {
			// Update existing configuration object
			$success = $xmlConfig->replace(
			  "//system/crontab/job[uuid='{$data['uuid']}']", $object);
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
		  "org.openmediavault.system.crontab.job", $object);
	}

	/**
	 * Set (add/update) a reboot/shutdown cron job
	 */
	public function setRebootShutdown($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Set missing //system/crontab/job config object fields
		$data['username'] = "root";
		switch ($data['type']) {
		case "reboot":
			$data['command'] = "shutdown -r now";
			break;
		case "shutdown":
			$data['command'] = "shutdown -h -P now";
			break;
		}
		// Set the cron job using the generic set method
		$this->set($data);
	}

	/**
	 * Delete a cron job
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
		$xpath = "//system/crontab/job[uuid='{$uuid}']";
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
		  "org.openmediavault.system.crontab.job", $object);
	}

	/**
	 * Execute a cron job.
	 * @param uuid The UUID of the cron job to execute.
	 * @return The process id of the executed command. This is required by the
	 * 'Exec' RPC for identification.
	 */
	public function execute($uuid) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Get the cron job object
		$xpath = "//system/crontab/job[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
				$xpath);
		}
		// Execute the cron command line
		$outputFile = tempnam(sys_get_temp_dir(), "cronjob");
		$cmd = sprintf("sudo -u '%s' %s >%s 2>&1 & echo $!",
		  $object['username'], $object['command'], $outputFile);
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
