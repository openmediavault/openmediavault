<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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
namespace Engined\Rpc;

class Iptables extends \OMV\Rpc\ServiceAbstract {
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
	 */
	private function getRulesByFamily($family) {
		// Get the configuration objects.
		$db = \OMV\Config\Database::getInstance();
		return $db->getByFilterAssoc("conf.system.network.iptables.rule", [
			"operator" => "stringEquals",
			"arg0" => "family",
			"arg1" => $family
		]);
	}

	/**
	 * Get all IPv4 iptables rules.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A list of configuration objects.
	 */
	function getRules($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the configuration objects.
		return $this->getRulesByFamily("inet6");
	}

	/**
	 * Set multiple iptables rules at one time.
	 * @param family The rule family type, e.g. inet or inet6.
	 * @param rules An array containing the iptables rules.
	 * @return An array containg the stored configuration objects.
	 */
	private function setRulesByFamily($family, $rules) {
		// Prepare the configuration objects.
		$objects = [];
		foreach ($rules as $rulek => $rulev) {
			$object = new \OMV\Config\ConfigObject(
			  "conf.system.network.iptables.rule");
			$object->setAssoc($rulev);
			$object->setNew(); // Mark the rule as new.
			$objects[] = $object;
		}
		// Remove all existing rules for the specified family.
		$filter = [
			"operator" => "stringEquals",
			"arg0" => "family",
			"arg1" => $family
		];
		$db = \OMV\Config\Database::getInstance();
		if ($db->exists("conf.system.network.iptables.rule", $filter)) {
			$db->deleteByFilter("conf.system.network.iptables.rule", $filter);
		}
		// Append the new iptables rules.
		$objectsAssoc = [];
		foreach ($objects as $objectk => $objectv) {
			$db->set($objectv);
			$objectsAssoc[] = $objectv->getAssoc();
		}
		// Return the configuration objects.
		return $objectsAssoc;
	}

	/**
	 * Set multiple IPv4 iptables rules at one time.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containg the stored configuration objects.
	 */
	function setRules($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.iptables.setrules");
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.iptables.setrules6");
		// Store the given IPv6 iptables rules.
		return $this->setRulesByFamily("inet6", $params);
	}

	/**
	 * Get an iptables rule configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	function getRule($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		return $db->getAssoc("conf.system.network.iptables.rule",
		  $params['uuid']);
	}

	/**
	 * Set an iptables rule configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function setRule($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.iptables.setrule");
		// Append rule to the end of the existing rules if 'rulenum'
		// is set to -1.
		if (-1 == $params['rulenum']) {
			$objects = $this->getRulesByFamily($params['family']);
			$params['rulenum'] = count($objects);
		}
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject(
			"conf.system.network.iptables.rule");
		$object->setAssoc($params);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		// Delete the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.network.iptables.rule",
		  $params['uuid']);
		$db->delete($object);
		// Return the deleted configuration object.
		return $object->getAssoc();
	}
}
