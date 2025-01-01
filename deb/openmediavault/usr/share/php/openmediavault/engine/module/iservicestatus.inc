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
 * Interface to get the state of the service implemented by the module, e.g.
 * is the service enabled and running.
 */
interface IServiceStatus {
	/**
	 * Get the status of the managed service.
	 * @return An array containing the following fields:
	 *   \em name (string) The name of the module.
	 *   \em title (string) A title displayed in the WebGUI.
	 *   \em enabled (boolean) Is the implemented service enabled?
	 *   \em running (boolean) Is the implemented service running?
	 */
	public function getStatus();
}
