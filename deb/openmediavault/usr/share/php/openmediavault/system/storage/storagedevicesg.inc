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
 * This class provides a simple interface to handle SCSI generic devices.
 * @ingroup api
 */
class StorageDeviceSG implements SmartInterface {
	use SmartTrait;

	protected $deviceFile = NULL;

	/**
	 * Constructor
	 * @param deviceFile The device file, e.g.
	 *   <ul>
	 *   \li /dev/sg0
	 *   \li /dev/sg21
	 *   </ul>
	 */
	public function __construct($deviceFile) {
		$this->deviceFile = $deviceFile;
	}

	public function exists() {
		return is_char_device($this->getDeviceFile());
	}

	public function getDeviceFile() {
		return $this->deviceFile;
	}

	public function getDeviceName() {
		return str_replace("/dev/", "", $this->getDeviceFile());
	}

	public function getDeviceNumber() {
		$filename = sprintf("/sys/class/scsi_generic/%s/dev",
			$this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return trim(file_get_contents($filename));
	}

	/**
	 * Get the SCSI type.
	 * @see http://www.tldp.org/HOWTO/SCSI-Generic-HOWTO/proc.html
	 * @return integer The SCSI type, e.g. 0->disk, 5->cdrom, 6->scanner,
	 *   otherwise FALSE.
	 */
	public function getType() {
		$filename = sprintf("/sys/class/scsi_generic/%s/device/type",
			$this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return intval(trim(file_get_contents($filename)));
	}

	/**
	 * Get the mapped block device.
	 * @return string|boolean The mapped block device file, otherwise FALSE.
	 */
	public function getMappedDeviceFile() {
		$pattern = sprintf("/sys/class/scsi_generic/%s/device/block/*",
			$this->getDeviceName());
		$deviceNames = glob($pattern);
		if (empty($deviceNames))
			return FALSE;
		if (!sort($deviceNames, SORT_NATURAL))
			return FALSE;
		return sprintf("/dev/%s", basename($deviceNames[0]));
	}

	public function hasMappedDeviceFile() {
		return (FALSE !== $this->getMappedDeviceFile());
	}

	/**
	 * Get the device model.
	 * @return The device model, otherwise an empty string.
	 */
	public function getModel() {
		$filename = sprintf("/sys/class/scsi_generic/%s/device/model",
		  $this->getDeviceName());
		if (file_exists($filename))
			return trim(file_get_contents($filename));
		return "";
	}

	/**
	 * Get the device vendor.
	 * @return The device vendor, otherwise an empty string.
	 */
	public function getVendor() {
		$filename = sprintf("/sys/class/scsi_generic/%s/device/vendor",
		  $this->getDeviceName());
		if (file_exists($filename))
			return trim(file_get_contents($filename));
		return "";
	}

	/**
	 * See interface definition.
	 */
	public function getDescription() {
		$model = $this->getModel();
		return sprintf("%s [%s]", !empty($model) ? $model : gettext("n/a"),
			$this->getDeviceFile());
	}

	/**
	 * See \OMV\System\Storage\SmartInterface interface definition.
	 */
	public function getSmartDeviceType() {
		return "scsi";
	}
}
