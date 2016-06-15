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
namespace OMV\System\Storage;

/**
 * This class provides a simple interface to handle CD-ROM devices.
 * @ingroup api
 */
class StorageDeviceCDROM extends StorageDeviceAbstract {
	/**
	 * Check if the given device is read-only.
	 * @return Returns always TRUE.
	 */
	final public function isReadOnly() {
		return TRUE;
	}

	/**
	 * Checks if any media is available in the device.
	 * @return Returns TRUE if a media is available, otherwise FALSE.
	 */
	public function IsMediaAvailable() {
		if (FALSE === ($property = $this->getUdevProperty("ID_CDROM_MEDIA")))
			return FALSE;
		return ("1" == $property);
	}

	/**
	 * Get the size of the device in bytes.
	 * @return The size (64bit) of the device in bytes as string.
	 */
	public function getSize() {
		// Check if a media is available.
		if (FALSE === $this->IsMediaAvailable())
			return "0";
		// If a media is available, then call the parent class
		// implementation.
		return parent::getSize();
	}
}
