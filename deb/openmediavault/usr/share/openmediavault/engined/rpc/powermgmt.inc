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
require_once("openmediavault/functions.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServicePowerMgmt extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "PowerMgmt";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("getScheduleList");
		$this->registerMethod("getScheduledJob");
		$this->registerMethod("setScheduledJob");
		$this->registerMethod("deleteScheduledJob");
		$this->registerMethod("executeScheduledJob");
	}

	/**
	 * Get the power management configuration settings.
	 * @param params The method parameters.
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
		// Get the configuration object.
		$xpath = "//system/powermanagement";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data.
		$object['cpufreq'] = boolvalEx($object['cpufreq']);
		return $object;
	}

	/**
	 * Set the power management configuration settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
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
				  "cpufreq":{"type":"boolean"},
				  "powerbtn":{"type":"string","enum":["nothing","shutdown",'.
					'"standby"]}
			  }
		  }');
		// Update the configuration object.
		$object = array(
			"cpufreq" => array_boolval($params, "cpufreq"),
			"powerbtn" => $params['powerbtn']
		);
		// Set the configuration object.
		$success = $xmlConfig->replace("//system/powermanagement", $object);
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.powermanagement", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Get the reboot/shutdown/standby cron jobs.
	 * @param params An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 *   \em type Array of cron jobs types which are reboot, shutdown
	 *   or userdefined.
	 * @param context The context of the caller.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getScheduleList($params, $context) {
		return OMVRpc::exec("cron", "getList", $params, $context);
	}

	/**
	 * Get a reboot/shutdown/standby cron job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	public function getScheduledJob($params, $context) {
		return OMVRpc::exec("cron", "get", $params, $context);
	}

	/**
	 * Set a reboot/shutdown/standby cron configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The configuration object.
	 */
	public function setScheduledJob($params, $context) {
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
				  "type":{"type":["string","null"],"format":"regex",'.
					'"pattern":"\/^reboot|shutdown|standby$\/i"},
				  "execution":{"type":"string","enum":["exactly","hourly",'.
					'"daily","weekly","monthly","yearly"]},
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
		  }');
		// Set missing configuration object fields.
		$params['username'] = "root";
		$params['sendemail'] = FALSE;
		switch ($params['type']) {
		case "reboot":
			$params['command'] = "shutdown -r now";
			break;
		case "shutdown":
			$params['command'] = "shutdown -h -P now";
			break;
		case "standby":
			$params['command'] = "omv-pm-standby";
			break;
		}
		return OMVRpc::exec("cron", "set", $params, $context);
	}

	/**
	 * Delete a reboot/shutdown/standby cron job configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	public function deleteScheduledJob($params, $context) {
		return OMVRpc::exec("cron", "delete", $params, $context);
	}

	/**
	 * Execute a reboot/shutdown/standby cron job.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The name of the background process status file.
	 */
	public function executeScheduledJob($params, $context) {
		return OMVRpc::exec("cron", "execute", $params, $context);
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServicePowerMgmt());
?>
