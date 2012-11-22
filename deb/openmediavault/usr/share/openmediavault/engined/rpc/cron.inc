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
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class CronRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'},
					"type":{"type":"array","items":{"type":"string",'.
					  '"format":"regex","pattern":"\/^reboot|shutdown|userdefined$\/i"}}
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
					"sendemail":{"type":"boolean"},
					"type":{"type":["string","null"],"format":"regex",'.
					  '"pattern":"\/^reboot|shutdown|userdefined$\/i"},
					"minute":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[0-9]|1[0-9]|2[0-9]|3[0-9]|4[0-9]|'.
					  '5[0-9]|[*]$\/"},
					"everynminute":{"type":"boolean"},
					"hour":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[0-9]|1[0-9]|2[0-3]|[*]$\/"},
					"everynhour":{"type":"boolean"},
					"month":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[1-9]|1[0-2]|[*]$\/"},
					"dayofmonth":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[1-9]|1[0-9]|2[0-9]|3[0-1]|[*]$\/"},
					"everyndayofmonth":{"type":"boolean"},
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
					"enable":{"type":"boolean"},
					"type":{"type":["string","null"],"format":"regex",'.
					  '"pattern":"\/^reboot|shutdown$\/i"},
					"minute":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[0-9]|1[0-9]|2[0-9]|3[0-9]|4[0-9]|'.
					  '5[0-9]|[*]$\/"},
					"everynminute":{"type":"boolean"},
					"hour":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[0-9]|1[0-9]|2[0-3]|[*]$\/"},
					"everynhour":{"type":"boolean"},
					"month":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[1-9]|1[0-2]|[*]$\/"},
					"dayofmonth":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[1-9]|1[0-9]|2[0-9]|3[0-1]|[*]$\/"},
					"everyndayofmonth":{"type":"boolean"},
					"dayofweek":{"type":"string","format":"regex",'.
					  '"pattern":"\/^[1-7]|[*]$\/"},
					"comment":{"type":"string"}
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
	 * Get the cron jobs of the requested type.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 *   \em type Array of cron jobs types which are reboot, shutdown
	 *   or userdefined.
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
			if (in_array($objectv['type'], $data['type'])) {
				// Modify result data
				$objectv['enable'] = boolval($objectv['enable']);
				$objectv['sendemail'] = boolval($objectv['sendemail']);
				$objectv['everynminute'] = boolval($objectv['everynminute']);
				$objectv['everynhour'] = boolval($objectv['everynhour']);
				$objectv['everyndayofmonth'] = boolval(
				  $objectv['everyndayofmonth']);
				$jobs[] = $objectv;
			}
		}
		// Filter result
		return $this->applyFilter($jobs, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get a cron job configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The requested configuration object.
	 */
	public function get($data) {
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
		$xpath = sprintf("//system/crontab/job[uuid='%s']", $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['sendemail'] = boolval($object['sendemail']);
		$object['everynminute'] = boolval($object['everynminute']);
		$object['everynhour'] = boolval($object['everynhour']);
		$object['everyndayofmonth'] = boolval($object['everyndayofmonth']);
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
			"enable" => array_boolval($data, 'enable'),
			"sendemail" => array_boolval($data, 'sendemail'),
			"type" => $data['type'],
			"minute" => $data['minute'],
			"everynminute" => array_boolval($data, 'everynminute'),
			"hour" => $data['hour'],
			"everynhour" => array_boolval($data, 'everynhour'),
			"dayofmonth" => $data['dayofmonth'],
			"everyndayofmonth" => array_boolval($data, 'everyndayofmonth'),
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
		$data['sendemail'] = FALSE;
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
	 * Delete a cron job configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return None
	 */
	public function delete($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions.
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters.
		$this->validateParams(__METHOD__, func_get_args());
		// Delete the configuration object.
		$xpath = sprintf("//system/crontab/job[uuid='%s']", $data['uuid']);
		$this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.system.crontab.job");
	}

	/**
	 * Execute a cron job.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The process ID of the executed command. This is required by the
	 *   'Exec' RPC for identification.
	 */
	public function execute($data) {
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
		$xpath = sprintf("//system/crontab/job[uuid='%s']", $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Execute the cron command line in a shell. The command must be
		// escaped to ensure the command line is correctly intepreted as
		// defined by the user. Arrrggg, what a command line.
		// Test:
		// Create a cron job with the command 'whoami' and run it from WebGUI.
		$outputFile = tempnam(sys_get_temp_dir(), "cronjob");
//		$cmd = sprintf("sudo -- sudo -u %s -- %s >%s 2>&1 & echo $!",
//		  $object['username'], $object['command'], $outputFile);
//		$cmd = sprintf("sudo -- su %s sh -c %s >%s 2>&1 & echo $!",
//		  $object['username'], escapeshellarg($object['command']),
//		  $outputFile);
		$cmd = sprintf("sudo -- sudo -u %s sh -c %s >%s 2>&1 & echo $!",
		  $object['username'], escapeshellarg($object['command']),
		  $outputFile);
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
