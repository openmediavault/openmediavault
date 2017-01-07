<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2017 Volker Theile
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
	 * Constructor
	 * @param deviceFile Specifies the device file, e.g. /dev/dm-1,
	 *   /dev/vg0/lv0 or /dev/mapper/vg0-lv0.
	 */
	public function __construct($deviceFile) {
		// Call parent constructor.
		parent::__construct($deviceFile);
		// Any devices of the form /dev/dm-n are for internal use only and
		// should never be used. Because of that the device file returned
		// should look like /dev/mapper/<xyz>. See for more information:
		// https://access.redhat.com/site/documentation/en-US/Red_Hat_Enterprise_Linux/6/html-single/DM_Multipath
		if (1 == preg_match("/^\/dev\/dm-\d+$/", $this->deviceFile)) {
			if (FALSE !== ($name = $this->getDeviceMapperName()))
				$this->deviceFile = sprintf("/dev/mapper/%s", $name);
		}
	}

	/**
	 * Get the name of the device mapper device.
	 * @return The name of the device mapper device.
	 */
	public function getDeviceMapperName() {
		// Make sure the canonical device file is used to extract the name
		// of the device.
		$path = sprintf("/sys/block/%s/dm/name", $this->getDeviceName(TRUE));
		if (!file_exists($path))
			return FALSE;
		return trim(file_get_contents($path));
	}

	/**
	 * Get the UUID of the device mapper device.
	 * @return The UUID of the device mapper device, otherwise FALSE.
	 */
	public function getDeviceMapperUuid() {
		// Make sure the canonical device file is used to extract the name
		// of the device.
		$path = sprintf("/sys/block/%s/dm/uuid", $this->getDeviceName(TRUE));
		if (!file_exists($path))
			return FALSE;
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
		  $this->getDeviceMapperName(), $this->getDeviceFile(),
		  binary_format($this->getSize()));
	}
}
