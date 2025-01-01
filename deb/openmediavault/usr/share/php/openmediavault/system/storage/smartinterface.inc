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
 * Interface for S.M.A.R.T. AT Attachment (ATA) devices.
 * @ingroup api
 */
interface SmartInterface {
	/**
	 * Identify the device type required by the smartctl utility program.
	 * @see https://www.smartmontools.org/browser/trunk/smartmontools/smartctl.8.in
	 * @return string Returns the identified device type, e.g. 'sat', 'scsi',
	 *   'usbcypress', ... or an empty string.
	 */
	public function getSmartDeviceType();

	/**
	 * Get the S.M.A.R.T. information.
	 * @return SmartInformation An \OMV\System\Storage\SmartInformation object.
	 * @throw \OMV\Exception
	 */
	public function getSmartInformation();

	/**
	 * Execute a S.M.A.R.T. self test.
	 * @param string test The test to be used. This can be:
	 *   <ul>
	 *   \li short (s)
	 *   \li long (l)
	 *   \li conveyance (c)
	 *   \li offline (o)
	 *   </ul>
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function executeSmartSelfTest($test);
}
