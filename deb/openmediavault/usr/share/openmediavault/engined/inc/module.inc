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
require_once("openmediavault/module.inc");

/**
 * Interface to get the state of the service implemented by the module, e.g.
 * is the service enabled and running.
 */
interface OMVIModuleServiceStatus {
	/**
	 * Get the status of the managed service.
	 * @return An array containing the following fields:
	 *   \em name (string) The name of the module.
	 *   \em title (string) A title displayed in the WebGUI.
	 *   \em enabled (boolean) Is the implemented service enabled?
	 *   \em running (boolean) Is the implemented service running?
	 */
	function getStatus();
}

/**
 * Interface to get the notification configuration for this module. This is
 * used to enable/disable notifications send via email for example.
 */
interface OMVIModuleNotification {
	/**
	 * Get the notification configuration.
	 * @return An array of arrays containing the following fields:
	 *   \em id (string) The identifier of the notification.
	 *   \em type (string) The type displayed in the WebGUI.
	 *   \em title (string) A title displayed in the WebGUI.
	 */
	function getNotificationConfig();
}

/**
 * Interface to get the Zeroconf service configuration for this module.
 */
interface OMVIModuleZeroconf {
	/**
	 * Get the Zeroconf service configuration.
	 * @return An array of arrays containing the following fields:
	 *   \em id (string) The identifier of the service.
	 *   \em title (string) A title displayed in the WebGUI.
	 */
	function getZeroconfConfig();
}
?>
