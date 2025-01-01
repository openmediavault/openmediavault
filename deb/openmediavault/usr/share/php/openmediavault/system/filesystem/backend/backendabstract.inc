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
namespace OMV\System\Filesystem\Backend;

/**
 * The generic class that represents a filesystem backend.
 * @ingroup api
 */
abstract class BackendAbstract {
	const PROP_NONE = 0x0;
	const PROP_MNTENT = 0x1;
	const PROP_POSIX_ACL = 0x2;
	const PROP_DISCARD = 0x4;
	const PROP_QUOTA = 0x8;
	const PROP_RESIZE = 0x10;
	const PROP_READ_ONLY = 0x20;
	const PROP_COMPRESS = 0x40;
	const PROP_AUTO_DEFRAG = 0x80;
	const PROP_MNTENT_DISCARD = 0x100;

	protected $type = "";
	protected $properties = self::PROP_NONE;
	protected $mkfsOptions = "";
	protected $mntOptions = [];

	/**
	 * Get the type of the filesystem, e.g. 'ext3', 'vfat' or 'btrfs'.
	 */
	final public function getType() {
		return strtolower($this->type);
	}

	/**
	 * Get a list of filesystem device files or identifiers that are
	 * managed by this filesystem backend.
	 * Override this method if the filesystem implemented by this backend
	 * is not identified by the block device identification library.
	 * This method is only called when the method \ref isBlkidEnumerated
	 * returns FALSE.
	 * @return An array of associative arrays with the fields \em devicefile,
	 *   \em uuid, \em label and \em type.
	 */
	public function enumerate() {
		return [];
	}

	/**
	 * Check whether the filesystem implemented by this backend is
	 * identified by the block device identification library. If this is
	 * not the case, then the backend must override the \ref enumerate
	 * method.
	 * @return TRUE if the filesystem is identified by the block device
	 *   identification library, otherwise FALSE.
	 */
	public function isBlkidEnumerated() {
		return TRUE;
	}

	/**
	 * Process the filesystems that are identified by the block device
	 * identification library.
	 * @param enums An array of associative arrays of the detected
	 *   filesystems. An associative array contains at least the fields
	 *   \em devicefile, \em uuid, \em label and \em type.
	 * @return Returns an array of associative arrays of the processed
	 *   filesystems.
	 */
	public function enumerateByBlkid(array $enums) {
		return $enums;
	}

	/**
	 * Check whether the given filesystem identifier is represented by this
	 * filesystem backend.
	 * @param id The filesystem identifier (e.g. UUID or device path), e.g.
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
	 * @return TRUE if successful, otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function isTypeOf($id) {
		if (!is_devicefile($id)) {
			// Get the device file containing the file system. This is
			// required for the blkid low-level probing mode.
			$cmdArgs = [];
			$cmdArgs[] = sprintf("UUID=%s", escapeshellarg($id));
			$cmd = new \OMV\System\Process("findfs", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			$id = $output[0];
			unset($cmdArgs);
			unset($output);
		}
		// Get the filesystem type.
		// !!! Note, do not use the '-p' option here. !!!
		$cmdArgs = [];
		$cmdArgs[] = "-o value";
		$cmdArgs[] = "-s TYPE";
		$cmdArgs[] = escapeshellarg($id);
		$cmd = new \OMV\System\Process("blkid", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		if (empty($output))
			return FALSE;
		// Compare the filesystem type.
		return ($output[0] == $this->getType()) ? TRUE : FALSE;
	}

	/**
	 * Get the filesystem properties.
	 * @return The properties defined for this filesystem.
	 */
	final public function getProperties() {
		return $this->properties;
	}

	/**
	 * Check whether the filesystem has the given property.
	 * @param property The property to check for, e.g. PROP_POSIX_ACL.
	 * @return TRUE if the given property is set, otherwise FALSE.
	 */
	final public function hasProperty($property) {
		return ($this->properties & $property) === $property;
	}

	/**
	 * Check whether the filesystem supports the discard/TRIM commands to
	 * the underlying block device when blocks are freed.
	 * See https://wiki.archlinux.org/index.php/Solid_State_Drives#TRIM.
	 * @return TRUE if the filesystem supports the discard/TRIM commands,
	 *   otherwise FALSE.
	 */
	public function hasDiscardSupport() {
		return $this->hasProperty(self::PROP_DISCARD);
	}

	/**
	 * Check whether the filesystem should be mounted with the 'discard'
	 * mount option.
	 * @return TRUE if the filesystem should be mounted with 'discard'
	 *   option, otherwise FALSE.
	 */
	public function hasFstabDiscardSupport() {
		if (!$this->hasDiscardSupport())
			return FALSE;
		return $this->hasProperty(self::PROP_MNTENT_DISCARD);
	}

