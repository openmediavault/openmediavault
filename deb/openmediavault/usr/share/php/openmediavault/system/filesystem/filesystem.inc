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
namespace OMV\System\Filesystem;

/**
 * Class to get detailed information about a filesystem.
 * @ingroup api
 */
class Filesystem extends FilesystemAbstract {
	/**
	 * Get all available/detected filesystems.
	 * @return An array of \ref Filesystem objects.
	 * @throw \OMV\Exception
	 */
	public static function getFilesystems() {
		$result = [];
		$mngr = Backend\Manager::getInstance();
		foreach ($mngr->enumerate() as $datak => $datav) {
			// Get the filesystem backend.
			$mngr->assertBackendExistsByType($datav['type']);
			$backend = $mngr->getBackendByType($datav['type']);
			// Get the filesystem implementation. Use the devicefile to get
			// the filesystem details because VFAT filesystems do not have
			// a valid UUID.
			$fs = $backend->getImpl($datav['devicefile']);
			if (is_null($fs) || !$fs->exists()) {
				throw new \OMV\Exception(
				  "Failed to get the '%s' filesystem implementation ".
				  "or '%s' does not exist.", $backend->getType(),
				  $datav['devicefile']);
			}
			$result[] = $fs;
		}
		return $result;
	}

	/**
	 * Get the object of the class which implements the specified filesystem
	 * for the given filesystem UUID or device path.
	 * @param id The UUID or device path of the filesystem, e.g.
	 *   <ul>
	 *   \li 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 *   \li 7A48-BA97 (FAT)
	 *   \li 2ED43920D438EC29 (NTFS)
	 *   \li /dev/sde1
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6-part1
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0-part2
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   \li /dev/cciss/c0d0p2
	 *   </ul>
	 * @return The object of the class implementing the given filesystem,
	 *   otherwise NULL.
	 */
	public static function getImpl($id) {
		$mngr = Backend\Manager::getInstance();
		if (NULL == ($backend = $mngr->getBackendById($id)))
			return NULL;
		return $backend->getImpl($id);
	}

	/**
	 * @throw \OMV\AssertException
	 */
	public static function assertGetImpl($id) {
		$result = self::getImpl($id);
		if (is_null($result) || !$result->exists()) {
			throw new \OMV\Exception(
			  "Failed to get the filesystem implementation for '%s' ".
			  "or the filesystem does not exist.", $id);
		}
	}

	/**
	 * Get the object of the class which implements the specified filesystem
	 * for the given mount point.
	 * @param dir The file system path prefix, e.g. '/home/ftp/data'.
	 * @return The class object or NULL on failure.
	 */
	public static function getImplByMountPoint($dir) {
		$result = NULL;
		foreach (file("/proc/mounts") as $rowk => $rowv) {
			// Parse command output:
			// rootfs / rootfs rw 0 0
			// none /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0
			// none /proc proc rw,nosuid,nodev,noexec,relatime 0 0
			// tmpfs /dev/shm tmpfs rw,nosuid,nodev,relatime 0 0
			// /dev/sde1 /home/ftp/data ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			// /dev/sde1 /media/46607e80-dc8e-427e-9939-7909985dfe5b ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			$parts = explode(" ", $rowv);
			if ($matches[1] == $dir) {
				$result = self::getImpl($parts[0]);
				break;
			}
		}
		return $result;
	}
}
