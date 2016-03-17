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
require_once("openmediavault/object.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/functions.inc");

class OMVRpcServiceWebGui extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "WebGui";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getSettings");
		$this->registerMethod("setSettings");
		$this->registerMethod("setPassword");
	}

	/**
	 * Get settings.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return The requested configuration object.
	 * @throw
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 */
	function getSettings($params, $context) {
		global $xmlConfig;
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.webadmin");
		return $object->getAssoc();
	}

	/**
	 * Set settings.
	 * @param params The method parameters.
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
		$this->validateMethodParams($params, "rpc.webgui.setsettings");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.webadmin");
		$object->setAssoc($params);
		// Update the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->set($object);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY, "org.openmediavault.webadmin",
		  $object->getAssoc());
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Set the administrator password.
	 * @param params An array containing the following fields:
	 *   \em password The new password.
	 * @param context The context of the caller.
	 * @return None.
	 * @throw E_EXEC_FAILED
	 */
	function setPassword($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.webgui.setpassword");
		// Modify user password.
		$cmd = sprintf("usermod --password %s admin 2>&1",
		  escapeshellarg(crypt($params['password'])));
		if (0 !== $this->exec($cmd, $output)) {
			throw new \OMV\ErrorMsgException(
			  \OMV\ErrorMsgException::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}
}
