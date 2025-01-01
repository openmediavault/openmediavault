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
namespace OMV\System\Storage;

/**
 * This class provides a simple interface to handle device-mapper devices.
 * See http://en.wikipedia.org/wiki/Device_mapper for more details.
 * @ingroup api
 */
class StorageDeviceDM extends StorageDevice {
	/**
	 * Get the device mapper subsystem, e.g. 'lvm', 'crypt', 'dmraid',
	 * 'mpath', 'part', ...
	 * @return Returns the device mapper subsystem, otherwise NULL.
	 */
	public function getDeviceMapperSubsystem() {
		// The DM_UUID prefix should be set to subsystem owning the device:
		// LVM, CRYPT, DMRAID, MPATH, PART
		// @see https://github.com/karelzak/util-linux/blob/master/misc-utils/lsblk.c#L389
		$path = sprintf("/sys/block/%s/dm/uuid", $this->getDeviceName(TRUE));
		if (!file_exists($path))
			return NULL;
		$content = trim(file_get_contents($path));
		$parts = explode("-", $content);
		if (count($parts) > 1)
			return mb_strtolower(array_shift($parts));
		return NULL;
	}

	/**
	 * Get the name of the device mapper device.
	 * @return The name of the device mapper device, otherwise NULL.
	 */
	public function getDeviceMapperName() {
		// Make sure the canonical device file is used to extract the name
		// of the device.
		$path = sprintf("/sys/block/%s/dm/name", $this->getDeviceName(TRUE));
		if (!file_exists($path))
			return NULL;
		return trim(file_get_contents($path));
	}

	/**
	 * Get the UUID of the device mapper device.
	 * @return The UUID of the device mapper device, otherwise NULL.
	 */
	public function getDeviceMapperUuid() {
		// Make sure the canonical device file is used to extract the name
		// of the device.
		$path = sprintf("/sys/block/%s/dm/uuid", $this->getDeviceName(TRUE));
		if (!file_exists($path))
			return NULL;
		return trim(file_get_contents($path));
	}

	/**
	 * Get the slave devices of the device mapper device.
	 * @return An array of device files, otherwise FALSE.
	 */
	public function getSlaves() {
		// Make sure the canonical device file is used to extract the name
		// of the device.
		$path = sprintf("/sys/block/%s/slaves", $this->getDeviceName(TRUE));
		if (!file_exists($path))
			return FALSE;
		$result = [];
		$dir = new \DirectoryIterator($path);
		foreach ($dir as $item) {
			if ($item->isDot() || !$item->isLink())
				continue;
			$result[] = sprintf("/dev/%s", $item->getFilename());
		}
		// Sort the devices using a "natural order" algorithm.
		if (!sort($result, SORT_NATURAL))
			return FALSE;
		return $result;
	}

	/**
	 * Get the description of the device mapper device.
	 * @return The logical volume description, FALSE on failure.
	 */
	public function getDescription() {
		return sprintf(gettext("Device Mapper %s [%s, %s]"),
		  $this->getDeviceMapperName(), $this->getPreferredDeviceFile(),
		  binary_format($this->getSize()));
	}
}
