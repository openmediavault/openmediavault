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
namespace OMV\System;

/**
 * Interface for block devices.
 * @ingroup api
 */
interface IBlockDevice {
	/**
	 * Checks if the device exists.
	 * @return TRUE if the device exists, otherwise FALSE.
	 */
	public function exists();

	/**
	 * Get the device file, e.g. /dev/sda.
	 * @return Returns the device file.
	 */
	public function getDeviceFile();

	/**
	 * Get the canonical device file, e.g. <ul>
	 * \li /dev/root -> /dev/sde1
	 * \li /dev/disk/by-uuid/4B04EA317E4AA567 -> /dev/sdd1
	 * \li /dev/mapper/vg0-lv0 -> /dev/dm-0
	 * </ul>
	 * @return Returns the canonical device file.
	 */
	public function getCanonicalDeviceFile();

	/**
	 * Get the device file by ID, e.g. <ul>
	 * \li /dev/disk/by-id/wwn-0x5000cca211cc703c
	 * \li /dev/disk/by-id/scsi-SATA_IBM-DHEA-36481_SG0SGF08038
	 * \li /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML-part2#
	 * </ul>
	 * The following order of paths will be retured if available: <ul>
	 * \li ata-xxx
	 * \li wwn-xxx
	 * \li scsi-xxx
	 * \li ...
	 * </ul>
	 * @return The device file (/dev/disk/by-id/xxx) if available,
	 *   otherwise NULL will be returned.
	 */
	public function getDeviceFileById();

	/**
	* Get all device file symlinks via udev, e.g. <ul>
	* \li /dev/disk/by-id/wwn-0x5000cca211cc703c
	* \li /dev/disk/by-id/scsi-SATA_IBM-DHEA-36481_SG0SGF08038
	* \li /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML
	* \li /dev/disk/by-path/pci-0000:00:02.5-scsi-0:0:0:0
	* \li /dev/disk/by-id/ata-WDC_WD15EARS-00MVWB0_WD-WMAZB2574325-part1
	* \li /dev/disk/by-uuid/fc3e1da5-fd8d-4fda-341e-d0135efa7a7c
	* </ul>
	* @return Returns an string array of device files.
	*/
	public function getDeviceFileSymlinks();

	/**
	 * Get the device name, e.g. sda or hdb.
	 * @param canonical If set to TRUE the canonical device file will
	 *   be used. Defaults to FALSE.
	 * @return The device name.
	 */
	public function getDeviceName($canonical = FALSE);

	/**
	 * Get the device number, e.g. 8:17.
	 * @link See https://www.kernel.org/doc/Documentation/devices.txt for
	 *   more information. @endlink
	 * @return The device number as string or FALSE on failure.
	 */
	public function getDeviceNumber();

	/**
	 * Get the major device number.
	 * @link See https://www.kernel.org/doc/Documentation/devices.txt for
	 *   more information. @endlink
	 * @return The major device number or FALSE on failure.
	 */
	public function getMajor();

	/**
	 * Get the minor device number.
	 * @link See https://www.kernel.org/doc/Documentation/devices.txt for
	 *   more information. @endlink
	 * @return The minor device number or FALSE on failure.
	 */
	public function getMinor();

	/**
	 * Get the size of the device in bytes.
	 * @return The size of the device in bytes, otherwise FALSE.
	 */
	public function getSize();

	/**
	 * Get the blocksize of the device in bytes.
	 * @return The blocksize of the device in bytes, otherwise FALSE.
	 */
	public function getBlockSize();

	/**
	 * Get the sectorsize of the device in bytes.
	 * @return The sectorsize of the device in bytes, otherwise FALSE.
	 */
	public function getSectorSize();

	/**
	 * Get the description of the device.
	 * @return The device description.
	 */
	public function getDescription();
}
