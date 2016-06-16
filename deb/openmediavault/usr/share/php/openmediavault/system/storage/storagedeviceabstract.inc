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
 * Abstract class implementing generic storage devices.
 * @ingroup api
 */
abstract class StorageDeviceAbstract extends BlockDeviceAbstract {
	protected $deviceFileById = FALSE;
	protected $model = FALSE;
	protected $vendor = FALSE;
	protected $udevInfo = FALSE;

	/**
	 * Constructor
	 * @param deviceFile The device file, e.g.
	 * \li /dev/sdb
	 * \li /dev/md1
	 * \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 * \li /dev/disk/by-id/wwn-0x5000cca211cc703c
	 */
	public function __construct($deviceFile) {
		if (preg_match("/^\/dev\/disk\/by-id\/.+$/", $deviceFile)) {
			$this->deviceFileById = $deviceFile;
		} else {
			$this->deviceFile = $deviceFile;
		}
	}

	/**
	 * See interface definition.
	 */
	public function exists() {
		return is_block(!$this->deviceFile ? $this->deviceFileById :
		  $this->deviceFile);
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceFile() {
		if (!$this->deviceFile)
			$this->deviceFile = realpath($this->deviceFileById);
		return $this->deviceFile;
	}

	/**
	 * Get the device path by ID, e.g.
	 * \li /dev/disk/by-id/wwn-0x5000cca211cc703c
	 * \li /dev/disk/by-id/scsi-SATA_IBM-DHEA-36481_SG0SGF08038
	 * \li /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML
	 * The following order of paths will be retured if available:
	 * \li ata-xxx
	 * \li wwn-xxx
	 * \li scsi-xxx
	 * \li ...
	 * @return The device path (/dev/disk/by-id/xxx) if available,
	 *   otherwise /dev/xxx will be returned. In case of an error FALSE
	 *   will be returned.
	 */
	public function getDeviceFileById() {
		if (!$this->deviceFileById) {
			$devLinks = $this->getDeviceFileSymlinks();
			// Set the device file by default.
			$this->deviceFileById = $this->getDeviceFile();
			// Get the 'by-id' device file symlinks.
			$deviceNamesById = [];
			$regex = "/^\/dev\/disk\/by-id\/(.+)$/";
			foreach ($devLinks as $devLinkk => $devLinkv) {
				if (1 == preg_match($regex, $devLinkv, $matches))
					$deviceNamesById[] = $matches[1];
			}
			if (empty($deviceNamesById))
				return FALSE;
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
		return (FALSE !== $this->getDeviceFileById());
	}

	/**
	* Get all device file symlinks via UDEV, e.g.
	* \li /dev/disk/by-id/wwn-0x5000cca211cc703c
	* \li /dev/disk/by-id/scsi-SATA_IBM-DHEA-36481_SG0SGF08038
	* \li /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML
	* \li /dev/disk/by-path/pci-0000:00:02.5-scsi-0:0:0:0
	* @return The device file symlinks as an array of strings.
	* @throw \OMV\ExecException
	*/
	public function getDeviceFileSymlinks() {
		$cmdArgs = [];
		$cmdArgs[] = "info";
		$cmdArgs[] = "--query=symlink";
		$cmdArgs[] = sprintf("--name=%s", escapeshellarg(
		  $this->getDeviceFile()));
		$cmd = new \OMV\System\Process("udevadm", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		// Parse the command output.
		// Example:
		// disk/by-id/usb-Kingston_DataTraveler_G2_001CC0EC21ADF011C6A20E35-0:0-part1 disk/by-path/pci-0000:02:02.0-usb-0:1:1.0-scsi-0:0:0:0-part1 disk/by-uuid/3849-705A
		$parts = explode(" ", $output[0]);
		$result = [];
		foreach ($parts as $partk => $partv)
			$result[] = sprintf("/dev/%s", $partv);
		return $result;
	}

	/**
	 * Get the size of the device in bytes.
	 * @return The size (64bit) of the device in bytes as string.
	 * @throw \OMV\ExecException
	 */
	public function getSize() {
		if ($this->size === FALSE) {
			$cmdArgs = [];
			$cmdArgs[] = "--getsize64";
			$cmdArgs[] = escapeshellarg($this->getDeviceFile());
			$cmd = new \OMV\System\Process("blockdev", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			$this->size = $output[0];
		}
		return $this->size;
	}

	/**
	 * Get the blocksize of the device in bytes.
	 * @return The blocksize of the device in bytes.
	 * @throw \OMV\ExecException
	 */
	public function getBlockSize() {
		if ($this->blockSize === FALSE) {
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
		if ($this->sectorSize === FALSE) {
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
	 * Get the device model.
	 * @return The device model, otherwise an empty string.
	 */
	public function getModel() {
		if ($this->model === FALSE) {
			$filename = sprintf("/sys/block/%s/device/model",
			  $this->getDeviceName());
			if (file_exists($filename)) {
				$this->model = trim(file_get_contents($filename));
			} else {
				$this->model = "";
			}
		}
		return $this->model;
	}

	/**
	 * Get the device vendor.
	 * @return The device vendor, otherwise an empty string.
	 */
	public function getVendor() {
		if ($this->vendor === FALSE) {
			$filename = sprintf("/sys/block/%s/device/vendor",
			  $this->getDeviceName());
			if (file_exists($filename)) {
				$this->vendor = trim(file_get_contents($filename));
			} else {
				$this->vendor = "";
			}
		}
		return $this->vendor;
	}

	/**
	 * Get the device serial number.
	 * @return The device serial number, otherwise an empty string.
	 */
	public function getSerialNumber() {
		if (FALSE === ($property = $this->getUdevProperty("ID_SERIAL_SHORT")))
			return "";
		return str_replace("_", " ", $property);
	}

	/**
	 * See interface definition.
	 */
	public function getDescription() {
		$model = $this->getModel();
		return sprintf("%s [%s, %s]", !empty($model) ? $model : gettext("n/a"),
		  $this->getDeviceFile(), binary_format($this->getSize()));
	}

	/**
	 * Check if the device is of rotational type or non-rotational type.
	 * See https://www.kernel.org/doc/Documentation/block/queue-sysfs.txt
	 * @return TRUE if device is of rotational type, otherwise FALSE.
	 */
	public function isRotational() {
		// Use udev property.
		if (FALSE !== ($property = $this->getUdevProperty(
		  "ID_SSD"))) {
			// If ID_SSD is not 1 then it is rotational.
			return (0 == strcasecmp("1", $property)) ? FALSE : TRUE;
		}
		if (FALSE !== ($property = $this->getUdevProperty(
		  "ID_ATA_ROTATION_RATE_RPM"))) {
			// If ID_ATA_ROTATION_RATE_RPM is non-zero then it is rotational.
			return (0 == strcasecmp("0", $property)) ? FALSE : TRUE;
		}
		if (FALSE !== ($property = $this->getUdevProperty(
		  "ID_ATA_FEATURE_SET_AAM"))) {
			// If ID_ATA_FEATURE_SET_AAM is non-zero then it is rotational.
			return (0 == strcasecmp("0", $property)) ? FALSE : TRUE;
		}
		// Use kernel attribute.
		$filename = sprintf("/sys/block/%s/queue/rotational",
		  $this->getDeviceName());
		if (file_exists($filename)) {
			// If file content is non-zero then it is rotational.
			return ("0" == trim(file_get_contents($filename))) ? FALSE : TRUE;
		}
		// Use heuristic.
		$model = $this->getModel();
		if (is_string($model) && (FALSE !== strstr($model, "SSD")))
			return FALSE;
		return TRUE;
	}

	/**
	* Check if the device is removable.
	* @return TRUE if device is removable, otherwise FALSE.
	*/
	public function isRemovable() {
		$filename = sprintf("/sys/block/%s/removable",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return (trim(file_get_contents($filename)) == "1") ? TRUE : FALSE;
	}

	/**
	 * Check if the given device is a hardware/software RAID device.
	 * @return TRUE if the device is a hardware/software RAID, otherwise FALSE.
	 */
	public function isRaid() {
		return FALSE;
	}

	/**
	 * Check if the given device is an USB device.
	 * @return TRUE if the device is a connected via USB, otherwise FALSE.
	 */
	public function isUsb() {
		// Identify USB devices via 'ID_BUS=usb'.
		if (FALSE !== ($property = $this->getUdevProperty("ID_BUS"))) {
			if (0 == strcasecmp("usb", $property))
				return TRUE;
		}
		// Identify USB devices via 'ID_USB_DRIVER=usb-storage'.
		if (FALSE !== ($property = $this->getUdevProperty("ID_USB_DRIVER"))) {
			if (0 == strcasecmp("usb-storage", $property))
				return TRUE;
		}
		// Identify USB devices via 'ID_DRIVE_THUMB=1'.
		if (FALSE !== ($property = $this->getUdevProperty("ID_DRIVE_THUMB"))) {
			if (0 == strcasecmp("1", $property))
				return TRUE;
		}
		// Identify USB devices via 'ID_PATH=xxx-usb-xxx'.
		// Example:
		// ID_PATH=pci-0000:02:02.0-usb-0:1:1.0-scsi-0:0:0:0
		// ID_PATH=pci-0000:00:12.2-usb-0:3:1.0-scsi-0:0:0:0
		if (FALSE !== ($property = $this->getUdevProperty("ID_PATH"))) {
			if (1 == preg_match('/^.+-usb-.+$/i', $property))
				return TRUE;
		}
		return FALSE;
	}

	/**
	 * Check if the given device is connected via ATA.
	 * @return TRUE if the device is connected via ATA, otherwise FALSE.
	 */
	public function isAta() {
		if (FALSE === ($property = $this->getUdevProperty("ID_BUS")))
			return FALSE;
		return (0 == strcasecmp("ata", $property)) ? TRUE : FALSE;
	}

	/**
	 * Check if the given device is read-only.
	 * @return TRUE if the device is read-only, otherwise FALSE.
	 */
	public function isReadOnly() {
		return FALSE;
	}

	/**
	 * Queries the udev database for device information stored in the udev
	 * database.
	 * @param id The name of the property, e.g. ID_VENDOR, ID_MODEL or
	 *   ID_SERIAL_SHORT.
	 * @return The requested property, otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	protected function getUdevProperty($id) {
		// Do we need to get the udev information?
		if (FALSE === $this->udevInfo) {
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
			$this->udevInfo = [];
			foreach ($output as $row) {
				$data = explode("=", $row);
				if (empty($data))
					continue;
				$key = trim($data[0]);
				$this->udevInfo[$key] = $data[1];
			}
		}
		if (!array_key_exists($id, $this->udevInfo))
			return FALSE;
		return $this->udevInfo[$id];
	}

	/**
	 * Check if the given device has S.M.A.R.T. support.
	 * @return TRUE if the device supports S.M.A.R.T., otherwise FALSE.
	 */
	public function hasSmartSupport() {
		return ($this instanceof StorageDeviceSmartAbstract) ? TRUE : FALSE;
	}

	/**
	 * Assert that the given device has S.M.A.R.T. support.
	 * @return None.
	 * @throw \OMV\AssertException
	 */
	public function assertHasSmartSupport() {
		if (FALSE === $this->hasSmartSupport()) {
			throw new \OMV\AssertException(
			  "Device '%s' does not support S.M.A.R.T.",
			  $this->getDeviceFile());
		}
	}
}
