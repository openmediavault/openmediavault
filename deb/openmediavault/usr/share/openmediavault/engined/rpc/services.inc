<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
require_once("openmediavault/module.inc");
require_once("openmediavault/rpcservice.inc");

class OMVRpcServiceServices extends OMVRpcServiceAbstract {
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
	 * OMVIModuleServiceStatus interface.
	 * @param params The method parameters.
	 * @param context The context of the caller.
	 * @return An array containing the module statistics.
	 * @throw E_EXEC_FAILED
	 */
	function getStatus($params, $context) {
		// Validate the RPC caller context.
		$this->validateMethodContext($context, array(
			  "role" => OMV_ROLE_ADMINISTRATOR
		  ));
		$status = array();
		// Get the list of registered modules.
		$moduleMgr = &OMVModuleMgr::getInstance();
		$modules = $moduleMgr->getModules();
		foreach($modules as $modulev) {
			// Check if the module implements the OMVIModuleServiceStatus
			// interface.
			if($modulev instanceof OMVIModuleServiceStatus) {
				$status[] = $modulev->getStatus();
			}
		}
		return array(
			"total" => count($status),
			"data" => $status
		);
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceServices());
?>
