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
namespace OMV\System;

/**
 * Interface to access udev information.
 * @ingroup api
 */
interface UdevInterface {
	/**
	 * Queries the udev database for device information stored in the
	 * udev database.
	 * Note, blanks are automatically trimmed.
	 * @param boolean force Force the collection of the information,
	 *   although the information is already cached. Defaults to FALSE.
	 * @result void
	 */
	public function queryUdevInfo($force = FALSE);

	/**
	 * Checks if a udev property exists.
	 * @param string id The name of the property, e.g. ID_VENDOR, ID_MODEL
	 *   or ID_SERIAL_SHORT.
	 * @return boolean Returns TRUE if the property exists, otherwise FALSE.
	 */
	public function hasUdevProperty($id);

	/**
	 * Get the specified udev property.
	 * @param string id The name of the property, e.g. ID_VENDOR, ID_MODEL
	 *   or ID_SERIAL_SHORT.
	 * @return mixed|boolean The requested property, otherwise FALSE.
	 */
	public function getUdevProperty($id);
}
