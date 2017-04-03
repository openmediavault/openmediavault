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
namespace OMV\System;

require_once("openmediavault/functions.inc");

/**
 * This class implements an interface to a generic Linux block device.
 * @ingroup api
 */
class BlockDevice implements IBlockDevice {
	protected $deviceFile = NULL;
	protected $deviceFileById = NULL;
	protected $size = FALSE;
	protected $blockSize = FALSE;
	protected $sectorSize = FALSE;
	protected $udevData = NULL;

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
	 * See interface definition.
	 */
	public function getCanonicalDeviceFile() {
		return realpath($this->deviceFile);
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
	 * Check whether the device has a /dev/disk/by-id/xxx device path.
	 * @return Returns TRUE if a disk/by-id device path exists,
	 *   otherwise FALSE.
	 */
	public function hasDeviceFileById() {
		return is_devicefile_by_id($this->getDeviceFileById());
	}

	/**
	 * Get the device file by PATH, e.g. <ul>
	 * \li /dev/disk/by-path/pci-0000:00:17.0-ata-3
	 * \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 * \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:1:0-part1
	 * </ul>
	 * @return The device file (/dev/disk/by-path/xxx) if available,
	 *   otherwise NULL will be returned.
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
	 * Check whether the device has a /dev/disk/by-path/xxx device path.
	 * @return Returns TRUE if a disk/by-path device path exists,
	 *   otherwise FALSE.
	 */
	public function hasDeviceFileByPath() {
		return is_devicefile_by_path($this->getDeviceFileByPath());
	}

	/**
	 * Get a predictable device file in the following order:
	 * <ul>
	 * \li /dev/disk/by-id/xxx
	 * \li /dev/disk/by-path/xxx
	 * \li /dev/xxx
	 * </ul>
	 * @return Returns a device file.
	 */
	public function getPredictableDeviceFile() {
		if (TRUE === $this->hasDeviceFileById())
			return $this->getDeviceFileById();
		if (TRUE === $this->hasDeviceFileByPath())
			return $this->getDeviceFileByPath();
		return $this->getCanonicalDeviceFile();
	}

	/**
	 * Get the device file to present in the UI, e.g.:
	 * <ul>
	 * \li /dev/disk/by-id/xxx
	 * \li /dev/disk/by-path/xxx
	 * \li /dev/xxx
	 * </ul>
	 * @return Returns a device file.
	 */
	public function getPreferredDeviceFile() {
		return $this->getCanonicalDeviceFile();
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceFileSymlinks() {
		if (FALSE === $this->hasUdevProperty("DEVLINKS"))
			return [];
		$property = $this->getUdevProperty("DEVLINKS");
		// Parse the property value, e.g.:
		// - disk/by-id/usb-Kingston_DataTraveler_G2_001CC0EC21ADF011C6A20E35-0:0-part1 disk/by-path/pci-0000:02:02.0-usb-0:1:1.0-scsi-0:0:0:0-part1 disk/by-uuid/3849-705A
		// - /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
		// - /dev/disk/by-id/ata-ST1000DM003-1CH132_S2DF60PB /dev/disk/by-id/wwn-0x5000b5016caa4832
		$parts = explode(" ", $property);
		$result = [];
		foreach ($parts as $partk => $partv) {
			// Make sure that the device path is correct.
			if (TRUE === is_devicefile_by($partv))
				$result[] = $partv;
			else
				$result[] = sprintf("/dev/%s", $partv);
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
	 * Get the size of the device in bytes.
	 * @return The size (64bit) of the device in bytes as string.
	 * @throw \OMV\ExecException
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
	 * Get the blocksize of the device in bytes.
	 * @return The blocksize of the device in bytes.
	 * @throw \OMV\ExecException
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
	 * Get the sectorsize of the device in bytes.
	 * @return The sectorsize of the device in bytes.
	 * @throw \OMV\ExecException
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
	 * Queries the udev database for device information stored in the
	 * udev database.
	 * @param force Force the collection of the information, even when the
	 *   information is already cached. Defaults to FALSE.
	 */
	protected function getUdevData($force = FALSE) {
		// Do we need to get the udev information?
		if ((TRUE === $force) || is_null($this->udevData)) {
			$cmdArgs = [];
			$cmdArgs[] = "info";
			$cmdArgs[] = "--query=property";
			$cmdArgs[] = sprintf("--name=%s", escapeshellarg(
			  $this->getDeviceFile()));
			$cmd = new \OMV\System\Process("udevadm", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			// Parse output:
			// UDEV_LOG=3
			// DEVPATH=/devices/pci0000:00/0000:00:10.0/host2/target2:0:1/2:0:1:0/block/sdb
			// MAJOR=8
			// MINOR=16
			// DEVNAME=/dev/sdb
			// DEVTYPE=disk
			// SUBSYSTEM=block
			// ID_SCSI=1
			// ID_VENDOR=VMware_
			// ID_VENDOR_ENC=VMware\x2c\x20
			// ID_MODEL=VMware_Virtual_S
			// ID_MODEL_ENC=VMware\x20Virtual\x20S
			//
			// DEVLINKS=/dev/disk/by-id/ata-IBM-DTTA-341050_WF0WABH1579 /dev/disk/by-id/scsi-SATA_IBM-DTTA-341050_WF0WABH1579 /dev/disk/by-path/pci-0000:00:02.5-scsi-0:0:0:0
			// DEVNAME=/dev/sda
			// DEVPATH=/devices/pci0000:00/0000:00:02.5/host0/target0:0:0/0:0:0:0/block/sda
			// DEVTYPE=disk
			// ID_ATA=1
			// ID_ATA_FEATURE_SET_HPA=1
			// ID_ATA_FEATURE_SET_HPA_ENABLED=1
			// ID_ATA_FEATURE_SET_PM=1
			// ID_ATA_FEATURE_SET_PM_ENABLED=1
			// ID_ATA_FEATURE_SET_SECURITY=1
			// ID_ATA_FEATURE_SET_SECURITY_ENABLED=0
			// ID_ATA_FEATURE_SET_SECURITY_ERASE_UNIT_MIN=22
			// ID_ATA_FEATURE_SET_SMART=1
			// ID_ATA_FEATURE_SET_SMART_ENABLED=0
			// ID_ATA_WRITE_CACHE=1
			// ID_ATA_WRITE_CACHE_ENABLED=1
			// ID_BUS=ata
			// ID_MODEL=IBM-DTTA-351010
			// ID_MODEL_ENC=IBM-DTTA-351010\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20
			// ID_PART_TABLE_TYPE=dos
			// ID_PATH=pci-0000:00:02.5-scsi-0:0:0:0
			// ID_PATH_TAG=pci-0000_00_02_5-scsi-0_0_0_0
			// ID_REVISION=T56OA7A3
			// ID_SCSI_COMPAT=SATA_IBM-DTTA-341050_WF0WABH1579
			// ID_SERIAL=IBM-DTTA-341050_WF0WABH1579
			// ID_SERIAL_SHORT=WF0WFJH1486
			// ID_TYPE=disk
			// MAJOR=8
			// MINOR=0
			// SUBSYSTEM=block
			// UDEV_LOG=3
			// USEC_INITIALIZED=16872806
			$this->udevData = [];
			foreach ($output as $row) {
				$data = explode("=", $row);
				if (empty($data))
					continue;
				$key = trim($data[0]);
				$this->udevData[$key] = $data[1];
			}
		}
	}

	/**
	 * Checks if a udev property exists.
	 * @param id The name of the property, e.g. ID_VENDOR, ID_MODEL or
	 *   ID_SERIAL_SHORT.
	 * @return Returns TRUE if the property exists, otherwise FALSE.
	 */
	public function hasUdevProperty($id) {
		$this->getUdevData();
		return array_key_exists($id, $this->udevData);
	}

	/**
	 * Assert that a udev property exists.
	 * @param id The name of the property, e.g. ID_VENDOR, ID_MODEL or
	 *   ID_SERIAL_SHORT.
	 * @return None.
	 * @throw \OMV\AssertException
	 */
	public function assertUdevProperty($id) {
		if (FALSE === $this->hasUdevProperty($id)) {
			throw new \OMV\AssertException(
			  "The udev property '%s' does not exist.", $id);
		}
	}

	/**
	 * Queries the udev database for device information stored in the udev
	 * database.
	 * @param id The name of the property, e.g. ID_VENDOR, ID_MODEL or
	 *   ID_SERIAL_SHORT.
	 * @return The requested property, otherwise FALSE.
	 */
	public function getUdevProperty($id) {
		$this->getUdevData();
		$this->assertUdevProperty($id);
		return $this->udevData[$id];
	}

	/**
	 * Wait for the specified device. If the device file is not available
	 * within the given time, then an exception is thrown.
	 * @param timeout Timeout in seconds to wait for an available device.
	 * @return None.
	 * @throw \OMV\Exception
	 */
	public function waitForDevice($timeout) {
		for ($i = 0; $i < $timeout; $i++) {
			sleep(1);
			// Check whether the device file exists.
			if (TRUE === $this->exists())
				return;
		}
		// Finally throw an exception if the device file does not exist.
		throw new \OMV\Exception(
		  "Device '%s' is not available after a waiting period of %d seconds.",
		  $this->getDeviceFile(), $timeout);
	}
}
