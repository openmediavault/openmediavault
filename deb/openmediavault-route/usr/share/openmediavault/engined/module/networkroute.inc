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
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");

class OMVModuleNetworkRoute extends \OMV\Engine\Module\ServiceAbstract
  implements \OMV\Notify\IListener {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "networkroute";
	}

	/**
	 * Generate the configuration.
	 * @throw E_EXEC_FAILED
	 */
	public function applyConfig() {
		$cmd = new \OMV\System\Process("omv-mkconf", "route");
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Stop the managed service.
	 */
	public function stopService() {
		$this->execTasks("delete");
	}

	/**
	 * Start the managed service.
	 * @throw E_CONFIG_GET_OBJECT_FAILED
	 * @throw E_EXEC_FAILED
	 */
	public function startService() {
		global $xmlConfig;
		// Get the configuration objects.
		$xpath = "//services/staticroutes/route";
		$objects = $xmlConfig->getList($xpath);
		if (is_null($objects)) {
			throw new OMVException(OMVErrorMsg::E_CONFIG_GET_OBJECT_FAILED,
			  $xpath);
		}
		foreach ($objects as $objectk => $objectv) {
			$cmdArgs = array();
			$cmdArgs[] = "route";
			$cmdArgs[] = sprintf("replace %s via %s", $objectv['network'],
			  $objectv['gateway']);
			$cmd = new \OMV\System\Process("ip", $cmdArgs);
			$cmd->setRedirect2to1(TRUE);
			$cmd->execute();
		}
	}

	/**
	 * Helper function to add a new task when a static route has been deleted.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 * @return None.
	 */
	final public function onDelete($type, $path, $object) {
		$this->addTask("delete", "deleteRoute", $object);
	}

	/**
	 * Delete a static route.
	 */
	public function deleteRoute($object) {
		// Delete the static route.
		$cmdArgs = array();
		$cmdArgs[] = "route";
		$cmdArgs[] = sprintf("delete %s via %s", $objectv['network'],
		  $objectv['gateway']);
		$cmd = new \OMV\System\Process("ip", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->setQuiet(TRUE);
		$cmd->execute();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(\OMV\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.staticroutes.route",
		  array($this, "setDirty"));
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.services.staticroutes.route",
		  array($this, "onDelete"));
    }
}

// Add config enums (elements that must be handled as arrays)-
$xmlConfig->addEnum("route");
