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
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class IptablesRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"setRules" => array('{
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
			}'),
			"getRule" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			),
			"setRule" => array('{
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
			}'),
			"deleteRule" => array(
				'{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}'
			)
		);
	}

	/**
	 * Get all iptables rules.
	 */
	function getRules() {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//system/network/iptables/rule";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		foreach ($objects as $objectk => &$objectv) {
			$objectv['rulenum'] = intval($objectv['rulenum']);
		}
		return $objects;
	}

	/**
	 * Set multiple iptables rules at one time.
	 */
	function setRules($data) {
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
		$objects = array();
		foreach ($data as $datak => $datav) {
			$objects[] = array(
				"uuid" => ($datav['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
				  OMVUtil::uuid() : $datav['uuid'],
				"rulenum" => $datav['rulenum'],
				"chain" => $datav['chain'],
				"action" => $datav['action'],
				"source" => $datav['source'],
				"sport" => $datav['sport'],
				"destination" => $datav['destination'],
				"dport" => $datav['dport'],
				"protocol" => $datav['protocol'],
				"extraoptions" => $datav['extraoptions'],
				"comment" => $datav['comment']
			);
		}
		// Update configuration objects
		$success = FALSE;
		if (empty($objects)) {
			$success = $xmlConfig->replace("//system/network/iptables",
			  array());
		} else {
			$success = $xmlConfig->replace("//system/network/iptables",
			  array("rule" => $objects));
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
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.network.iptables.rule", $objects);
	}

	/**
	 * Get an iptables rule.
	 */
	function getRule($uuid) {
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
		$xpath = "//system/network/iptables/rule[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['rulenum'] = intval($object['rulenum']);
		return $object;
	}

	/**
	 * Set an iptables rule.
	 */
	function setRule($data) {
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
			"rulenum" => $data['rulenum'],
			"chain" => $data['chain'],
			"action" => $data['action'],
			"source" => $data['source'],
			"sport" => $data['sport'],
			"destination" => $data['destination'],
			"dport" => $data['dport'],
			"protocol" => $data['protocol'],
			"extraoptions" => $data['extraoptions'],
			"comment" => $data['comment']
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration
			$success = $xmlConfig->set("//system/network/iptables",
			  array("rule" => $object));
		} else {
			// Update existing configuration object
			$success = $xmlConfig->replace("//system/network/iptables/rule" .
			  "[uuid='{$data['uuid']}']", $object);
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
		  "org.openmediavault.system.network.iptables.rule", $object);
	}

	/**
	 * Delete an iptables rule.
	 */
	function deleteRule($uuid) {
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
		// Get the object
		$xpath = "//system/network/iptables/rule[uuid='{$uuid}']";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Notify configuration changes
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.system.network.iptables.rule", $object);
		// Delete configuration object
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
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.system.network.iptables.rule", $object);
	}
}
?>
