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

class Ssh extends \OMV\Rpc\ServiceAbstract {
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
	 */
	function get($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.ssh");
		return $object->getAssoc();
	}

	/**
	 * Set the configuration settings.
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
		$this->validateMethodParams($params, "rpc.ssh.set");
		// Get the existing configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.ssh");
		$object->setAssoc($params);
		// Set the configuration object.
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}

	/**
	 * Get statistics, e.g. who's online.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return A string containing the requested statistics.
	 */
	public function getStats($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		if ($this->isModuleDirty("ssh")) {
			throw new \OMV\Config\ConfigDirtyException();
		}
		// Get the configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.service.ssh");
		// Is service enabled?
		if (!$object->get("enable")) {
			$stats = gettext("Service disabled");
		} else {
			// Get users currently logged in.
			$stats = sprintf("%s:\n", gettext("Currently logged in users"));
			$cmd = new \OMV\System\Process("who", "-u");
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			$stats .= empty($output) ? gettext("No users connected") :
			  implode("\n", $output);
			unset($output);
			// Get the current connections.
			// Note, if no connections are listed, lsof will exit with 1.
			// See the manual page for this behaviour.
			$cmdArgs = [];
			$cmdArgs[] = "-n";
			$cmdArgs[] = "-P";
			$cmdArgs[] = sprintf("-i :%d", $object->get("port"));
			$cmdArgs[] = "-s TCP:ESTABLISHED";
			$cmd = new \OMV\System\Process("lsof", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->setQuiet(TRUE);
			$cmd->execute($output);
			$stats .= sprintf("\n\n%s:\n", gettext("Connections"));
			$stats .= empty($output) ? gettext("No connections") :
			  implode("\n", $output);
		}
		return $stats;
	}
}
