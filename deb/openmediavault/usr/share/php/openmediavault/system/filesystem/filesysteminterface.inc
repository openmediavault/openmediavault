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
namespace OMV\System\Filesystem;

/**
 * Interface for filesystems.
 * @ingroup api
 */
interface FilesystemInterface {
	/**
	 * Checks if the device exists.
	 * @return TRUE if the device exists, otherwise FALSE.
	 */
	public function exists();

	/**
	 * Get the device file, e.g. <ul>
	 * \li /dev/sda1
	 * \li /dev/mapper/vg0-lv0
	 * \li /dev/disk/by-uuid/fc3e1da5-fd8d-4fda-341e-d0135efa7a7c
	 * \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6-part1
	 * \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0-part2
	 * </ul>
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
	 * \li /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML-part2
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
	 * Check whether the filesystem has a /dev/disk/by-id/xxx device path.
	 * @return Returns TRUE if a disk/by-id device path exists,
	 *   otherwise FALSE.
	 */
	public function hasDeviceFileById();

	/**
	 * Get the device path by UUID, e.g. <ul>
	 * \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 * \li /dev/disk/by-uuid/2ED43920D438EC29 (NTFS)
	 * </ul>
	 * @return The device file (/dev/disk/by-uuid/xxx) if available,
	 *   otherwise NULL will be returned.
	 */
	public function getDeviceFileByUuid();

	/**
	 * Check whether the filesystem has a /dev/disk/by-uuid/xxx device path.
	 * @return Returns TRUE if a disk/by-uuid device path exists,
	 *   otherwise FALSE.
	 */
	public function hasDeviceFileByUuid();

	/**
	 * Get the device path by label, e.g. <ul>
	 * \li /dev/disk/by-label/data
	 * </ul>
	 * @return The device file (/dev/disk/by-label/xxx) if available,
	 *   otherwise NULL will be returned.
	 */
	public function getDeviceFileByLabel();

	/**
	 * Check whether the filesystem has a /dev/disk/by-label/xxx device path.
	 * @return Returns TRUE if a disk/by-label device path exists,
	 *   otherwise FALSE.
	 */
	public function hasDeviceFileByLabel();

	/**
	 * Get a predictable device file in the following order:
	 * <ul>
	 * \li /dev/disk/by-label/xxx
	 * \li /dev/disk/by-id/xxx
	 * \li /dev/disk/by-path/xxx
	 * \li /dev/xxx
	 * </ul>
	 * @return Returns a device file.
	 */
	public function getPredictableDeviceFile();

	/**
	 * Get the device file to present in the UI, e.g.:
	 * <ul>
	 * \li /dev/disk/by-label/xxx
	 * \li /dev/disk/by-id/xxx
	 * \li /dev/disk/by-path/xxx
	 * \li /dev/xxx
	 * </ul>
	 * @return Returns a device file.
	 */
	public function getPreferredDeviceFile();

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
	 * Get all devices that make up the filesystem.
	 * @return An array that contains the component devices of the filesystem.
	 */
	public function getDeviceFiles();

	/**
	 * Get the device file of the storage device which contains this
	 * file system.
	 * Example:
	 * <ul>
	 * \li /dev/sdb1 => /dev/sdb
	 * \li /dev/cciss/c0d0p2 => /dev/cciss/c0d0
	 * </ul>
	 * @return The device file of the underlying storage device or NULL
	 *   on failure.
	 */
	public function getParentDeviceFile();

	/**
	 * Check if the filesystem has an UUID, e.g. <ul>
	 * \li 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 * \li 7A48-BA97 (FAT)
	 * \li 2ED43920D438EC29 (NTFS)
	 * </ul>
	 * @see http://wiki.ubuntuusers.de/UUID
	 * @return Returns TRUE if the filesystem has an UUID, otherwise FALSE.
	 */
	public function hasUuid();

	/**
	 * Get the UUID of the filesystem.
	 * @see http://wiki.ubuntuusers.de/UUID
	 * @return Returns the UUID of the filesystem, otherwise an empty
	 *   string.
	 */
	public function getUuid();

	/**
	 * Check if the filesystem has a label.
	 * @return Returns TRUE if the filesystem has a label, otherwise FALSE.
	 */
	public function hasLabel();

	/**
	 * Get the filesystem label.
	 * @return Returns the label of the filesystem, otherwise an empty
	 *   string.
	 */
	public function getLabel();

	/**
	 * Get the filesystem type, e.g. 'ext3' or 'vfat'.
	 * @return The filesystem type.
	 */
	public function getType();

	/**
	 * Get the size of the device in bytes.
	 * @return The size of the device in bytes.
	 * @throw \OMV\Exception
	 */
	public function getSize();

	/**
	 * Get the description (e.g. incl. additional information like used
	 * and available disk space usage) of the file system.
	 * @return The file system description.
	 */
	public function getDescription();

	/**
	 * Get statistics about a mounted filesystem.
	 * @return The filesystem statistics as an array if successful,
	 *   otherwise FALSE. The following fields are included:
	 *   \em devicefile, \em type, \em blocks, \em size, \em used,
	 *   \em available, \em percentage and \em mountpoint.
	 *   Please note, the fields \em size, \em used and \em available are
	 *   strings and their unit is 'B' (bytes).
	 * @throw \OMV\ExecException
	 */
	public function getStatistics();

	/**
	 * Get details about the filesystem, e.g. the health status.
	 * @return A string that contains information and details about the
	 *   filesystem.
	 * @throw \OMV\ExecException
	 */
	public function getDetails();

	/**
	 * Check if a filesystem is mounted.
	 * @return TRUE if the filesystem is mounted, otherwise FALSE.
	 * @throw \OMV\Exception
	 */
	public function isMounted();

	/**
	 * Mount the filesystem by its device file or UUID.
	 * @param options Additional mount options.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function mount($options);

	/**
	 * Unmount the file system.
	 * @param force Set to TRUE to force unmount. Defaults to FALSE.
	 * @param lazy Set to TRUE to lazy unmount. Defaults to FALSE.
	 * @param directory Set to TRUE to unmount the file system using
	 * the directory where it has been mounted, otherwise the device
	 * file is used. Defaults to FALSE.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function umount($force, $lazy, $directory);

	/**
	 * Grow the filesystem.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function grow();

	/**
	 * Shrink the filesystem.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function shrink();

	/**
	 * Remove the filesystem.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function remove();
}
