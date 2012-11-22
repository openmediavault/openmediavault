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
require_once("openmediavault/module.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/util.inc");

class OMVNetworkRouteModule extends OMVModuleAbstract
  implements OMVINotifyListener {
	/**
	 * Get the module name.
	 */
	public static function getName() {
		return "networkroute";
	}

	/**
	 * Create the script to set the routes during interface setup.
	 */
	public function mkConf() {
		$cmd = "sudo omv-mkconf route 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
	}

	/**
	 * Add a static route.
	 */
	public function add($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Add the static route. Throw an exception if adding the route
		// fails, thus we ensure we do not store an invalid route.
		$cmd = sprintf("sudo ip route add %s via %s 2>&1",
		  $object['network'], $object['gateway']);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
			  $cmd, implode("\n", $output));
		}
		// Update the script that is executed when a interface comes up.
		$this->mkConf();
	}

	/**
	 * Delete a static route.
	 */
	public function delete($object) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Delete the static route.
		$cmd = sprintf("sudo ip route delete %s via %s 2>&1",
		  $object['network'], $object['gateway']);
		OMVUtil::exec($cmd, $output, $result);
// Ignore errors
//		if ($result !== 0) {
//			throw new OMVException(OMVErrorMsg::E_EXEC_FAILED,
//			  $cmd, implode("\n", $output));
//		}
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(OMVNotifyDispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_PRECREATE,
		  "org.openmediavault.services.staticroutes.route",
		  array($this, "add"));
		$dispatcher->addListener(
		  OMV_NOTIFY_PREDELETE,
		  "org.openmediavault.services.staticroutes.route",
		  array($this, "delete"));
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.staticroutes.route",
		  array($this, "mkConf"));
    }
}

// Add config enums (elements that must be handled as arrays)
$xmlConfig->addEnum("route");

// Register module
$moduleMgr = &OMVModuleMgr::getInstance();
$moduleMgr->registerModule(new OMVNetworkRouteModule());
?>
