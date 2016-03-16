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
require_once("openmediavault/functions.inc");

class OMVRpcServiceSsh extends \OMV\Rpc\ServiceAbstract {
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
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("//services/ssh", "conf.service.ssh");
		return $object->getAssoc();
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
		$this->validateMethodParams($params, "rpc.ssh.set");
		// Prepare the configuration object.
		$object = new \OMV\Config\ConfigObject("conf.service.ssh");
		$object->setAssoc($params);
		// Update the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$db->replace("//services/ssh", $object);
		// Notify configuration changes.
		$dispatcher = \OMV\Engine\Notify\Dispatcher::getInstance();
		$dispatcher->notify(OMV_NOTIFY_MODIFY,
		  "org.openmediavault.services.ssh",
		  $object->getAssoc());
		// Return the configuration object.
		return $object->getAssoc();
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
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("//services/ssh", "conf.service.ssh");
		// Is service enabled?
		if (!$object->get("enable")) {
			$stats = gettext("Service disabled");
		} else {
			// Get users currently logged in.
			$stats = sprintf("%s:\n", gettext("Currently logged in users"));
			$cmd = new \OMV\System\Process("who", "-u");
			$cmd->setRedirect2to1(TRUE);
			$cmd->execute($output);
			$stats .= empty($output) ? gettext("No users connected") :
			  implode("\n", $output);
			unset($output);
			// Get the current connections.
			$cmdArgs = array();
			$cmdArgs[] = "-n";
			$cmdArgs[] = "-s TCP:ESTABLISHED";
			$cmdArgs[] = sprintf("-i :%d", $object->get("port"));
			$cmd = new \OMV\System\Process("lsof", $cmdArgs);
			$cmd->setRedirect2to1(TRUE);
			$cmd->execute($output);
			$stats .= sprintf("\n\n%s:\n", gettext("Connections"));
			$stats .= empty($output) ? gettext("No connections") :
			  implode("\n", $output);
		}
		return $stats;
	}
}
