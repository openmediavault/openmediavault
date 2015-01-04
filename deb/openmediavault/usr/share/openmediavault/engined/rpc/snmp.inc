<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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
require_once("openmediavault/functions.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceSnmp extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "SNMP";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("get");
		$this->registerMethod("set");
	}

	/**
	 * Get the power management configuration.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function get($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$xpath = "//services/snmp";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data.
		$object['enable'] = boolvalEx($object['enable']);
		$object['trapenable'] = boolvalEx($object['trap']['enable']);
		$object['trapcommunity'] = $object['trap']['community'];
		$object['traphost'] = $object['trap']['host'];
		$object['trapport'] = intval($object['trap']['port']);
		unset($object['trap']);
		return $object;
	}

	/**
	 * Set the configuration settings.
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
			  "type":"object",
			  "properties":{
				  "enable":{"type":"boolean"},
				  "community":{"type":"string"},
				  "syslocation":{"type":"string"},
				  "syscontact":{"type":"string"},
				  "version":{"type":"string","enum":["2c","3"]},
				  "username":{"type":"string"},
				  "securitylevel":{"type":"string","enum":["noauth",'.
					'"auth","priv"]},
				  "authtype":{"type":"string","enum":["MD5","SHA"]},
				  "authpassphrase":{"type":"string"},
				  "privtype":{"type":"string","enum":["DES","AES"]},
				  "privpassphrase":{"type":"string"},
				  "extraoptions":{"type":"string"},
				  "trapenable":{"type":"boolean"},
				  "trapcommunity":{"type":"string"},
				  "traphost":{"type":"string"},
				  "trapport":{"type":"integer","minimum":1,"maximum":65535}
			  }
		  }');
		// Update the configuration object.
		$xpath = "//services/snmp";
		$object = array(
			"enable" => array_boolval($params, 'enable'),
			"community" => $params['community'],
			"syslocation" => $params['syslocation'],
			"syscontact" => $params['syscontact'],
			"version" => $params['version'],
			"securitylevel" => $params['securitylevel'],
			"username" => $params['username'],
			"authtype" => $params['authtype'],
			"authpassphrase" => $params['authpassphrase'],
			"privtype" => $params['privtype'],
			"privpassphrase" => $params['privpassphrase'],
			"extraoptions" => $params['extraoptions'],
			"trap" => array(
				"enable" => array_boolval($params, 'trapenable'),
				"community" => $params['trapcommunity'],
				"host" => $params['traphost'],
				"port" => $params['trapport']
			)
		);
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.snmp", $object);
		// Return the configuration object.
		return $object;
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceSnmp());
?>
