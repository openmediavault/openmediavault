<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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
class OMVRpcServiceZeroconf extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Zeroconf";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("get");
		$this->registerMethod("set");
	}

	/**
	 * Get the Zeroconf services configuration.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing the Zeroconf services configuration.
	 */
	function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		$result = [];
		// Get the list of registered modules.
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$modules = $moduleMngr->getModules();
		foreach ($modules as $modulev) {
			// Check if the module implements the required interface.
			if ($modulev instanceof \OMV\Engine\Module\IZeroconf) {
				$config = $modulev->getZeroconfConfig();
				// Try to get the Zeroconf service configuration object.
				$db = \OMV\Config\Database::getInstance();
				$object = $db->getByFilter("conf.service.zeroconf.service", [
					"operator" => "stringEquals",
					"arg0" => "id",
					"arg1" => $config['id']
				], 1);
				$object->add("title", "string");
				$object->set("title", $config['title']);
				// Append object to result.
				$result[] = $object->getAssoc();
			}
		}
		return $result;
	}

	/**
	 * Set the Zeroconf services configuration.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function set($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.zeroconf.set");
		// Prepare the configuration object.
		$objects = [];
		// Get the list of registered modules.
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$modules = $moduleMngr->getModules();
		foreach ($modules as $modulev) {
			// Check if the module implements the required interface.
			if ($modulev instanceof \OMV\Engine\Module\IZeroconf) {
				$config = $modulev->getZeroconfConfig();
				// Set default values.
				$object = new \OMV\Config\ConfigObject(
				  "conf.service.zeroconf.service");
				$object->setNew(); // Mark the service configuration as new.
				$object->set("id", $config['id']);
				$object->set("enable", FALSE);
				$object->set("name", $config['title']);
				// Process the RPC arguments.
				foreach ($params as $paramk => $paramv) {
					if ($paramv['id'] === $config['id']) {
						$object->set("enable", $paramv['enable']);
						$object->set("name", $paramv['name']);
						break;
					}
				}
				$objects[] = $object;
			}
		}
		// Remove all existing service configuration objects.
		$filter = [
			"operator" => "stringEquals",
			"arg0" => "id",
			"arg1" => "*"
		];
		$db = \OMV\Config\Database::getInstance();
		if ($db->exists("conf.service.zeroconf.service", $filter)) {
			$db->deleteByFilter("conf.service.zeroconf.service", $filter);
		}
		// Set the service configuration objects.
		$objectsAssoc = [];
		foreach ($objects as $objectk => $objectv) {
			$db->set($objectv);
			$objectsAssoc[] = $objectv->getAssoc();
		}
		// Notify the configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.service.zeroconf.services", $objectsAssoc);
		// Return the configuration object.
		return $objectsAssoc;
	}
}
