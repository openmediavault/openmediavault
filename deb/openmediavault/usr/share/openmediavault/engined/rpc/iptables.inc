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
		$this->registerMethod("getRules6");
		$this->registerMethod("setRules6");
		$this->registerMethod("getRule");
		$this->registerMethod("setRule");
		$this->registerMethod("deleteRule");
	}

	/**
	 * Helper function to get all rules for the given family.
	 * @param family The rule family type, e.g. inet or inet6.
	 * @return The iptables rules of the given family.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	private function getRulesByFamily($family) {
		global $xmlConfig;
		// Get the configuration objects.
		$xpath = sprintf("//system/network/iptables/rule[family='%s']",
		  $family);
		$objects = $xmlConfig->getList($xpath);
		if(is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the result objects.
		foreach($objects as $objectk => &$objectv) {
			$objectv['rulenum'] = intval($objectv['rulenum']);
		}
		return $objects;
	}

	/**
	 * Get all IPv4 iptables rules.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A list of configuration objects.
	 */
	function getRules($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration objects.
		return $this->getRulesByFamily("inet");
	}

	/**
	 * Get all IPv6 iptables rules.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A list of configuration objects.
	 */
	function getRules6($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration objects.
		return $this->getRulesByFamily("inet6");
	}

	/**
	 * Set multiple iptables rules at one time.
	 * @param family The rule family type, e.g. inet or inet6.
	 * @param rules An array containing the iptables rules.
	 * @return An array containg the stored configuration objects.
	 * @throw E_CONFIG_OBJECT_NOT_FOUND
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	private function setRulesByFamily($family, $rules) {
		global $xmlConfig;
		// Prepare the configuration objects.
		$objects = array();
		foreach($rules as $rulek => $rulev) {
			$objects[] = array(
				"uuid" => ($rulev['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
				  OMVUtil::uuid() : $rulev['uuid'],
				"rulenum" => $rulev['rulenum'],
				"chain" => $rulev['chain'],
				"action" => $rulev['action'],
				"family" => $rulev['family'],
				"source" => $rulev['source'],
				"sport" => $rulev['sport'],
				"destination" => $rulev['destination'],
				"dport" => $rulev['dport'],
				"protocol" => $rulev['protocol'],
				"extraoptions" => $rulev['extraoptions'],
				"comment" => $rulev['comment']
			);
		}
		// Remove all existing rules for the given family.
		$xpath = sprintf("//system/network/iptables/rule[family='%s']",
		  $family);
		if(TRUE === $xmlConfig->exists($xpath)) {
			if(FALSE === $xmlConfig->delete($xpath)) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_NOT_FOUND,
				  $xpath);
			}
		}
		// Append the given iptables rules.
		if(!empty($objects)) {
			if(FALSE === $xmlConfig->set("//system/network/iptables",
			  array("rule" => $objects))) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
			}
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.iptables.rule", $objects);
		// Return the configuration object.
		return $objects;
	}

	/**
	 * Set multiple IPv4 iptables rules at one time.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containg the stored configuration objects.
	 */
	function setRules($params, $context) {
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
					  "family":{"type":"string","enum":["inet"]},
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
		// Store the given IPv4 iptables rules.
		return $this->setRulesByFamily("inet", $params);
	}

	/**
	 * Set multiple IPv6 iptables rules at one time.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containg the stored configuration objects.
	 */
	function setRules6($params, $context) {
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
					  "family":{"type":"string","enum":["inet6"]},
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
		// Store the given IPv6 iptables rules.
		return $this->setRulesByFamily("inet6", $params);
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
				  "family":{"type":"string","enum":["inet","inet6"]},
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
			  "family" => $params['family'],
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
