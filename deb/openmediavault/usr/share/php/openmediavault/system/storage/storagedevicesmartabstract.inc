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
 * Abstract class implementing generic storage devices that support the
 * S.M.A.R.T. AT Attachment (ATA) interface.
 * @ingroup api
 */
abstract class StorageDeviceSmartAbstract extends StorageDevice {
	protected $smartDeviceType = "";

	/**
	 * Set the device type required by the smartctl command.
	 * @return Returns the device type.
	 */
	public function setSmartDeviceType($type) {
		$this->smartDeviceType = $type;
		return $this->smartDeviceType;
	}

	/**
	 * Identify the device type required by the smartctl command.
	 * @return Returns the identified device type or an empty string.
	 */
	public function getSmartDeviceType() {
		return $this->smartDeviceType;
	}

	/**
	 * Get the S.M.A.R.T. information.
	 * @throw \OMV\Exception
	 */
	public function getSmartInformation() {
		$smartInfo = new SmartInformation($this->getDeviceFile(),
		  $this->getSmartDeviceType(), $this);
		if (is_null($smartInfo)) {
			throw new \OMV\Exception(
			  "Failed to get S.M.A.R.T. interface for device '%s'.",
			  $this->getDeviceFile());
		}
		return $smartInfo;
	}
}
