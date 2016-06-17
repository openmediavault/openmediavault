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
abstract class StorageDeviceAbstract extends \OMV\System\BlockDevice {
	protected $model = FALSE;
	protected $vendor = FALSE;

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
	 * Get the device model.
	 * @return The device model, otherwise an empty string.
	 */
	public function getModel() {
		if (FALSE === $this->model) {
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
		if (FALSE === $this->vendor) {
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
		if (FALSE === $this->hasUdevProperty("ID_SERIAL_SHORT"))
			return "";
		$property = $this->getUdevProperty("ID_SERIAL_SHORT");
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
		if (TRUE === $this->hasUdevProperty("ID_SSD")) {
			$property = $this->getUdevProperty("ID_SSD");
			// If ID_SSD is not 1 then it is rotational.
			return (0 == strcasecmp("1", $property)) ? FALSE : TRUE;
		}
		if (TRUE === $this->hasUdevProperty("ID_ATA_ROTATION_RATE_RPM")) {
			$property = $this->getUdevProperty("ID_ATA_ROTATION_RATE_RPM");
			// If ID_ATA_ROTATION_RATE_RPM is non-zero then it is rotational.
			return (0 == strcasecmp("0", $property)) ? FALSE : TRUE;
		}
		if (TRUE === $this->hasUdevProperty("ID_ATA_FEATURE_SET_AAM")) {
			$property = $this->getUdevProperty("ID_ATA_FEATURE_SET_AAM");
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
		if (TRUE === $this->hasUdevProperty("ID_BUS")) {
			$property = $this->getUdevProperty("ID_BUS");
			if (0 == strcasecmp("usb", $property))
				return TRUE;
		}
		// Identify USB devices via 'ID_USB_DRIVER=usb-storage'.
		if (TRUE === $this->hasUdevProperty("ID_USB_DRIVER")) {
			$property = $this->getUdevProperty("ID_USB_DRIVER");
			if (0 == strcasecmp("usb-storage", $property))
				return TRUE;
		}
		// Identify USB devices via 'ID_DRIVE_THUMB=1'.
		if (TRUE === $this->hasUdevProperty("ID_DRIVE_THUMB")) {
			$property = $this->getUdevProperty("ID_DRIVE_THUMB");
			if (0 == strcasecmp("1", $property))
				return TRUE;
		}
		// Identify USB devices via 'ID_PATH=xxx-usb-xxx'.
		// Example:
		// ID_PATH=pci-0000:02:02.0-usb-0:1:1.0-scsi-0:0:0:0
		// ID_PATH=pci-0000:00:12.2-usb-0:3:1.0-scsi-0:0:0:0
		if (TRUE === $this->hasUdevProperty("ID_PATH")) {
			$property = $this->getUdevProperty("ID_PATH");
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
		if (FALSE === $this->hasUdevProperty("ID_BUS"))
			return FALSE;
		$property = $this->getUdevProperty("ID_BUS");
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
