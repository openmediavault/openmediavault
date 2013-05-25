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
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceNetworkUPSTools extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Nut";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("getStats");
	}

	/**
	 * Get the configuration settings.
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
		$xpath = "//services/nut";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result object.
		$object['enable'] = boolval($object['enable']);
		$object['remotemonitor'] = boolval($object['remotemonitor']);
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
				  "upsname":{"type":"string","format":"regex","pattern":'.
					'"\/^[a-z0-9_-]+$\/i"},
				  "comment":{"type":"string"},
				  "driverconf":{"type":"string"},
				  "shutdownmode":{"type":"string","enum":["fsd","onbatt"]},
				  "shutdowntimer":{"type":"integer","minimum":1},
				  "remotemonitor":{"type":"boolean"},
				  "remoteuser":{"type":"string"},
				  "remotepassword":{"type":"string"}
			  }
		  }');
		// Prepare the configuration object.
		$object = array(
			"enable" => array_boolval($params, 'enable'),
			"upsname" => $params['upsname'],
			"comment" => $params['comment'],
			"driverconf" => $params['driverconf'],
			"shutdownmode" => $params['shutdownmode'],
			"shutdowntimer" => $params['shutdowntimer'],
			"remotemonitor" => array_boolval($params, 'remotemonitor'),
			"remoteuser" => $params['remoteuser'],
			"remotepassword" => $params['remotepassword']
		);
		// Set the configuration object.
		$xpath = "//services/nut";
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.nut", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Get statistics, e.g. who's online.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A string containing the requested statistics.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_EXEC_FAILED
	 */
	public function getStats($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Is service enabled?
		$xpath = "//services/nut";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if(!boolval($object['enable'])) {
			$stats = gettext("Service disabled");
		} else {
			$cmd = sprintf("upsc %s 2>&1", $object['upsname']);
			if(0 !== $this->exec($cmd, $output)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			$stats = implode("\n", $output);
		}
		return $stats;
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceNetworkUPSTools());
?>
