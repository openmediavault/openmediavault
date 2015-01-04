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
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceCron extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Cron";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getList");
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("delete");
		$this->registerMethod("execute");
	}

	/**
	 * Get the cron jobs of the requested type.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 *   \em type Array of cron jobs types which are reboot, shutdown,
	 *   standby or userdefined.
	 * @param context The context of the caller.
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
				  "sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'},
				  "type":{"type":"array","items":{"type":"string",'.
					'"format":"regex","pattern":"\/^reboot|shutdown|standby|userdefined$\/i"}}
			  }
		  }');
		// Get configuration data.
		$xpath = "//system/crontab/job";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Get cron jobs that match the given type.
		$jobs = array();
		foreach($objects as $objectk => $objectv) {
			if(in_array($objectv['type'], $params['type'])) {
				// Modify result data.
				$objectv['enable'] = boolvalEx($objectv['enable']);
				$objectv['sendemail'] = boolvalEx($objectv['sendemail']);
				$objectv['everynminute'] = boolvalEx($objectv['everynminute']);
				$objectv['everynhour'] = boolvalEx($objectv['everynhour']);
				$objectv['everyndayofmonth'] = boolvalEx(
				  $objectv['everyndayofmonth']);
				$jobs[] = $objectv;
			}
		}
		// Filter the result.
		return $this->applyFilter($jobs, $params['start'], $params['limit'],
		  $params['sortfield'], $params['sortdir']);
	}

	/**
	 * Get a cron job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function get($params, $context) {
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
		$xpath = sprintf("//system/crontab/job[uuid='%s']", $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the result object.
		$object['enable'] = boolvalEx($object['enable']);
		$object['sendemail'] = boolvalEx($object['sendemail']);
		$object['everynminute'] = boolvalEx($object['everynminute']);
		$object['everynhour'] = boolvalEx($object['everynhour']);
		$object['everyndayofmonth'] = boolvalEx($object['everyndayofmonth']);
		return $object;
	}

	/**
	 * Set (add/update) a cron job configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	public function set($params, $context) {
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
				  "execution":{"type":"string","enum":["exactly","hourly",'.
					'"daily","weekly","monthly","yearly","reboot"]},
				  "sendemail":{"type":"boolean"},
				  "type":{"type":["string","null"],"format":"regex",'.
					'"pattern":"\/^reboot|shutdown|standby|userdefined$\/i"},
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
		  }');
		// Prepare configuration data.
		$object = array(
			"uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $params['uuid'],
			"enable" => array_boolval($params, 'enable'),
			"execution" => $params['execution'],
			"sendemail" => array_boolval($params, 'sendemail'),
			"type" => $params['type'],
			"minute" => $params['minute'],
			"everynminute" => array_boolval($params, 'everynminute'),
			"hour" => $params['hour'],
			"everynhour" => array_boolval($params, 'everynhour'),
			"dayofmonth" => $params['dayofmonth'],
			"everyndayofmonth" => array_boolval($params, 'everyndayofmonth'),
			"month" => $params['month'],
			"dayofweek" => $params['dayofweek'],
			"username" => $params['username'],
			"command" => $params['command'],
			"comment" => $params['comment']
		);
		// Set the configuration data.
		$success = FALSE;
		if ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration.
			$success = $xmlConfig->set("//system/crontab",
			  array("job" => $object));
		} else {
			// Update existing configuration object.
			$success = $xmlConfig->replace(
			  "//system/crontab/job[uuid='{$params['uuid']}']", $object);
		}
		if (FALSE === $success)
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.crontab.job", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete a cron job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function delete($params, $context) {
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
		// Delete the configuration object.
		$xpath = sprintf("//system/crontab/job[uuid='%s']", $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.system.crontab.job");
	}

	/**
	 * Execute a cron job.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 */
	public function execute($params, $context) {
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
			// Get the cron job object.
			$object = $this->callMethod("get", array(
			  "uuid" => $params['uuid']), $context);
			// Execute the cron command line in a shell.
			$cmd = "/bin/su";
			$cmdArgs = array("-c", $object['command'], $object['username']);
			if(FALSE === $this->execve($cmd, $cmdArgs, $bgOutputFilename)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd." ".implode(" ", $cmdArgs), "");
			}
		} catch(Exception $e) {
			$this->finalizeBgProcStatus($bgStatusFilename, "", $e);
			exit(1);
		}
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceCron());
?>
