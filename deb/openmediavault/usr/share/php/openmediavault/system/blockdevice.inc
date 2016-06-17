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
namespace OMV\System;

require_once("openmediavault/functions.inc");

/**
 * This class implements an interface to a generic Linux block device.
 * @ingroup api
 */
class BlockDevice implements IBlockDevice {
	use \OMV\System\UdevDevice;

	protected $deviceFile = NULL;
	protected $deviceFileById = NULL;
	protected $size = FALSE;
	protected $blockSize = FALSE;
	protected $sectorSize = FALSE;

	/**
	 * Constructor
	 * @param deviceFile The device file, e.g. <ul>
	 * \li /dev/sda2
	 * \li /dev/sdb
	 * \li /dev/md1
	 * \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 * \li /dev/disk/by-id/wwn-0x5000cca211cc703c-part1
	 * </ul>
	 */
	public function __construct($deviceFile) {
		if (TRUE === is_devicefile_by_id($deviceFile)) {
			$this->deviceFileById = $deviceFile;
			$this->deviceFile = realpath($deviceFile);
		} else {
			$this->deviceFile = $deviceFile;
		}
	}

	/**
	 * See interface definition.
	 */
	public function exists() {
		return is_block($this->getDeviceFile());
	}

	/**
	 * Assert that the device exists.
	 * @return None.
	 * @throw \OMV\AssertException
	 */
	public function assertExists() {
		if (FALSE === $this->exists()) {
			throw new \OMV\AssertException("Device '%s' does not exist.",
			  $this->getDeviceFile());
		}
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
	 * Get the device path by ID, e.g. <ul>
	 * \li /dev/disk/by-id/wwn-0x5000cca211cc703c
	 * \li /dev/disk/by-id/scsi-SATA_IBM-DHEA-36481_SG0SGF08038
	 * \li /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML-part2#
	 * </ul>
	 * The following order of paths will be retured if available: <ul>
	 * \li ata-xxx
	 * \li wwn-xxx
	 * \li scsi-xxx
	 * \li ...
	 * </ul>
	 * @return The device path (/dev/disk/by-id/xxx) if available,
	 *   otherwise NULL will be returned.
	 */
	public function getDeviceFileById() {
		if (FALSE === is_devicefile_by_id($this->deviceFileById)) {
			// Set the device file by default.
			$this->deviceFileById = $this->getDeviceFile();
			// Get all device file symlinks.
			$symlinks = $this->getDeviceFileSymlinks();
			// Get the 'by-id' device file symlinks.
			$deviceNamesById = [];
			$regex = "/^\/dev\/disk\/by-id\/(.+)$/";
			foreach ($symlinks as $symlinkk => $symlinkv) {
				if (1 == preg_match($regex, $symlinkv, $matches))
					$deviceNamesById[] = $matches[1];
			}
			if (empty($deviceNamesById))
				return NULL;
			// Sort the device names in the following order:
			// 1. ata-xxx
			// 2. wwn-xxx
			// 3. scsi-xxx
			// 4. ...
			//
			// Legend:
			// a=[a]ta-xxx
			// w=[w]wn-xxx
			// s=[s]csi-xxx
			// x=any other device file name
			// fn=call strnatcmp function
			//
			// Compare function matrix:
			// |      | $b=a | $b=w | $b=s | $b=x |
			// |------|------|------|------|------|
			// | $a=a |  fn  |  -1  |  -1  |  -1  |
			// | $a=w |   1  |  fn  |  -1  |  -1  |
			// | $a=s |   1  |   1  |  fn  |  -1  |
			// | $a=x |   1  |   1  |   1  |  fn  |
			//
			// Example:
			// ata-WDC_WD2002FAEX-007BA0_WD-WCAY01065572
			// wwn-0x50014ee25d4cdecd
			// scsi-SATA_WDC_WD2002FAEX-_WD-WCAY01065572
			// xxx...
			// yyy...
			usort($deviceNamesById, create_function('$a, $b',
			  '$map = array("a" => 0, "w" => 1, "s" => 2);'.
			  '$indexA = array_key_exists($a[0], $map) ? $map[$a[0]] : 3;'.
			  '$indexB = array_key_exists($b[0], $map) ? $map[$b[0]] : 3;'.
			  '$cmpMatrix = array('.
			  '    0 => array(0, -1, -1, -1),'.
			  '    1 => array(1,  0, -1, -1),'.
			  '    2 => array(1,  1,  0, -1),'.
			  '    3 => array(1,  1,  1,  0)'.
			  ');'.
			  '$cmpResult = $cmpMatrix[$indexA][$indexB];'.
			  'return (0 !== $cmpResult) ? $cmpResult : strnatcmp($a, $b);'));
			// Finally build the whole path of the device file.
			$this->deviceFileById = sprintf("/dev/disk/by-id/%s",
			  array_shift($deviceNamesById));
		}
		return $this->deviceFileById;
	}

	/**
	 * Check whether the filesystem has a /dev/disk/by-id/xxx device path.
	 * @return Returns TRUE if a disk/by-id device path exists,
	 *   otherwise FALSE.
	 */
	public function hasDeviceFileById() {
		return is_devicefile_by_id($this->getDeviceFileById());
	}

	/**
	* Get all device file symlinks via udev, e.g. <ul>
	* \li /dev/disk/by-id/wwn-0x5000cca211cc703c
	* \li /dev/disk/by-id/scsi-SATA_IBM-DHEA-36481_SG0SGF08038
	* \li /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML
	* \li /dev/disk/by-path/pci-0000:00:02.5-scsi-0:0:0:0
	* </ul>
	* @return Returns an string array of device files.
	*/
	public function getDeviceFileSymlinks() {
		if (FALSE === $this->hasUdevProperty("DEVLINKS"))
			return [];
		$property = $this->getUdevProperty("DEVLINKS");
		// Parse the property value, e.g.:
		// disk/by-id/usb-Kingston_DataTraveler_G2_001CC0EC21ADF011C6A20E35-0:0-part1 disk/by-path/pci-0000:02:02.0-usb-0:1:1.0-scsi-0:0:0:0-part1 disk/by-uuid/3849-705A
		$parts = explode(" ", $output[0]);
		$result = [];
		foreach ($parts as $partk => $partv)
			$result[] = sprintf("/dev/%s", $partv);
		return $result;
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

	/**
	 * See interface definition.
	 */
	public function getDescription() {
		return sprintf("Block device %s [%s]", $this->getDeviceName(),
		  $this->getDeviceNumber());
	}
}
