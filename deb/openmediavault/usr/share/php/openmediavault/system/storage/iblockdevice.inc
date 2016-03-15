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
 * Class implementing generic block devices.
 * @ingroup api
 */
interface IBlockDevice {
	/**
	 * Checks if the device exists.
	 * @return TRUE if the device exists, otherwise FALSE.
	 */
	function exists();

	/**
	 * Get the device path, e.g. /dev/sda.
	 * @return The device path.
	 */
	function getDeviceFile();

	/**
	 * Get the device name, e.g. sda or hdb.
	 * @param canonical If set to TRUE the canonical device file will
	 *   be used. Defaults to FALSE.
	 * @return The device name.
	 */
	function getDeviceName($canonical = FALSE);

	/**
	 * Get the device number, e.g. 8:17.
	 * @link See https://www.kernel.org/doc/Documentation/devices.txt for
	 *   more information. @endlink
	 * @return The device number as string or FALSE on failure.
	 */
	function getDeviceNumber();

	/**
	 * Get the major device number.
	 * @link See https://www.kernel.org/doc/Documentation/devices.txt for
	 *   more information. @endlink
	 * @return The major device number or FALSE on failure.
	 */
	function getMajor();

	/**
	 * Get the minor device number.
	 * @link See https://www.kernel.org/doc/Documentation/devices.txt for
	 *   more information. @endlink
	 * @return The minor device number or FALSE on failure.
	 */
	function getMinor();

	/**
	 * Get the size of the device in bytes.
	 * @return The size of the device in bytes, otherwise FALSE.
	 */
	function getSize();

	/**
	 * Get the blocksize of the device in bytes.
	 * @return The blocksize of the device in bytes, otherwise FALSE.
	 */
	function getBlockSize();

	/**
	 * Get the sectorsize of the device in bytes.
	 * @return The sectorsize of the device in bytes, otherwise FALSE.
	 */
	function getSectorSize();

	/**
	 * Get the description of the device.
	 * @return The device description.
	 */
	function getDescription();
}
