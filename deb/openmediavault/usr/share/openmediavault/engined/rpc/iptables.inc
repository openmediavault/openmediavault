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
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceIptables extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Iptables";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getRules");
		$this->registerMethod("setRules");
		$this->registerMethod("getRule");
		$this->registerMethod("setRule");
		$this->registerMethod("deleteRule");
	}

	/**
	 * Get all iptables rules.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A list of configuration objects.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getRules($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration objects.
		$xpath = "//system/network/iptables/rule";
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result object.
		foreach($objects as $objectk => &$objectv) {
			$objectv['rulenum'] = intval($objectv['rulenum']);
		}
		return $objects;
	}

	/**
	 * Set multiple iptables rules at one time.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration objects.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setRules($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"array",
			  "items":{
				  "type":"object",
				  "properties":{
					  "uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					  "rulenum":{"type":"integer"},
					  "chain":{"type":"string","enum":["INPUT","OUTPUT"]},
					  "action":{"type":"string","enum":["ACCEPT","REJECT",'.
						'"DROP","LOG"]},
					  "source":{"type":"string"},
					  "sport":{"type":"string"},
					  "destination":{"type":"string"},
					  "dport":{"type":"string"},
					  "protocol":{"type":"string","format":"regex",'.
						'"pattern":"\/^(!?(tcp|udp|icmp))|all$\/"},
					  "extraoptions":{"type":"string"},
					  "comment":{"type":"string"}
				  }
			  }
		  }');
		// Prepare the configuration object.
		$objects = array();
		foreach($params as $paramk => $paramv) {
			$objects[] = array(
				"uuid" => ($paramv['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
				  OMVUtil::uuid() : $paramv['uuid'],
				"rulenum" => $paramv['rulenum'],
				"chain" => $paramv['chain'],
				"action" => $paramv['action'],
				"source" => $paramv['source'],
				"sport" => $paramv['sport'],
				"destination" => $paramv['destination'],
				"dport" => $paramv['dport'],
				"protocol" => $paramv['protocol'],
				"extraoptions" => $paramv['extraoptions'],
				"comment" => $paramv['comment']
			);
		}
		// Update configuration objects.
		$success = FALSE;
		if(empty($objects)) {
			$success = $xmlConfig->replace("//system/network/iptables",
			  array());
		} else {
			$success = $xmlConfig->replace("//system/network/iptables",
			  array("rule" => $objects));
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.iptables.rule", $objects);
		// Return the configuration object.
		return $objects;
	}

	/**
	 * Get an iptables rule configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getRule($params, $context) {
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
		$xpath = sprintf("//system/network/iptables/rule[uuid='%s']",
		  $params['uuid']);
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result object.
		$object['rulenum'] = intval($object['rulenum']);
		return $object;
	}

	/**
	 * Set an iptables rule configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setRule($params, $context) {
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
				  "rulenum":{"type":"integer","minimum":1},
				  "chain":{"type":"string","enum":["INPUT","OUTPUT"]},
				  "action":{"type":"string","enum":["ACCEPT","REJECT",'.
					'"DROP","LOG"]},
				  "source":{"type":"string"},
				  "sport":{"type":"string"},
				  "destination":{"type":"string"},
				  "dport":{"type":"string"},
				  "protocol":{"type":"string","format":"regex",'.
					'"pattern":"\/^((! )?(tcp|udp|icmp))|all$\/"},
				  "extraoptions":{"type":"string"},
				  "comment":{"type":"string"}
			  }
		  }');
		// Prepare the configuration object.
		$object = array(
			  "uuid" => ($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
				OMVUtil::uuid() : $params['uuid'],
			  "rulenum" => $params['rulenum'],
			  "chain" => $params['chain'],
			  "action" => $params['action'],
			  "source" => $params['source'],
			  "sport" => $params['sport'],
			  "destination" => $params['destination'],
			  "dport" => $params['dport'],
			  "protocol" => $params['protocol'],
			  "extraoptions" => $params['extraoptions'],
			  "comment" => $params['comment']
		  );
		// Set the configuration object.
		$success = FALSE;
		if($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration.
			$success = $xmlConfig->set("//system/network/iptables",
			  array("rule" => $object));
		} else {
			// Update existing configuration object.
			$success = $xmlConfig->replace("//system/network/iptables/rule" .
			  "[uuid='{$params['uuid']}']", $object);
		}
		if(FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(($params['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
		  OMV_NOTIFY_CREATE : OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.iptables.rule", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Delete an iptables rule configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The deleted configuration object.
	 */
	function deleteRule($params, $context) {
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
		$xpath = sprintf("//system/network/iptables/rule[uuid='%s']",
		  $params['uuid']);
		return $this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.system.network.iptables.rule");
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceIptables());
?>
