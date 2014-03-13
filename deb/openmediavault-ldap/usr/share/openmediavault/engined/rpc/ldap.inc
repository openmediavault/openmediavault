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
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceLDAP extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "LDAP";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
	}

	/**
	 * Get the LDAP configuration settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An JSON object containing the fields \em enable, \em host,
	 *   \em base, \em rootbinddn, \em rootbindpw, \em usersuffix,
	 *   \em groupsuffix and \em extraoptions.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$xpath = "//services/ldap";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result object.
		$object['enable'] = boolval($object['enable']);
		$object['port'] = intval($object['port']);
		return $object;
	}

	/**
	 * Set the LDAP configuration settings.
	 * @param params An array containing the following fields:
	 *   \em enable Boolean value indicating wether the service is enabled.
	 *   \em host The FQDN or IP address of the LDAP server.
	 *   \em base The base DN.
	 *   \em rootbinddn The root bind DN.
	 *   \em rootbindpw The password used to bind to the LDAP server.
	 *   \em usersuffix The group suffix, e.g. ou=Users.
	 *   \em groupsuffix The group suffix, e.g. ou=Groups.
	 *   \em extraoptions The extra libnss.conf options.
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
				  "enable":{"type":"boolean"},
				  "host":{"type":"string"},
				  "port":{"type":"integer","minimum":1,"maximum":65535},
				  "base":{"type":"string"},
				  "rootbinddn":{"type":"string"},
				  "rootbindpw":{"type":"string"},
				  "usersuffix":{"type":"string"},
				  "groupsuffix":{"type":"string"},
				  "extraoptions":{"type":"string"}
			  }
		  }');
		// Prepare the configuration object.
		$object = array(
			"enable" => array_boolval($params, 'enable'),
			"host" => trim($params['host']),
			"port" => $params['port'],
			"base" => trim($params['base']),
			"rootbinddn" => trim($params['rootbinddn']),
			"rootbindpw" => $params['rootbindpw'],
			"usersuffix" => trim($params['usersuffix']),
			"groupsuffix" => trim($params['groupsuffix']),
			"extraoptions" => $params['extraoptions']
		);
		// Set the configuration object.
		$xpath = "//services/ldap";
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ldap", $object);
		// Return the configuration object.
		return $object;
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceLDAP());
?>
