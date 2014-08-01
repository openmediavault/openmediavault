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
require_once("openmediavault/util.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/notify.inc");

class OMVRpcServiceNotification extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Notification";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("get");
		$this->registerMethod("set");
	}

	/**
	 * Get the notification configuration.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing the notification configuration.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function get($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		$result = array();
		// Get the configuration object.
		$xpath = "//system/notification/notifications";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Get the list of registered modules.
		$moduleMgr = &OMVModuleMgr::getInstance();
		$modules = $moduleMgr->getModules();
		foreach($modules as $modulev) {
			// Check if the module implements the OMVIModuleNotification
			// interface.
			if($modulev instanceof OMVIModuleNotification) {
				$config = $modulev->getNotificationConfig();
				foreach($config as $configk => $configv) {
					$enable = FALSE; // Default value.
					if(array_key_exists($configv['id'], $object)) {
						$enable = boolvalEx($object[$configv['id']]);
					}
					$result[] = array(
						  "id" => $configv['id'],
						  "type" => $configv['type'],
						  "title" => $configv['title'],
						  "enable" => $enable
					  );
				}
			}
		}
		return $result;
	}

	/**
	 * Set the notification configuration.
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
			  "type":"array",
			  "items":{
				  "type":"object",
				  "properties":{
					  "id":{"type":"string"},
					  "enable":{"type":"boolean"}
				  }
			  }
		  }');
		// Prepare the configuration object.
		$object = array();
		// Get the list of registered modules.
		$moduleMgr = &OMVModuleMgr::getInstance();
		$modules = $moduleMgr->getModules();
		foreach($modules as $modulev) {
			// Check if the module implements the OMVIModuleNotification
			// interface.
			if($modulev instanceof OMVIModuleNotification) {
				$config = $modulev->getNotificationConfig();
				foreach($config as $configk => $configv) {
					$enable = FALSE; // Default value.
					foreach($params as $paramk => $paramv) {
						if($paramv['id'] === $configv['id']) {
							$enable = array_boolval($paramv, 'enable');
							break;
						}
					}
					$object[$configv['id']] = $enable;
				}
			}
		}
		// Set the configuration object.
		$xpath = "//system/notification/notifications";
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify the configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.notification.notifications", $object);
		// Return the configuration object.
		return $object;
	}
}

class OMVRpcServiceEmailNotification extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "EmailNotification";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("sendTestEmail");
	}

	/**
	 * Get configuration object.
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
		$xpath = "//system/email";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Modify the result object.
		return array(
			"enable" => boolvalEx($object['enable']),
			"server" => $object['server'],
			"port" => intval($object['port']),
			"tls" => boolvalEx($object['tls']),
			"sender" => $object['sender'],
			"authenable" => boolvalEx($object['authentication']['enable']),
			"username" => $object['authentication']['username'],
			"password" => $object['authentication']['password'],
			"primaryemail" => $object['primaryemail'],
			"secondaryemail" => $object['secondaryemail']
		);
	}

	/**
	 * Set configuration object.
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
		  }');
		// Prepare the configuration data.
		$object = array(
			"enable" => array_boolval($params, 'enable'),
			"server" => $params['server'],
			"port" => $params['port'],
			"tls" => array_boolval($params, 'tls'),
			"sender" => $params['sender'],
			"authentication" => array(
				"enable" => array_boolval($params, 'authenable'),
				"username" => $params['username'],
				"password" => $params['password']
			),
			"primaryemail" => $params['primaryemail'],
			"secondaryemail" => $params['secondaryemail']
		);
		// Set the configuration object.
		$xpath = "//system/email";
		if(FALSE === $xmlConfig->replace($xpath, $object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_SET_OBJECT_FAILED);
		}
		// Notify the configuration changes.
		$dispatcher = &OMVNotifyDispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.system.email", $object);
		// Return the configuration object.
		return $object;
	}

	/**
	 * Send a test email. The email notification service must already be
	 * configured.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_MISC_FAILURE
	 * @throw E_CONFIG_DIRTY
	 */
	function sendTestEmail($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Check whether the module is marked dirty. In this case the
		// current configuration is not applied and sending an email might
		// fail.
		if($this->isModuleDirty("email")) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_DIRTY);
		}
		// Get the configuration object.
		$xpath = "//system/email";
		$object = $xmlConfig->get($xpath);
		if(is_null($object)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		// Do some generic tests to find out if email notification is
		// already configured.
		if (!boolvalEx($object['enable'])) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  "Email notification service is disabled.");
		}
		foreach(array("server", "port", "sender", "primaryemail") as
		  $keyk => $keyv) {
			if(empty($object[$keyv])) {
				throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
				  "Email notification service is not configured.");
			}
		}
		// Send a test email.
		$hostname = php_uname('n');
		$to = $object['primaryemail'];
		if(!empty($object['secondaryemail'])) {
			$to .= sprintf(",%s", $object['secondaryemail']);
		}
		$subject = sprintf("Test message from %s", $hostname);
		$message = sprintf("This is an automatically generated mail message ".
		  "from the system notification service running on %s.", $hostname);
		if(!OMVUtil::mail($object['sender'], $to, $subject, $message)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  "Failed to send email. Please check the logs.");
		}
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceNotification());
$rpcServiceMgr->registerService(new OMVRpcServiceEmailNotification());
?>
