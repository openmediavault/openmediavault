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
 * Interface for S.M.A.R.T. AT Attachment (ATA) devices.
 * @ingroup api
 */
interface ISmartStorageDevice {
	/**
	 * Check if the given device has S.M.A.R.T. support.
	 * @return TRUE if the device supports S.M.A.R.T., otherwise FALSE.
	 */
	public function hasSmartSupport();

	/**
	 * Identify the device type required by the smartctl utility program.
	 * @see https://www.smartmontools.org/browser/trunk/smartmontools/smartctl.8.in
	 * @return Returns the identified device type, e.g. 'sat', 'scsi',
	 *   'usbcypress', ... or an empty string.
	 */
	public function getSmartDeviceType();

	/**
	 * Get the S.M.A.R.T. information.
	 * @return An object of the \OMV\System\Storage\SmartInformation class.
	 * @throw \OMV\Exception
	 */
	public function getSmartInformation();
}