	/**
	 * Does the filesystem support POSIX ACL.
	 * @see http://de.wikipedia.org/wiki/Access_Control_List
	 * @see http://www.suse.de/~agruen/acl/linux-acls/online
	 * @return TRUE if the filesystem supports POSIX ACL, otherwise FALSE.
	 */
	public function hasPosixAclSupport() {
		return $this->hasProperty(self::PROP_POSIX_ACL);
	}

	/**
	 * Is the filesystem mounted via /etc/fstab?
	 * @return TRUE if the filesystem is mounted via '/etc/fstab',
	 *   otherwise FALSE.
	 */
	public function hasFstabSupport() {
		return $this->hasProperty(self::PROP_MNTENT);
	}

	/**
	 * Does the filesystem support disc quota?
	 * @return TRUE if the filesystem supports disc quota, otherwise FALSE.
	 */
	public function hasQuotaSupport() {
		return $this->hasProperty(self::PROP_QUOTA);
	}

	/**
	 * Does the filesystem support the capability to resize it online?
	 * @see http://wiki.ubuntuusers.de/Dateisystemgr%C3%B6%C3%9Fe_%C3%A4ndern
	 * @return TRUE if the filesystem supports online resizing, otherwise FALSE.
	 */
	public function hasResizeSupport() {
		return $this->hasProperty(self::PROP_RESIZE);
	}

	/**
	 * Is the filesystem read-only, e.g. ISO9660?
	 * @return TRUE if the filesystem is read-only, otherwise FALSE.
	 */
	public function hasReadOnlySupport() {
		return $this->hasProperty(self::PROP_READ_ONLY);
	}

	/**
	 * Does the filesystem support compression?
	 * @return TRUE if the filesystem supports compression, otherwise FALSE.
	 */
	public function hasCompressSupport() {
		return $this->hasProperty(self::PROP_COMPRESS);
	}

	/**
	 * Does the filesystem support auto defragmentation?
	 * @return TRUE if the filesystem supports auto-defrag, otherwise FALSE.
	 */
	public function hasAutoDefragSupport() {
		return $this->hasProperty(self::PROP_AUTO_DEFRAG);
	}

	/**
	 * Does the filesystem have a device file? E.g. union mount or overlay
	 * file systems like UnionFS, aufs or mhddfs do not have such a device
	 * file.
	 * @return TRUE if the filesystem has a device file, otherwise FALSE.
	 */
	public function hasDeviceFile() {
		return TRUE;
	}

	/**
	 * Get filesystem mount options used in '/etc/fstab'.
	 * See http://linux.die.net/man/8/mount.
	 * If a storage device is given the result mount option list can contain
	 * specific options for the given storage device.
	 * @param sd A optional storage device object. Defaults to NULL.
	 * @return An array of 'mount' options.
	 */
	public function getFstabMntOptions(
	  \OMV\System\Storage\StorageDevice $sd = null) {
		$options = $this->mntOptions;
		// Append additional mount options depending on the storage device.
		if (!is_null($sd)) {
			// Check if the device is a non-rotational type (e.g. SSD, DOM,
			// CF card, USB stick, ...).
			// In this case optimize the mount options:
			// - add 'discard' to support the ATA_TRIM command, see
			//   http://en.wikipedia.org/wiki/TRIM.
			// - add 'noatime' to do not update inode access times on the
			//   filesystem.
			// - add 'nodiratime' to do not update directory inode access
			//   times.
			if (FALSE === $sd->isRotational()) {
				// File systems that support TRIM.
				// See https://wiki.archlinux.org/index.php/Solid_State_Drives#TRIM
				if ($this->hasFstabDiscardSupport()) {
					array_push($options, "discard");
				}
				//array_push($options, "noatime");
				//array_push($options, "nodiratime");
			}
			// Append the 'nofail' option if the device is removable to
			// prevent an error during the boot process when the device is
			// unplugged.
			// !!! Note, if the device is unplugged then it will cause other
			// !!! problems like writing data into the root filesystem.
			if (TRUE === $sd->isRemovable()) {
				array_push($options, "nofail");
			}
		}
		// Remove duplicates and re-index the array.
		return array_values(array_unique($options));
	}

	/**
	 * Get 'mkfs' command options used to create the filesystem.
	 * See http://linux.die.net/man/8/mkfs.
	 * If a storage device is given the result command option list can contain
	 * specific options for the given storage device.
	 * @param sd A optional storage device object. Defaults to NULL.
	 * @return An array of 'mkfs' options.
	 */
	public function getMkfsOptions(
	  \OMV\System\Storage\StorageDevice $sd = null) {
		return $this->mkfsOptions;
	}

	/**
	 * Get the object of the class that represents and implements a filesystem
	 * of this filesystem backend.
	 * @param args The arguments to the class constructor.
	 * @return The object of the class implementing the given filesystem type.
	 */
	function getImpl($args) {
		$object = new \OMV\System\Filesystem\Filesystem($args);
		$object->setBackend($this);
		return $object;
	}
}
