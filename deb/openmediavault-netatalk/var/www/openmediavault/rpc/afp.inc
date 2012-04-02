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

class AFPRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"setSettings" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"allowguests":{"type":"boolean"},
					"extraoptions":{"type":"string"}
				}
			}'),
			"getShareList" => array('{
				"type":"object",
				"properties":{
					"start":{"type":"integer"},
					"limit":{'.$GLOBALS['OMV_JSONSCHEMA_COUNTFIELD'].'},
					"sortfield":{'.$GLOBALS['OMV_JSONSCHEMA_SORTFIELD'].'},
					"sortdir":{'.$GLOBALS['OMV_JSONSCHEMA_SORTDIR'].'}
				}
			}'),
			"getShare" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}'),
			"setShare" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID_UNDEFINED'].'},
					"sharedfolderref":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'},
					"name":{'.$GLOBALS['OMV_JSONSCHEMA_SHARENAME'].'},
					"comment":{"type":"string"},
					"password":{"type":"string"},
					"casefold":{"type":"string","enum":["none","tolower",'.
					  '"toupper","xlatelower","xlateupper"]},
					"ro":{"type":"boolean"},
					"upriv":{"type":"boolean"},
					"usedots":{"type":"boolean"},
					"invisibledots":{"type":"boolean"},
					"tm":{"type":"boolean"},
					"mswindows":{"type":"boolean"},
					"extraoptions":{"type":"string"}
				}
			}'),
			"deleteShare" => array('{
				"type":"object",
				"properties":{
					"uuid":{'.$GLOBALS['OMV_JSONSCHEMA_UUID'].'}
				}
			}')
		);
	}

	public function getSettings() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/afp";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		unset($object['shares']);
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['allowguests'] = boolval($object['allowguests']);
		return $object;
	}

	public function setSettings($data) {
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
		// Get existing configuration object
		$xpath = "//services/afp";
		$oldObject = $xmlConfig->get($xpath);
		if (is_null($oldObject)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update configuration
		$object = array(
			"enable" => array_boolval($data, 'enable'),
			"allowguests" => array_boolval($data, 'allowguests'),
			"extraoptions" => $data['extraoptions'],
			"shares" => isset($oldObject['shares']) ?
			  $oldObject['shares'] : array()
		);
		if (FALSE === $xmlConfig->replace("//services/afp", $object)) {
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
		  "org.openmediavault.services.afp", $object);
	}

	/**
	 * Get list of share configuration objects.
	 * @param data An array containing the following fields:
	 *   \em start The index where to start.
	 *   \em limit The number of objects to process.
	 *   \em sortfield The name of the column used to sort.
	 *   \em sortdir The sort direction, ASC or DESC.
	 * @return An array containing the requested objects. The field \em total
	 *   contains the total number of objects, \em data contains the object
	 *   array. An exception will be thrown in case of an error.
	 */
	public function getShareList($data) {
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
		$xpath = "//services/afp/shares/share";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Add additional share informations
		foreach ($objects as $objectk => &$objectv) {
			$objectv['sharedfoldername'] = gettext("n/a");
			// Modify result data
			foreach ($objectv['options'] as $optionk => $optionv) {
				$objectv['options'][$optionk] = boolval($optionv);
			}
			// Get the shared folder configuration object
			$sfobject = $xmlConfig->get("//system/shares/sharedfolder[" .
			  "uuid='{$objectv['sharedfolderref']}']");
			if (!is_null($sfobject) && !empty($sfobject)) {
				$objectv['sharedfoldername'] = $sfobject['name'];
			}
		}
		// Filter result
		return $this->applyFilter($objects, $data['start'], $data['limit'],
		  $data['sortfield'], $data['sortdir']);
	}

	/**
	 * Get a share configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return The requested configuration object.
	 */
	public function getShare($data) {
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
		$xpath = sprintf("//services/afp/shares/share[uuid='%s']",
		  $data['uuid']);
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		foreach ($object['options'] as $optionk => $optionv) {
			$object[$optionk] = boolval($optionv);
		}
		unset($object['options']);
		return $object;
	}

	/**
	 * Set a share configuration object.
	 */
	public function setShare($data) {
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
			"sharedfolderref" => $data['sharedfolderref'],
			"name" => $data['name'],
			"comment" => $data['comment'],
			"password" => $data['password'],
			"casefold" => $data['casefold'],
			"options" => array(
				"ro" => array_boolval($data, 'ro'),
				"upriv" => array_boolval($data, 'upriv'),
				"usedots" => array_boolval($data, 'usedots'),
				"invisibledots" => array_boolval($data, 'invisibledots'),
				"tm" => array_boolval($data, 'tm'),
				"mswindows" => array_boolval($data, 'mswindows')
			),
			"extraoptions" => $data['extraoptions']
		);
		// Set configuration data
		$success = FALSE;
		if ($data['uuid'] == $GLOBALS['OMV_UUID_UNDEFINED']) {
			// Append object to configuration
			$success = $xmlConfig->set("//services/afp/shares",
			  array("share" => $object));
		} else {
			// Update existing configuration object
			$success = $xmlConfig->replace("//services/afp/shares/share[" .
			  "uuid='{$data['uuid']}']", $object);
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
		  "org.openmediavault.services.afp.shares.share", $object);
	}

	/**
	 * Delete a share configuration object.
	 * @param data An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @return None
	 */
	public function deleteShare($data) {
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
		// Delete configuration object
		$xpath = sprintf("//services/afp/shares/share[uuid='%s']",
		  $data['uuid']);
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
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.afp.shares.share", $object);
	}
}
?>
