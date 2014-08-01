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
require_once("openmediavault/functions.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceWebGui extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "WebGui";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
		$this->registerMethod("setPassword");
	}

	/**
	 * Get settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw 
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$xpath = "//webadmin";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the result object.
		$object['port'] = intval($object['port']);
		$object['enablessl'] = boolvalEx($object['enablessl']);
		$object['sslport'] = intval($object['sslport']);
		$object['forcesslonly'] = boolvalEx($object['forcesslonly']);
		$object['timeout'] = intval($object['timeout']);
		return $object;
	}

	/**
	 * Set settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 * @throw E_CONFIG_SET_OBJECT_FAILED
	 */
	function setSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "port":{"type":"integer","minimum":1,"maximum":65535},
				  "enablessl":{"type":"boolean"},
				  "sslport":{"type":"integer","minimum":1,"maximum":65535},
				  "forcesslonly":{"type":"boolean"},
				  "sslcertificateref":{'.
					$GLOBALS['OMV_JSONSCHEMA_UUID_NONE'].'},
				  "timeout":{"type":"integer","minimum":0,"maximum":30}
			  }
		  }');
		// Prepare configuration objects
		$object = array(
			"port" => $params['port'],
			"enablessl" => array_boolval($params, 'enablessl'),
			"sslport" => $params['sslport'],
			"forcesslonly" => array_boolval($params, 'forcesslonly'),
			"sslcertificateref" => $params['sslcertificateref'],
			"timeout" => $params['timeout']
		);
		// Set the configuration object.
		$xpath = "//webadmin";
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY, "org.openmediavault.webadmin",
		  $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Set the administrator password.
	 * @param params An array containing the following fields:
	 *   \em password The new password.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_FAILED
	 */
	function setPassword($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "password":{"type":"string"}
			  }
		  }');
		// Modify user password.
		$cmd = sprintf("usermod --password %s admin 2>&1",
		  escapeshellarg(crypt($params['password'])));
		if(0 !== $this->exec($cmd, $output)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceWebGui());
?>
