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
class StorageDeviceCDROM extends StorageDevice {
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
		// Check if there is a medium inserted.
		if (FALSE === $this->hasUdevProperty("ID_CDROM_MEDIA"))
			return FALSE;
		$property = $this->getUdevProperty("ID_CDROM_MEDIA");
		if ("1" !== $property)
			return FALSE;
		// Note, there are problems with USB network/modem dongles that have
		// CDROM support for driver installation. The CDROM part is identified,
		// and the above code is TRUE which indicates an inserted media, but
		// blkid will fail with an error accessing the device. To prevent this
		// we need to filter such devices. To do so we expect that for a CDROM,
		// DVD or other optical media device the ID_CDROM_MEDIA_SESSION_COUNT
		// identifier must exist.
		return $this->hasUdevProperty("ID_CDROM_MEDIA_SESSION_COUNT");
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
