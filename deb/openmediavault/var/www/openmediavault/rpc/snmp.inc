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
require_once("openmediavault/functions.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class SNMPRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"set" => array('{
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
			}')
		);
	}

	function get() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/snmp";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['trapenable'] = boolval($object['trap']['enable']);
		$object['trapcommunity'] = $object['trap']['community'];
		$object['traphost'] = $object['trap']['host'];
		$object['trapport'] = intval($object['trap']['port']);
		unset($object['trap']);
		return $object;
	}

	function set($data) {
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
		// Update configuration
		$xpath = "//services/snmp";
		$object = array(
			"enable" => array_boolval($data, 'enable'),
			"community" => $data['community'],
			"syslocation" => $data['syslocation'],
			"syscontact" => $data['syscontact'],
			"version" => $data['version'],
			"securitylevel" => $data['securitylevel'],
			"username" => $data['username'],
			"authtype" => $data['authtype'],
			"authpassphrase" => $data['authpassphrase'],
			"privtype" => $data['privtype'],
			"privpassphrase" => $data['privpassphrase'],
			"extraoptions" => $data['extraoptions'],
			"trap" => array(
				"enable" => array_boolval($data, 'trapenable'),
				"community" => $data['trapcommunity'],
				"host" => $data['traphost'],
				"port" => $data['trapport']
			)
		);
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
		  "org.openmediavault.services.snmp", $object);
	}
}
?>
