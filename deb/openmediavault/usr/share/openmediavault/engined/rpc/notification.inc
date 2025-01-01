<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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
namespace Engined\Rpc;

class Notification extends \OMV\Rpc\ServiceAbstract {
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
		$this->registerMethod("getList");
		$this->registerMethod("setList");
		$this->registerMethod("get");
		$this->registerMethod("set");
		$this->registerMethod("isEnabled");
	}

	/**
	 * Get the notification configuration of all registered modules.
	 */
	private function getNotificationConfig() {
		$result = [];
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$modules = $moduleMngr->getModules();
		foreach ($modules as $modulev) {
			// Check if the module implements the required interface.
			if ($modulev instanceof \OMV\Engine\Module\INotification) {
				$config = $modulev->getNotificationConfig();
				$result = array_merge($result, $config);
			}
		}
		return $result;
	}

	/**
	 * Get the notification configuration objects.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing the notification configuration.
	 */
	function getList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$result = [];
		$notificationConfig = $this->getNotificationConfig();
		foreach ($notificationConfig as $configk => $configv) {
			// Get the notification configuration object.
			$db = \OMV\Config\Database::getInstance();
			$filter = [
				"operator" => "stringEquals",
				"arg0" => "id",
				"arg1" => $configv['id']
			];
			if ($db->exists("conf.system.notification.notification",
					$filter)) {
				// Load the configuration object from database. Raise
				// an error if several objects exist.
				$object = $db->getByFilter(
					"conf.system.notification.notification",
					$filter, 1);
			} else {
				// Create a new database object. The notification is
				// disabled by default.
				$object = new \OMV\Config\ConfigObject(
					"conf.system.notification.notification");
				$object->setFlatAssoc([
					"id" => $configv['id'],
					"enable" => FALSE
				]);
			}
			// Add additional properties.
			$object->add("title", "string");
			$object->add("type", "string");
			$object->setFlatAssoc([
				"title" => $configv['title'],
				"type" => $configv['type']
			]);
			// Append object to result list.
			$result[] = $object->getAssoc();
		}
		return $result;
	}

	/**
	 * Set a bunch of notification configuration objects.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array of the stored configuration objects.
	 */
	function setList($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.notification.setList");
		// Prepare the result array.
		$objectsAssoc = [];
		$notificationConfig = $this->getNotificationConfig();
		foreach ($notificationConfig as $configk => $configv) {
			// Set default values.
			$object = new \OMV\Config\ConfigObject(
				"conf.system.notification.notification");
			$object->set("id", $configv['id']);
			$object->set("enable", FALSE);
			// Process the RPC arguments.
			foreach ($params as $paramk => $paramv) {
				if ($paramv['id'] === $configv['id']) {
					$object->setFlatAssoc([
						"uuid" => $paramv['uuid'],
						"enable" => $paramv['enable']
					]);
					break;
				}
			}
			// Set the configuration object.
			$db = \OMV\Config\Database::getInstance();
			$db->set($object);
			$objectsAssoc[] = $object->getAssoc();
		}
		// Return the configuration objects.
		return $objectsAssoc;
	}

	/**
	 * Get a notification configuration object.
	 * @param params An array containing the following fields:
	 *   \em uuid The UUID of the configuration object.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 */
	public function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.common.objectuuid");
		$notificationConfig = $this->getNotificationConfig();
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.notification.notification",
			$params['uuid']);
		// Add additional properties.
		$object->add("title", "string");
		$object->add("type", "string");
		foreach ($notificationConfig as $configk => $configv) {
			if ($object->get('id') === $configv['id']) {
				$object->setFlatAssoc([
					"title" => $configv['title'],
					"type" => $configv['type']
				]);
				break;
			}
		}
		return $object->getAssoc();
	}

	/**
	 * Set a notification configuration object.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The stored configuration object.
	 */
	public function set($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.notification.set");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.system.notification.notification");
		$object->setAssoc($params);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.notification.isenabled");
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->getByFilter("conf.system.notification.notification", [
			  "operator" => "stringEquals",
			  "arg0" => "id",
			  "arg1" => $params['id']
		  ], 1);
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.notification.email");
		// Relocate various properties.
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
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.emailnotification.set");
		// Prepare the configuration data.
		$object = new \OMV\Config\ConfigObject(
		  "conf.system.notification.email");
		$object->setAssoc($params, TRUE, TRUE);
		$object->setFlatAssoc([
			"authentication.enable" => $params['authenable'],
			"authentication.username" => $params['username'],
			"authentication.password" => $params['password']
		]);
		// Set the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Send a test email. The email notification service must already be
	 * configured.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return void
	 * @throw \OMV\Exception
	 * @throw \OMV\Config\ConfigDirtyException
	 */
	function sendTestEmail($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Check whether the module is marked dirty. In this case the
		// current configuration is not applied and sending an email might
		// fail.
		if ($this->isModuleDirty("postfix"))
			throw new \OMV\Config\ConfigDirtyException();
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.notification.email");
		// Do some generic tests to find out if email notification is
		// already configured.
		if (FALSE === $object->get("enable"))
			throw new \OMV\Exception("Email notification service is disabled.");
		foreach ([ "server", "port", "sender", "primaryemail" ] as
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
			$to .= sprintf(",%s", $object->get("secondaryemail"));
		$subject = sprintf("Test message from %s", $hostname);
		$message = sprintf("This is an automatically generated mail message ".
		  "from the system notification service running on %s.", $hostname);
		$mail = new \OMV\Email($object->get("sender"), $to, $subject, $message);
		$mail->assertSend();
	}
}
