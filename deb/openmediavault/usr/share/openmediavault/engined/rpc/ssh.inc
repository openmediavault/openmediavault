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
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceSsh extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "SSH";
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
		$xpath = "//services/ssh";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data.
		$object['enable'] = boolvalEx($object['enable']);
		$object['port'] = intval($object['port']);
		$object['passwordauthentication'] = boolvalEx(
		  $object['passwordauthentication']);
		$object['pubkeyauthentication'] = boolvalEx(
		  $object['pubkeyauthentication']);
		$object['permitrootlogin'] = boolvalEx($object['permitrootlogin']);
		$object['tcpforwarding'] = boolvalEx($object['tcpforwarding']);
		$object['compression'] = boolvalEx($object['compression']);
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
				  "port":{"type":"integer","minimum":1,"maximum":65535},
				  "passwordauthentication":{"type":"boolean"},
				  "pubkeyauthentication":{"type":"boolean"},
				  "permitrootlogin":{"type":"boolean"},
				  "tcpforwarding":{"type":"boolean"},
				  "compression":{"type":"boolean"},
				  "extraoptions":{"type":"string"}
			  }
		  }');
		// Update the configuration.
		$xpath = "//services/ssh";
		$object = array(
			"enable" => array_boolval($params, 'enable'),
			"port" => $params['port'],
			"extraoptions" => $params['extraoptions'],
			"passwordauthentication" => array_boolval($params,
			  'passwordauthentication'),
			"pubkeyauthentication" => array_boolval($params,
			  'pubkeyauthentication'),
			"permitrootlogin" => array_boolval($params, 'permitrootlogin'),
			"tcpforwarding" => array_boolval($params, 'tcpforwarding'),
			"compression" => array_boolval($params, 'compression')
		);
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ssh", $object);
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
		$xpath = "//services/ssh";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		if (!boolvalEx($object['enable'])) {
			$stats = gettext("Service disabled");
		} else {
			// Get users currently logged in.
			$stats = sprintf("%s:\n", gettext("Currently logged in users"));
			$cmd = "export LANG=C; who -u 2>&1";
			if(0 !== $this->exec($cmd, $output)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			$stats .= empty($output) ? gettext("No users connected") :
			  implode("\n", $output);
			// Get connections.
			$cmd = sprintf("export LANG=C; lsof -n -s TCP:ESTABLISHED ".
			  "-i :%d 2>&1 || true", $object['port']);
			if(0 !== $this->exec($cmd, $output)) {
				throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
				  $cmd, implode("\n", $output));
			}
			$stats .= sprintf("\n\n%s:\n", gettext("Connections"));
			$stats .= empty($output) ? gettext("No connections") :
			  implode("\n", $output);
		}
		return $stats;
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceSsh());
?>
