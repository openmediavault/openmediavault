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

require_once("openmediavault/functions.inc");

/**
 * @ingroup api
 */
class StorageDevice extends \OMV\System\BlockDevice
		implements StorageDeviceInterface {
	/**
	 * Get the device model (via udev or sysfs).
	 * @return string The device model, otherwise an empty string.
	 *   Note, underscores are replaced by blanks when the value is
	 *   retrieved via udev.
	 */
	public function getModel() {
		if (FALSE === $this->hasUdevProperty("ID_MODEL")) {
			$filename = sprintf("/sys/block/%s/device/model",
				$this->getDeviceName(TRUE));
			if (file_exists($filename))
				return trim(file_get_contents($filename));
			return "";
		}
		$property = $this->getUdevProperty("ID_MODEL");
		return str_replace("_", " ", $property);
	}

	/**
	 * Get the device vendor (via udev or sysfs).
	 * @return string The device vendor, otherwise an empty string.
	 *   Note, underscores are replaced by blanks when the value is
	 *   retrieved via udev.
	 */
	public function getVendor() {
		if (FALSE === $this->hasUdevProperty("ID_VENDOR")) {
			$filename = sprintf("/sys/block/%s/device/vendor",
				$this->getDeviceName(TRUE));
			if (file_exists($filename))
				return trim(file_get_contents($filename));
			return "";
		}
		$property = $this->getUdevProperty("ID_VENDOR");
		return str_replace("_", " ", $property);
	}

	/**
	 * Get the device serial number.
	 * @return string The device serial number, otherwise an empty string.
	 *   Note, underscores are replaced by blanks.
	 */
	public function getSerialNumber() {
		if (FALSE === $this->hasUdevProperty("ID_SERIAL_SHORT"))
			return "";
		$property = $this->getUdevProperty("ID_SERIAL_SHORT");
		return str_replace("_", " ", $property);
	}

	/**
	 * Get the World Wide Name (WWN) identifier.
	 * @return string The WWN identifier, otherwise an empty string.
	 */
	public function getWorldWideName() {
		if (FALSE === $this->hasUdevProperty("ID_WWN"))
			return "";
		return $this->getUdevProperty("ID_WWN");
	}

	/**
	 * See interface definition.
	 */
	public function getDescription() {
		$model = $this->getModel();
		return sprintf("%s [%s, %s]", !empty($model) ? $model : gettext("n/a"),
			$this->getPreferredDeviceFile(), binary_format($this->getSize()));
	}

	/**
	 * Check if the device is of rotational or non-rotational type.
	 * See https://www.kernel.org/doc/Documentation/block/queue-sysfs.txt
	 * @return boolean TRUE if device is rotational, otherwise FALSE.
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
		  $this->getDeviceName(TRUE));
		if (file_exists($filename)) {
			// 0 => SSD, 1 => HDD
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
	* @return boolean TRUE if device is removable, otherwise FALSE.
	*/
	public function isRemovable() {
		$filename = sprintf("/sys/block/%s/removable",
		  $this->getDeviceName(TRUE));
		if (!file_exists($filename))
			return FALSE;
		return (trim(file_get_contents($filename)) == "1") ? TRUE : FALSE;
	}

	/**
	 * Check if the given device is a hardware/software RAID device.
	 * @return boolean TRUE if the device is a hardware/software RAID,
	 *   otherwise FALSE.
	 */
	public function isRaid() {
		return FALSE;
	}

	/**
	 * Check if the given device is an USB device.
	 * @return boolean TRUE if the device is connected via USB,
	 *   otherwise FALSE.
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
	 * @return boolean TRUE if the device is connected via ATA,
	 *   otherwise FALSE.
	 */
	public function isAta() {
		if (FALSE === $this->hasUdevProperty("ID_BUS"))
			return FALSE;
		$property = $this->getUdevProperty("ID_BUS");
		return (0 == strcasecmp("ata", $property)) ? TRUE : FALSE;
	}

	/**
	 * Check if the given device is an SD or MMC device.
	 * @return boolean TRUE if device is SD or MMC, otherwise FALSE.
	 */
	public function isMmc() {
		return FALSE;
	}

	/**
	 * Check if the given device is read-only.
	 * @return boolean TRUE if the device is read-only, otherwise FALSE.
	 */
	public function isReadOnly() {
		return FALSE;
	}

	/**
	 * Check if a medium is available. The method should always return TRUE
	 * for devices without removable media.
	 * @return boolean Returns FALSE if no medium is available.
	 */
	public function IsMediaAvailable() {
		return TRUE;
	}

	/**
	 * Check if the given device is hot pluggable.
	 * @return boolean TRUE if the device is hot pluggable, otherwise FALSE.
	 */
	public function isHotPluggable(): bool {
		return $this->isUsb();
	}

	/**
	 * Check if the given device has S.M.A.R.T. support.
	 * @return boolean Returns TRUE if the device supports S.M.A.R.T.,
	 *   otherwise FALSE.
	 */
	public function hasSmartSupport() {
		// Check whether this object implements the SmartInterface interface.
		return is_a($this, "\OMV\System\Storage\SmartInterface");
	}

	/**
	 * Assert that the given device has S.M.A.R.T. support.
	 * @return void
	 * @throw \OMV\AssertException
	 */
	public function assertHasSmartSupport() {
		if (FALSE === $this->hasSmartSupport()) {
			throw new \OMV\AssertException(
				"Device '%s' does not support S.M.A.R.T.",
				$this->getDeviceFile());
		}
	}

	/**
	 * Wipe the storage device.
	 * @return void
	 */
	public function wipe() {
		$cmdArgs = [];
		$cmdArgs[] = "--zap-all";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("sgdisk", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Get the device names of the partitions of this device.
	 * @return array Returns an array of device names.
	 */
	public function getPartitionDeviceNames() {
		$result = [];
		$pattern = sprintf("/sys/block/%s/*/partition",
			$this->getDeviceName());
		foreach (glob($pattern) as $path) {
			$result[] = basename(substr($path, 0, -10));
		}
		return $result;
	}

	/**
	 * Enumerate devices matching the given storage device type.
	 * @param integer type Defines the storage device type, e.g. hard disk,
	 *   hard or Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return array|boolean A list of devicefile names, otherwise FALSE.
	 */
	public static function enumerate($type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		$result = [];
		$mngr = Backend\Manager::getInstance();
		foreach ($mngr as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($devs = $backendv->enumerate()))
				return FALSE;
			$result = array_merge($result, $devs);
		}
		// Remove duplicate values.
		$result = array_unique($result);
		// Sort the values using a "natural order" algorithm.
		if (!sort($result, SORT_NATURAL))
			return FALSE;
		return $result;
	}

	/**
	 * Enumerate all unused devices. This list contains all devices of the
	 * given type except the devices that are used by other storage devices
	 * (e.g. LVM as physical volume or a member of a Software RAID).
	 * @param integer type Defines the storage device type, e.g. hard disk,
	 *   hard or Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return array|boolean A list of devicefile names, otherwise FALSE.
	 */
	public static function enumerateUnused($type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		// Append all available storage devices.
		if (FALSE === ($result = self::enumerate($type)))
			return FALSE;
		// Remove used devices.
		$mngr = Backend\Manager::getInstance();
		foreach ($mngr as $backendk => $backendv) {
// Always collect and remove all slave devices.
//			if (!($type & $backendv->getType()))
//				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (empty($slaves))
				continue;
			$result = array_diff($result, $slaves);
		}
		// Remove duplicate values.
		$result = array_unique($result);
		// Sort the values using a "natural order" algorithm.
		if (!sort($result, SORT_NATURAL))
			return FALSE;
		return $result;
	}

	/**
	 * Enumerate all used devices. The list contains all those devices that
	 * are used by the given storage devices, e.g. all members of a LVM
	 * or Software RAID.
	 * @param integer type Defines the storage device type, e.g. hard disk,
	 *   hard or Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return array|boolean A list of devicefile names, otherwise FALSE.
	 */
	public static function enumerateUsed($type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		$result = [];
		$mngr = Backend\Manager::getInstance();
		foreach ($mngr as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (empty($slaves))
				continue;
			$result = array_merge($result, $slaves);
		}
		// Remove duplicate values.
		$result = array_unique($result);
		// Sort the values using a "natural order" algorithm.
		if (!sort($result, SORT_NATURAL))
			return FALSE;
		return $result;
	}

	/**
	 * Check if the given device is used/consumed by another storage device.
	 * @param string deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @param integer type Defines the storage device type, e.g. hard disk,
	 *   hard or Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return boolean TRUE if the given device is used/consumed by another
	 *   storage device, otherwise FALSE.
	 */
	public static function isUsed($deviceFile, $type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		$result = FALSE;
		$mngr = Backend\Manager::getInstance();
		foreach ($mngr as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (in_array($deviceFile, $slaves)) {
				$result = TRUE;
				break;
			}
		}
		return $result;
	}

	/**
	 * Factory method to get the object of the class which implements the
	 * given storage device.
	 * @param string deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @return object|null The object of the class implementing the given
	 *   storage device, otherwise NULL.
	 */
	public static function getStorageDevice($deviceFile) {
		$mngr = Backend\Manager::getInstance();
		if (NULL == ($backend = $mngr->getBackend($deviceFile)))
			return NULL;
		$result = $backend->getImpl($deviceFile);
		if (is_null($result) || !$result->exists())
			return NULL;
		return $result;
	}

	/**
	 * @see StorageDevice::getStorageDevice()
	 * @return object The object of the class implementing the given
	 *   storage device.
	 * @throw \OMV\AssertException
	 */
	public static function assertGetStorageDevice($deviceFile) {
		$result = self::getStorageDevice($deviceFile);
		if (is_null($result) || !$result->exists()) {
			throw new \OMV\AssertException("Device '%s' does not exist.",
				$deviceFile);
		}
		return $result;
	}
}
