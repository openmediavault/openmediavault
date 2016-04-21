<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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
class OMVRpcServiceNotification extends \OMV\Rpc\ServiceAbstract {
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
		$this->registerMethod("isEnabled");
	}

	/**
	 * Get the notification configuration.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing the notification configuration.
	 */
	function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		$result = [];
		// Get the list of registered modules.
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$modules = $moduleMngr->getModules();
		foreach ($modules as $modulev) {
			// Check if the module implements the required interface.
			if ($modulev instanceof \OMV\Engine\Module\INotification) {
				$config = $modulev->getNotificationConfig();
				foreach ($config as $configk => $configv) {
					// Try to get the notification configuration object.
					$db = \OMV\Config\Database::getInstance();
					$object = $db->getByFilter(
					  "conf.system.notification.notification", [
						  "operator" => "stringEquals",
						  "arg0" => "id",
						  "arg1" => $configv['id']
					  ], 1);
					$object->add("title", "string");
					$object->add("type", "string");
					$object->set("title", $configv['title']);
					$object->set("type", $configv['type']);
					// Append object to result.
					$result[] = $object->getAssoc();
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
	 */
	function set($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.notification.set");
		// Get the list of registered modules.
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$modules = $moduleMngr->getModules();
		foreach ($modules as $modulev) {
			// Check if the module implements the required interface.
			if ($modulev instanceof \OMV\Engine\Module\INotification) {
				$config = $modulev->getNotificationConfig();
				foreach ($config as $configk => $configv) {
					// Set default values.
					$object = new \OMV\Config\ConfigObject(
					  "conf.system.notification.notification");
					$object->setNew(); // Mark the service configuration as new.
					$object->set("id", $configv['id']);
					$object->set("enable", FALSE);
					// Process the RPC arguments.
					foreach ($params as $paramk => $paramv) {
						if ($paramv['id'] === $configv['id']) {
							$object->set("enable", $paramv['enable']);
							break;
						}
					}
					$objects[] = $object;
				}
			}
		}
		// Remove all existing notification configuration objects.
		$filter = [
			"operator" => "stringEquals",
			"arg0" => "id",
			"arg1" => "*"
		];
		$db = \OMV\Config\Database::getInstance();
		if ($db->exists("conf.system.notification.notification",
		  $filter)) {
			$db->deleteByFilter(
			  "conf.system.notification.notification", $filter);
		}
		// Set the notification configuration objects.
		$objectsAssoc = [];
		foreach ($objects as $objectk => $objectv) {
			$db->set($objectv);
			$objectsAssoc[] = $objectv->getAssoc();
		}
		// Notify the configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.system.notification.notification",
		  $objectsAssoc);
		// Return the configuration object.
		return $objectsAssoc;
	}

	/**
	 * Checks whether a notification is enabled.
	 * @param params An array containing the following fields:
	 *   \em id The notification identifier.
	 * @param context The context of the caller.
	 * @return TRUE if the serice is enabled, otherwise FALSE.
	 */
	function isEnabled($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.notification.isenabled");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get(
		  "conf.system.notification.notification",
		  "id", $params['id']);
		// Get the notification status.
		return $object->get("enable");
	}
}

class OMVRpcServiceEmailNotification extends \OMV\Rpc\ServiceAbstract {
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
	 */
	function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.notification.email");
		$object->copy("authentication.enable", "authenable");
		$object->copy("authentication.username", "username");
		$object->copy("authentication.password", "password");
		$object->remove("authentication");
		// Return the values.
		return $object->getAssoc();
	}

	/**
	 * Set configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	function set($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.emailnotification.set");
		// Prepare the configuration data.
		$object = new \OMV\Config\ConfigObject(
		  "conf.system.notification.email");
		$object->setAssoc($params, TRUE, TRUE);
		$object->set("authentication.enable", $params['authenable']);
		$object->set("authentication.username", $params['username']);
		$object->set("authentication.password", $params['password']);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Notify the configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.system.notification.email",
		  $object->getAssoc());
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Send a test email. The email notification service must already be
	 * configured.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw \OMV\Exception
	 * @throw \OMV\Config\ConfigDirtyException
	 */
	function sendTestEmail($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Check whether the module is marked dirty. In this case the
		// current configuration is not applied and sending an email might
		// fail.
		if ($this->isModuleDirty("email"))
			throw new \OMV\Config\ConfigDirtyException();
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.notification.email");
		// Do some generic tests to find out if email notification is
		// already configured.
		if (FALSE === $object->get("enable"))
			throw new \OMV\Exception("Email notification service is disabled.");
		foreach (array("server", "port", "sender", "primaryemail") as
		  $keyk => $keyv) {
			if (TRUE === $object->isEmpty($keyv)) {
				throw new \OMV\Exception(
				  "Email notification service is not configured.");
			}
		}
		// Send a test email.
		$hostname = php_uname('n');
		$to = $object->get("primaryemail");
		if (FALSE === $object->isEmpty("secondaryemail"))
			$to .= sprintf(",%s", $object['secondaryemail']);
		$subject = sprintf("Test message from %s", $hostname);
		$message = sprintf("This is an automatically generated mail message ".
		  "from the system notification service running on %s.", $hostname);
		$mail = new \OMV\Email($object->get("sender"), $to, $subject, $message);
		$mail->assertSend();
	}
}
