<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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

class TransmissionBTRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"set" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"peerport":{"type":"integer","minimum":0,"maximum":65535},
					"sharedfolderref":{'.
					  $GLOBALS['OMV_JSONSCHEMA_UUID_NONE'].'},
					"portforwardingenabled":{"type":"boolean"},
					"pexenabled":{"type":"boolean"},
					"dhtenabled":{"type":"boolean"},
					"encryption":{"type":"integer","enum":[0,1,2]},
					"rpcenabled":{"type":"boolean"},
					"rpcport":{"type":"integer","minimum":1024,"maximum":65535},
					"rpcauthenticationrequired":{"type":"boolean"},
					"rpcusername":{"type":"string"},
					"rpcpassword":{"type":"string"},
					"blocklistenabled":{"type":"boolean"},
					"blocklistsyncenabled":{"type":"boolean"},
					"blocklistsyncfrequency":{"type":"string","enum":'.
					  '["hourly","daily","weekly","monthly"]},
					"blocklisturl":{"type":"string"}
				}
			}')
		);
	}

	/**
	 * Get configuration object.
	 */
	function get() {
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//services/transmissionbt";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['peerport'] = intval($object['peerport']);
		$object['portforwardingenabled'] = boolval(
		  $object['portforwardingenabled']);
		$object['pexenabled'] = boolval($object['pexenabled']);
		$object['dhtenabled'] = boolval($object['dhtenabled']);
		$object['encryption'] = intval($object['encryption']);
		$object['rpcenabled'] = boolval($object['rpcenabled']);
		$object['rpcport'] = intval($object['rpcport']);
		$object['rpcauthenticationrequired'] = boolval(
		  $object['rpcauthenticationrequired']);
		$object['blocklistenabled'] = boolval($object['blocklistenabled']);
		$object['blocklistsyncenabled'] = boolval(
		  $object['blocklistsyncenabled']);
		return $object;
	}

	/**
	 * Set configuration object.
	 */
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
		// Prepare configuration data
		$object = array(
			"enable" => array_boolval($data, 'enable'),
			"peerport" => $data['peerport'],
			"sharedfolderref" => $data['sharedfolderref'],
			"portforwardingenabled" => array_boolval($data,
			  'portforwardingenabled'),
			"pexenabled" => array_boolval($data, 'pexenabled'),
			"dhtenabled" => array_boolval($data, 'dhtenabled'),
			"encryption" => $data['encryption'],
			"rpcenabled" => array_boolval($data, 'rpcenabled'),
			"rpcport" => $data['rpcport'],
			"rpcauthenticationrequired" => array_boolval($data,
			  'rpcauthenticationrequired'),
			"rpcusername" => $data['rpcusername'],
			"rpcpassword" => $data['rpcpassword'],
			"blocklistenabled" => array_boolval($data, 'blocklistenabled'),
			"blocklistsyncenabled" => array_boolval($data,
			  'blocklistsyncenabled'),
			"blocklistsyncfrequency" => $data['blocklistsyncfrequency'],
			"blocklisturl" => $data['blocklisturl']
		);
		// Set configuration object
		$xpath = "//services/transmissionbt";
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
		  "org.openmediavault.services.transmissionbt", $object);
	}
}
?>
