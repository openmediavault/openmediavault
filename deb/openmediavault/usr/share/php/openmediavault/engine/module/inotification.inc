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
namespace OMV\Engine\Module;

/**
 * Interface to get the notification configuration for this module. This is
 * used to enable/disable notifications send via email for example.
 */
interface INotification {
	/**
	 * Get the notification configuration.
	 * @return An array of arrays containing the following fields:
	 *   \em id (string) The identifier of the notification.
	 *   \em type (string) The type that is displayed in the WebGUI.
     *     This should be one of the following values:
     *     - Administration
     *     - Backup
     *     - Downloaders
     *     - Filesystems
     *     - Gaming
     *     - Multimedia
     *     - Network
     *     - Remote
     *     - Sharing
     *     - Utilities
     *     - Virtualization
     *     - System
	 *   \em title (string) A title displayed in the WebGUI.
	 */
	public function getNotificationConfig();
}
