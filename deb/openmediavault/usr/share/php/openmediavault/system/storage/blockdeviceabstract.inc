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
 * Abstract class implementing generic block devices.
 * @ingroup api
 */
abstract class BlockDeviceAbstract implements IBlockDevice {
	protected $deviceFile = FALSE;
	protected $size = FALSE;
	protected $blockSize = FALSE;
	protected $sectorSize = FALSE;

	/**
	 * Constructor
	 * @param deviceFile The device file.
	 */
	public function __construct($deviceFile) {
		$this->deviceFile = $deviceFile;
	}

	/**
	 * See interface definition.
	 */
	public function exists() {
		return is_block($this->getDeviceFile());
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceFile() {
		return $this->deviceFile;
	}

	/**
	 * Get the canonical device file, e.g. /dev/mapper/vg0-lv0 -> /dev/dm-0
	 */
	public function getCanonicalDeviceFile() {
		return realpath($this->deviceFile);
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceName($canonical = FALSE) {
		// Get the device file and extract the name, e.g. /dev/sda => sda.
		$deviceName = str_replace("/dev/", "", !$canonical ?
		  $this->getDeviceFile() : $this->getCanonicalDeviceFile());
		return $deviceName;
	}

	/**
	 * See interface definition.
	 */
	public function getSize() {
		return $this->size;
	}

	/**
	 * See interface definition.
	 */
	public function getBlockSize() {
		return $this->blockSize;
	}

	/**
	 * See interface definition.
	 */
	public function getSectorSize() {
		return $this->sectorSize;
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceNumber() {
		// Get the canonical device name, e.g.
		// /dev/root => /dev/sda1
		$deviceName = $this->getDeviceName(TRUE);
		// Get the device number via sysfs.
		$filename = sprintf("/sys/class/block/%s/dev", $deviceName);
		if (!file_exists($filename))
			return FALSE;
		return trim(file_get_contents($filename));
	}

	/**
	 * See interface definition.
	 */
	public function getMajor() {
		if (FALSE === ($devNum = $this->getDeviceNumber()))
			return FALSE;
		$devNumParts = explode(":", $devNum);
		return intval($devNumParts[0]);
	}

	/**
	 * See interface definition.
	 */
	public function getMinor() {
		if (FALSE === ($devNum = $this->getDeviceNumber()))
			return FALSE;
		$devNumParts = explode(":", $devNum);
		return intval($devNumParts[1]);
	}
}
