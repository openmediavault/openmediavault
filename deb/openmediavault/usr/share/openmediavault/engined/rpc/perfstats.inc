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

class PerfStats extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "PerfStats";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("get");
		$this->registerMethod("set");
	}

	/**
	 * Get the system monitoring configuration settings.
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
		return $db->getAssoc("conf.system.monitoring.perfstats");
	}

	/**
	 * Set the system monitoring configuration settings.
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
		$this->validateMethodParams($params, "rpc.perfstats.set");
		// Get the existing configuration object.
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.monitoring.perfstats");
		$object->setAssoc($params);
		// Set the configuration object.
		$db->set($object);
		// Return the configuration object.
		return $object->getAssoc();
	}
}
