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
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceZeroconf extends OMVRpcServiceAbstract {
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
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function get($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		$result = array();
		// Get the list of registered modules.
		$moduleMgr = &OMVModuleMgr::getInstance();
		$modules = $moduleMgr->getModules();
		foreach($modules as $modulev) {
			// Check if the module implements the OMVIModuleZeroconf
			// interface.
			if($modulev instanceof OMVIModuleZeroconf) {
				$config = $modulev->getZeroconfConfig();
				// Try to get the Zeroconf service configuration object.
				$xpath = sprintf("//services/zeroconf/services/".
				  "service[id='%s']", $config['id']);
				$object = $xmlConfig->get($xpath);
				if(is_null($object)) {
					// Set default values.
					$object = array(
						  "id" => $config['id'],
						  "enable" => FALSE,
						  "name" => sprintf("%%h - %s", $config['title'])
					  );
				}
				$object['title'] = $config['title'];
				// Append object to result.
				$result[] = $object;
			}
		}
		return $result;
	}

	/**
	 * Set the Zeroconf services configuration.
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
			  "type":"array",
			  "items":{
				  "type":"object",
				  "properties":{
					  "id":{"type":"string"},
					  "name":{"type":"string"},
					  "enable":{"type":"boolean"}
				  }
			  }
		  }');
		// Prepare the configuration object.
		$object = array();
		// Get the list of registered modules.
		$moduleMgr = &OMVModuleMgr::getInstance();
		$modules = $moduleMgr->getModules();
		foreach($modules as $modulev) {
			// Check if the module implements the OMVIModuleZeroconf
			// interface.
			if($modulev instanceof OMVIModuleZeroconf) {
				$config = $modulev->getZeroconfConfig();
				// Set default values.
				$service = array(
					  "id" => $config['id'],
					  "enable" => FALSE,
					  "name" => $config['title']
				  );
				foreach($params as $paramk => $paramv) {
					if($paramv['id'] === $config['id']) {
						$service = array_merge($service, array(
							  "enable" => array_boolval($paramv, 'enable'),
							  "name" => $paramv['name']
						  ));
						break;
					}
				}
				$object[] = $service;
			}
		}
		// Set the configuration object.
		$xpath = "//services/zeroconf/services";
		if(FALSE === $xmlConfig->replace($xpath, array(
		  "service" => $object))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify the configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.zeroconf.services", $object);
		// Return the configuration object.
		return $object;
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceZeroconf());
?>
