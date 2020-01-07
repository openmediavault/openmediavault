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
class OMVModuleMonit extends \OMV\Engine\Module\ServiceAbstract
  implements \OMV\Engine\Notify\IListener, \OMV\Engine\Module\INotification {
	/**
	 * Get the module name.
	 */
	public function getName() {
		return "monit";
	}

	/**
	 * Defines the modules that if present should start before the service
	 * provided by this module.
	 * @return An array of modules.
	 */
	public function shouldStart() { return [ "email" ]; }

	/**
	 * Generate the configuration.
	 */
	public function applyConfig() {
		// Build the configuration.
		$cmd = new \OMV\System\Process("omv-mkconf", "monit");
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Test the new configuration.
		$cmd = new \OMV\System\Process("monit", "-t");
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Start the managed service.
	 */
	public function startService() {
		$systemCtl = new \OMV\System\SystemCtl("monit");
		$systemCtl->reload();
	}

	/**
	 * Get the notification configuration.
	 */
	public function getNotificationConfig() {
		return [
			[
				"id" => "monitprocevents",
				"type" => getText("System"),
				"title" => gettext("Process monitoring")
			],[
				"id" => "monitloadavg",
				"type" => getText("System"),
				"title" => gettext("Load average")
			],[
				"id" => "monitcpuusage",
				"type" => getText("System"),
				"title" => gettext("CPU usage")
			],[
				"id" => "monitmemoryusage",
				"type" => getText("System"),
				"title" => gettext("Memory usage")
			],[
				"id" => "monitfilesystems",
				"type" => getText("Storage"),
				"title" => gettext("Filesystems")
			]
		];
	}

	/**
	 * Bind listeners.
	 */
	function bindListeners(\OMV\Engine\Notify\Dispatcher $dispatcher) {
		$dispatcher->addListener(
		  OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.system.notification.email",
		  [ $this, "setDirty" ]);
		$dispatcher->addListener(
		  OMV_NOTIFY_CREATE | OMV_NOTIFY_MODIFY,
		  "org.openmediavault.conf.system.notification.notification",
		  [ $this, "setDirty" ]);
	}
}
