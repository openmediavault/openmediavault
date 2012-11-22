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

class LDAPRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"setSettings" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"host":{"type":"string"},
					"base":{"type":"string"},
					"rootbinddn":{"type":"string"},
					"rootbindpw":{"type":"string"},
					"usersuffix":{"type":"string"},
					"groupsuffix":{"type":"string"},
					"extraoptions":{"type":"string"}
				}
			}')
		);
	}

	/**
	 * Get the LDAP configuration settings.
	 * @return An JSON object containing the fields \em enable, \em host,
	 *   \em base, \em rootbinddn, \em rootbindpw, \em usersuffix,
	 *   \em groupsuffix and \em extraoptions.
	 */
	function getSettings() {
		global $xmlConfig;
		// Check permissions.
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object.
		$xpath = "//services/ldap";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data.
		$object['enable'] = boolval($object['enable']);
		return $object;
	}

	/**
	 * Set the LDAP configuration settings.
	 * @param data An array containing the following fields:
	 *   \em enable Boolean value indicating wether the service is enabled.
	 *   \em host The FQDN or IP address of the LDAP server.
	 *   \em base The base DN.
	 *   \em rootbinddn The root bind DN.
	 *   \em rootbindpw The password used to bind to the LDAP server.
	 *   \em usersuffix The group suffix, e.g. ou=Users.
	 *   \em groupsuffix The group suffix, e.g. ou=Groups.
	 *   \em extraoptions The extra libnss.conf options.
	 * @return None
	 */
	function setSettings($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions.
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Validate method parameters.
		$this->validateParams(__METHOD__, func_get_args());
		// Get current configuration.
		$xpath = "//services/ldap";
		if (is_null($oldObject = $xmlConfig->get($xpath))) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Update configuration.
		$object = array(
			"enable" => array_boolval($data, 'enable'),
			"host" => trim($data['host']),
			"base" => trim($data['base']),
			"rootbinddn" => trim($data['rootbinddn']),
			"rootbindpw" => $data['rootbindpw'],
			"usersuffix" => trim($data['usersuffix']),
			"groupsuffix" => trim($data['groupsuffix']),
			"extraoptions" => $data['extraoptions']
		);
		if (FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Save configuration.
		if (FALSE === $xmlConfig->save()) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SAVE_FAILED,
			  $xmlConfig->getError());
		}
		// Notify configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ldap",
		  $object, $oldObject);
	}
}
?>
