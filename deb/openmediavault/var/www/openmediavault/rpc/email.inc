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
require_once("openmediavault/util.inc");
require_once("openmediavault/rpc.inc");
require_once("openmediavault/notify.inc");

class EmailRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"set" => array('{
				"type":"object",
				"properties":{
					"enable":{"type":"boolean"},
					"server":{"type":"string"},
					"port":{"type":"integer","minimum":1,"maximum":65535},
					"tls":{"type":"boolean"},
					"sender":{'.$GLOBALS['OMV_JSONSCHEMA_EMAIL'].'},
					"authenable":{"type":"boolean"},
					"username":{"type":"string"},
					"password":{"type":"string"},
					"primaryemail":{'.$GLOBALS['OMV_JSONSCHEMA_EMAIL'].'},
					"secondaryemail":{'.$GLOBALS['OMV_JSONSCHEMA_EMAIL'].'}
				}
			}')
		);
	}

	/**
	 * Get configuration object.
	 */
	function get() {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Check permissions
		$this->validateSession();
		if (!$this->hasRole(OMV_ROLE_ADMINISTRATOR)) {
			throw new OMVException(OMVErrorMsg::E_RPC_SERVICE_INVALID_PERMISSION);
		}
		$this->commitSession();
		// Get configuration object
		$xpath = "//system/email";
		$object = $xmlConfig->get($xpath);
		if (is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify result data
		return array(
			"enable" => boolval($object['enable']),
			"server" => $object['server'],
			"port" => intval($object['port']),
			"tls" => boolval($object['tls']),
			"sender" => $object['sender'],
			"authenable" => boolval($object['authentication']['enable']),
			"username" => $object['authentication']['username'],
			"password" => $object['authentication']['password'],
			"primaryemail" => $object['primaryemail'],
			"secondaryemail" => $object['secondaryemail']
		);
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
			"server" => $data['server'],
			"port" => $data['port'],
			"tls" => array_boolval($data, 'tls'),
			"sender" => $data['sender'],
			"authentication" => array(
				"enable" => array_boolval($data, 'authenable'),
				"username" => $data['username'],
				"password" => $data['password']
			),
			"primaryemail" => $data['primaryemail'],
			"secondaryemail" => $data['secondaryemail']
		);
		// Set configuration object
		$xpath = "//system/email";
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
		  "org.openmediavault.system.email", $object);
	}

	/**
	 * Send a test email.
	 */
	function sendTestEmail($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Update configuration.
		OMVRpc::exec("Email", "set", $data);
		// Send email
		$hostname = php_uname('n');
		$to = $data['primaryemail'];
		if (!empty($data['secondaryemail'])) {
			$to += ",{$data['secondaryemail']}";
		}
		$subject = "Test message from {$hostname}";
		$message = "This is an automatically generated mail message from " .
		  "the system notification service running on {$hostname}.";
		if (!OMVUtil::mail($data['sender'], $to, $subject, $message)) {
			throw new OMVException(OMVErrorMsg::E_EXEC_MISC,
			  "Failed to send email. Please check the logs.");
		}
	}
}
?>
