<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2020 Volker Theile
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
class OMVModuleNetworkRoute extends \OMV\Engine\Module\ServiceAbstract
  implements \OMV\Engine\Notify\IListener {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "networkroute";
	}

	/**
	 * Generate the configuration.
	 */
	public function applyConfig() {
		$cmd = new \OMV\System\Process("omv-mkconf", "route");
		$cmd->setRedirect2to1();
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
	 */
	public function startService() {
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.service.staticroutes.route");
		foreach ($objects as $objectk => $objectv) {
			$cmdArgs = [];
			$cmdArgs[] = "route";
			$cmdArgs[] = sprintf("replace %s", escapeshellarg(
				$objectv->get("network")));
			$cmdArgs[] = sprintf("via %s", escapeshellarg(
				$objectv->get("gateway")));
			$cmd = new \OMV\System\Process("ip", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute();
		}
	}

	/**
	 * Helper function to add a new task when a static route has been deleted.
	 * @param type The event message type.
	 * @param path The event message path.
	 * @param object The configuration object.
	 * @return void
	 */
	final public function onDelete($type, $path, $object) {
		$this->addTask("delete", "deleteRoute", $object);
	}

	/**
	 * Delete a static route.
	 */
	public function deleteRoute($object) {
		// Delete the static route.
		$cmdArgs = [];
		$cmdArgs[] = "route";
		$cmdArgs[] = sprintf("delete %s", escapeshellarg($objectv['network']));
		$cmdArgs[] = sprintf("via %s", escapeshellarg($objectv['gateway']));
		$cmd = new \OMV\System\Process("ip", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->setQuiet(TRUE);
		$cmd->execute();
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY | OMV_NOTIFY_DELETE,
		  "org.openmediavault.conf.service.staticroutes.route",
		  [ $this, "setDirty" ]);
		$dispatcher->addListener(
		  OMV_NOTIFY_DELETE,
		  "org.openmediavault.conf.service.staticroutes.route",
		  [ $this, "onDelete" ]);
    }
}
