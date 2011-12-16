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

class NutRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"set" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"upsname":{"type":"string"},
					"comment":{"type":"string"},
					"driverconf":{"type":"string"},
					"shutdownmode":{"type":"string","enum":["fsd","onbatt"]},
					"shutdowntimer":{"type":"integer","minimum":1},
					"remotemonitor":{"type":"boolean"},
					"remoteuser":{"type":"string"},
					"remotepassword":{"type":"string"}
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
		$xpath = "//services/nut";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		$object['enable'] = boolval($object['enable']);
		$object['remotemonitor'] = boolval($object['remotemonitor']);
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
			"upsname" => $data['upsname'],
			"comment" => $data['comment'],
			"driverconf" => $data['driverconf'],
			"shutdownmode" => $data['shutdownmode'],
			"shutdowntimer" => $data['shutdowntimer'],
			"remotemonitor" => array_boolval($data, 'remotemonitor'),
			"remoteuser" => $data['remoteuser'],
			"remotepassword" => $data['remotepassword']
		);
		// Set configuration object
		$xpath = "//services/nut";
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
		  "org.openmediavault.services.nut", $object);
	}

	/**
	 * Get statistics.
	 */
	public function getStats() {
		global $xmlConfig;
		// Is service enabled?
		$xpath = "//services/nut";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if (!array_boolval($object, 'enable')) {
			$stats = gettext("Service disabled");
		} else {
			$cmd = sprintf("sudo upsc %s 2>&1", $object['upsname']);
			OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			$stats = implode("\n", $output);
		}
		return $stats;
	}
}
?>
