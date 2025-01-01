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

require_once("openmediavault/functions.inc");

/**
 * This class implements an interface to a generic Linux block device.
 * @ingroup api
 */
class BlockDevice implements BlockDeviceInterface, UdevInterface {
	use UdevTrait;

	protected $deviceFile = NULL;
	protected $deviceFileById = NULL;
	protected $size = FALSE;
	protected $blockSize = FALSE;
	protected $sectorSize = FALSE;

	/**
	 * Constructor
	 * @param deviceFile The device file, e.g.
	 *   <ul>
	 *   \li /dev/sda2
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-id/wwn-0x5000cca211cc703c-part1
	 *   \li /dev/disk/by-id/md-name-vmpc01:data
	 *   \li /dev/disk/by-id/md-uuid-75de9de9:6beca92e:8442575c:73eabbc9
	 *   \li /dev/disk/by-id/lvm-pv-uuid-VDH5Om-Rkjc-cQid-PeJI-Sfqm-66DI-w0dpCO
	 *   </ul>
	 */
	public function __construct($deviceFile) {
		if (TRUE === is_devicefile_by_id($deviceFile)) {
			$this->deviceFileById = $deviceFile;
			$this->deviceFile = realpath($deviceFile);
		} else if (TRUE === is_devicefile($deviceFile)) {
			$this->deviceFile = $deviceFile;
		}
	}

	/**
	 * See interface definition.
	 */
	public function exists() {
		return is_block_device($this->getDeviceFile());
	}

	/**
	 * Assert that the device exists.
	 * @return void
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
	 * See interface definition.
	 */
	public function getCanonicalDeviceFile() {
		$deviceFile = realpath($this->deviceFile);
		if (FALSE === $deviceFile) {
			throw new \OMV\Exception("Device '%s' does not exist.",
			  $this->deviceFile);
		}
		return $deviceFile;
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceFileById() {
		if (FALSE === is_devicefile_by_id($this->deviceFileById)) {
			// Get all device file symlinks.
			$symlinks = $this->getDeviceFileSymlinks();
			// Get the 'by-id' device file symlinks.
			$deviceNamesById = [];
			$regex = "/^\/dev\/disk\/by-id\/(.+)$/";
			foreach ($symlinks as $symlinkk => $symlinkv) {
				if (1 == preg_match($regex, $symlinkv, $matches))
					$deviceNamesById[] = $matches[1];
			}
			if (TRUE === empty($deviceNamesById))
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
			// lvm-pv-uuid-VDH5Om-Rkjc-cQid-PeJI-Sfqm-66DI-w0dpCO
			// xxx...
			// yyy...
			//
			// Use a decorate-sort-undecorate method to sort the list of
			// device names.
			array_walk($deviceNamesById, function(&$v, $k) {
				$map = array("a" => 0, "w" => 1, "s" => 2);
				$v = array($v, array_key_exists($v[0], $map) ?
					$map[$v[0]] : 3);
			});
			usort($deviceNamesById, function($a, $b) {
				return ($a[1] < $b[1]) ? -1 : (($a[1] > $b[1]) ? 1 :
					strnatcmp($a[0], $b[0]));
			});
			array_walk($deviceNamesById, function(&$v, $k) {
				$v = $v[0];
			});
			// Finally build the whole path of the device file.
			$this->deviceFileById = sprintf("/dev/disk/by-id/%s",
			  array_shift($deviceNamesById));
		}
		return $this->deviceFileById;
	}

	/**
	 * See interface definition.
	 */
	public function hasDeviceFileById() {
		return is_devicefile_by_id($this->getDeviceFileById());
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceFileByPath() {
		$result = NULL;
		// Get all device file symlinks.
		$symlinks = $this->getDeviceFileSymlinks();
		foreach ($symlinks as $symlinkk => $symlinkv) {
			if (TRUE === is_devicefile_by_path($symlinkv)) {
				$result = $symlinkv;
				break;
			}
		}
		return $result;
	}

	/**
	 * See interface definition.
	 */
	public function hasDeviceFileByPath() {
		return is_devicefile_by_path($this->getDeviceFileByPath());
	}

	/**
	 * See interface definition.
	 */
	public function getPredictableDeviceFile() {
		if (TRUE === $this->hasDeviceFileById())
			return $this->getDeviceFileById();
		if (TRUE === $this->hasDeviceFileByPath())
			return $this->getDeviceFileByPath();
		return $this->getCanonicalDeviceFile();
	}

	/**
	 * See interface definition.
	 */
	public function getPreferredDeviceFile() {
		return $this->getCanonicalDeviceFile();
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceFileSymlinks() {
		if (FALSE === $this->hasUdevProperty("DEVLINKS")) {
			return [];
		}
		$property = $this->getUdevProperty("DEVLINKS");
		// Parse the property value, e.g.:
		// - disk/by-id/usb-Kingston_DataTraveler_G2_001CC0EC21ADF011C6A20E35-0:0-part1 disk/by-path/pci-0000:02:02.0-usb-0:1:1.0-scsi-0:0:0:0-part1 disk/by-uuid/3849-705A
		// - /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
		// - /dev/disk/by-id/ata-ST1000DM003-1CH132_S2DF60PB /dev/disk/by-id/wwn-0x5000b5016caa4832
		// - /dev/mapper/omv--lvm--vg-root /dev/omv-lvm-vg/root /dev/disk/by-id/dm-name-omv--lvm--vg-root /dev/disk/by-id/dm-uuid-LVM-qrB1zl0Sshr0gzosPHD4NeJC7D2aLNF50m40r7lb7304SjsnCw6V0hOOeMmByPrq /dev/disk/by-uuid/4a1aa5db-980b-43ac-a62a-a2b9eadcb34f
		$parts = array_map("trim", explode(" ", $property));
		sort($parts, SORT_NATURAL);
		$result = [];
		foreach ($parts as $partk => $partv) {
			// Make sure that the device path is correct.
			if (TRUE === is_devicefile($partv)) {
				$result[] = $partv;
			} else {
				$result[] = sprintf("/dev/%s", $partv);
			}
		}
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
		if (FALSE === $this->size) {
			$cmdArgs = [];
			$cmdArgs[] = "--getsize64";
			$cmdArgs[] = escapeshellarg($this->getDeviceFile());
			$cmd = new \OMV\System\Process("blockdev", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			$this->size = $output[0]; // Do not convert to long.
		}
		return $this->size;
	}

	/**
	 * See interface definition.
	 */
	public function getBlockSize() {
		if (FALSE === $this->blockSize) {
			$cmdArgs = [];
			$cmdArgs[] = "--getbsz";
			$cmdArgs[] = escapeshellarg($this->getDeviceFile());
			$cmd = new \OMV\System\Process("blockdev", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			$this->blockSize = intval($output[0]);
		}
		return $this->blockSize;
	}

	/**
	 * See interface definition.
	 */
	public function getSectorSize() {
		if (FALSE === $this->sectorSize) {
			$cmdArgs = [];
			$cmdArgs[] = "--getss";
			$cmdArgs[] = escapeshellarg($this->getDeviceFile());
			$cmd = new \OMV\System\Process("blockdev", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			$this->sectorSize = intval($output[0]);
		}
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

	/**
	 * See interface definition.
	 */
	public function waitForDevice($timeout) {
		waitUntil($timeout, [$this, "exists"], sprintf(
			"Device '%s' is not available after a waiting period of %d seconds.",
		  	$this->getDeviceFile(), $timeout));
	}
}
