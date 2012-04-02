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
require_once("openmediavault/functions.inc");
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/system.inc");
require_once("openmediavault/product.inc");
require_once("openmediavault/notify.inc");

class WebGUIRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"setSettings" => array('{
				"type":"object",
				"properties":{
					"port":{"type":"integer","minimum":1,"maximum":65535},
					"enablessl":{"type":"boolean"},
					"sslport":{"type":"integer","minimum":1,"maximum":65535},
					"forcesslonly":{"type":"boolean"},
					"sslcertificateref":{'.
					  $GLOBALS['OMV_JSONSCHEMA_UUID_NONE'].'},
					"timeout":{"type":"integer","minimum":1,"maximum":30}
				}
			}'),
			"setPassword" => array('{
				"type":"object",
				"properties":{
					"password":{"type":"string"}
				}
			}')
		);
	}

	/**
	 * Get settings.
	 */
	function getSettings() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//webadmin";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['port'] = intval($object['port']);
		$object['enablessl'] = boolval($object['enablessl']);
		$object['sslport'] = intval($object['sslport']);
		$object['forcesslonly'] = boolval($object['forcesslonly']);
		$object['timeout'] = intval($object['timeout']);
		return $object;
	}

	/**
	 * Set settings.
	 */
	function setSettings($data) {
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
		// Prepare configuration objects
		$object = array(
			"port" => $data['port'],
			"enablessl" => array_boolval($data, 'enablessl'),
			"sslport" => $data['sslport'],
			"forcesslonly" => array_boolval($data, 'forcesslonly'),
			"sslcertificateref" => $data['sslcertificateref'],
			"timeout" => $data['timeout']
		);
		// Set configuration object
		$xpath = "//webadmin";
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
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
		  "org.openmediavault.webadmin", $object);
	}

	/**
	 * Set the administrator password.
	 * @param data An array containing the following fields:
	 *   \em password The new password.
	 * @return None
	 */
	function setPassword($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Modify user password
		$cmd = sprintf("sudo usermod --password %s admin 2>&1",
		  escapeshellarg(crypt($data['password'])));
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}
}
?>
