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

require_once("openmediavault/functions.inc");

class Services extends \OMV\Rpc\ServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	public function getName() {
		return "Services";
	}

	/**
	 * Initialize the RPC service.
	 */
	public function initialize() {
		$this->registerMethod("getStatus");
	}

	/**
	 * Get the status of the modules that implement the
	 * \OMV\Engine\Module\IServiceStatus interface.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing the module statistics.
	 */
	function getStatus($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, [
			"role" => OMV_ROLE_ADMINISTRATOR
		]);
		$status = [];
		// Get the list of registered modules.
		$moduleMngr = \OMV\Engine\Module\Manager::getInstance();
		$modules = $moduleMngr->getModules();
		foreach ($modules as $modulev) {
			// Check if the module implements the required interface.
			if ($modulev instanceof \OMV\Engine\Module\IServiceStatus)
				$status[] = $modulev->getStatus();
		}
		// Remove duplicate entries caused by the 'alias' feature
		// of modules.
		$status = array_unique_key($status, "name");
		return [
			"total" => count($status),
			"data" => $status
		];
	}
}
