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
require_once("openmediavault/system.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class NetworkRouteRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"getList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
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
					"network":{"type":"string"},
					"gateway":{"type":"string"},
					"comment":{"type":"string"}
				}
			}'),
			"delete" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}')
		);
	}

	/**
	 * Get list of static route configuration objects.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
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
		// Get configuration object
		$xpath = "//services/staticroutes/route";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Filter result
		return $this->applyFilter($objects, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get a static route configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The requested configuration object.
	 */
	function get($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions.
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters.
		$this->validateParams(__METHOD__, func_get_args());
		// Get configuration object.
		$xpath = sprintf("//services/staticroutes/route[uuid='%s']",
		  $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		return $object;
	}

	/**
	 * Set a static route configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 *   \em network The IP or network address.
	 *   \em gateway The IP address of the gateway.
	 *   \em comment Any text string.
	 * @return None
	 */
	function set($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions.
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters.
		$this->validateParams(__METHOD__, func_get_args());
		// Get the configuration object.
		$xpath = sprintf("//services/staticroutes/route[uuid='%s']",
		  $data['uuid']);
		$oldObject = $xmlConfig->get($xpath);
		// Set configuration data.
		$object = array(
			"uuid" => ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) ?
			  OMVUtil::uuid() : $data['uuid'],
			"network" => $data['network'],
			"gateway" => $data['gateway'],
			"comment" => $data['comment']
		);
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Perform some checks. These could be improved somewhen, e.g.
			// checking if a network fits into an already configured route.
			if (TRUE === $xmlConfig->exists("//services/staticroutes/route[" .
			  "network='{$data['network']}']")) {
				throw new OMVException(OMVErrorMsg::E_CONFIG_OBJECT_UNIQUENESS,
				  gettext("A route with the network already exists"));
			}
			$success = $xmlConfig->set("//services/staticroutes",
			  array("route" => $object));
		} else {
			// Usually some checks should be performed here too, but
			// the 'Edit' button is disabled in the WebGUI to make it
			// easier.
			// Update existing configuration object.
			$success = $xmlConfig->replace($xpath, $object);
		}
		if (FALSE === $success) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			$dispatcher->notify(OMV_NOTIFY_PRECREATE,
			  "org.openmediavault.services.staticroutes.route", $object);
		} else {
			$dispatcher->notify(OMV_NOTIFY_PREMODIFY,
			  "org.openmediavault.services.staticroutes.route", $object,
			  $oldObject);
		}
		// Save configuration.
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes.
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			$dispatcher->notify(OMV_NOTIFY_CREATE,
			  "org.openmediavault.services.staticroutes.route", $object);
		} else {
			$dispatcher->notify(OMV_NOTIFY_MODIFY,
			  "org.openmediavault.services.staticroutes.route", $object,
			  $oldObject);
		}
	}

	/**
	 * Delete a static route configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return None
	 */
	function delete($data) {
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
		$xpath = sprintf("//services/staticroutes/route[uuid='%s']",
		  $data['uuid']);
		$this->deleteConfigObjectByPath($xpath,
		  "org.openmediavault.services.staticroutes.route");
	}
}
?>
