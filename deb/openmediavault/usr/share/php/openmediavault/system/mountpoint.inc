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
namespace OMV\System;

require_once("openmediavault/functions.inc");

/**
 * A class that implements a mountpoint.
 * @ingroup api
 */
class MountPoint {
	private $dir;

	/**
	 * Constructor
	 * @param dir Path to the directory, e.g.
	 *   <ul>
	 *   \li /srv/78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 *   \li /srv/7A48-BA97 (FAT)
	 *   \li /srv/2ED43920D438EC29 (NTFS)
	 *   </ul>
	 */
	public function __construct($dir) {
		$this->dir = $dir;
	}

	/**
	 * Get the directory path.
	 * @return The directory path.
	 */
	public function getPath() {
		return $this->dir;
	}

	/**
	 * Set the directory path, e.g.
	 *   <ul>
	 *   \li /srv/78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 *   \li /srv/7A48-BA97 (FAT)
	 *   \li /srv/2ED43920D438EC29 (NTFS)
	 *   </ul>
	 * @return void
	 */
	public function setPath($dir) {
		$this->dir = $dir;
	}

	/**
	 * Check whether the directory exists.
	 * @return Returns TRUE if the directory exists, otherwise FALSE.
	 */
	public function exists() {
		if (FALSE == file_exists($this->getPath()))
			return FALSE;
		if (FALSE === is_dir($this->getPath())) {
			throw new \OMV\Exception("The file '%s' is no directory.",
				$this->getPath());
		}
		return TRUE;
	}

	/**
	 * Create the mountpoint of the file system.
	 * @param mode Set the file mode (as in chmod), e.g. '0755'.
	 *   Defaults to '0700'.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function create($mode = "0700") {
		// Exit immediately if the directory already exists.
		if (TRUE === $this->exists())
			return;
		$cmdArgs = [];
		$cmdArgs[] = "--parents";
		$cmdArgs[] = sprintf("--mode=%s", $mode);
		$cmdArgs[] = escapeshellarg($this->getPath());
		$cmd = new \OMV\System\Process("mkdir", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Unlink the mountpoint of the file system.
	 * @param force Set to TRUE to ignore nonexistent files and remove all
	 *   files recursively. Defaults to FALSE.
	 * @param quiet Set to TRUE to do not throw an exception in case of an
	 *   error, e.g. the directory is not empty. Defaults to FALSE.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function unlink($force = FALSE, $quiet = FALSE) {
		// Exit immediately if the directory doesn't exists.
		if (FALSE === $this->exists())
			return;
		$cmdArgs = [];
		if (TRUE === $force) {
			$cmdArgs[] = "rm";
			$cmdArgs[] = "--force";
			$cmdArgs[] = "--recursive";
		} else {
			$cmdArgs[] = "rmdir";
		}
		$cmdArgs[] = escapeshellarg($this->getPath());
		$cmd = new \OMV\System\Process($cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->setQuiet($quiet);
		$cmd->execute();
	}

	/**
	 * See if the directory is a mountpoint.
	 * @return TRUE if the directory is a mountpoint, FALSE if not.
	 * @throw \OMV\ExecException
	 */
	public function isMountPoint() {
		$cmdArgs = [];
		$cmdArgs[] = "-q";
		$cmdArgs[] = escapeshellarg($this->getPath());
		$cmd = new \OMV\System\Process("mountpoint", $cmdArgs);
		$cmd->setQuiet(TRUE);
		$cmd->execute($output, $exitStatus);
		return ($exitStatus == 0) ? TRUE : FALSE;
		/*
		if (!is_dir($this->getPath()))
			return FALSE;
		if (FALSE === ($stat = stat($this->getPath())))
			return FALSE;
		if (FALSE === ($stat2 = stat(sprintf("%s/..", $this->getPath()))))
			return FALSE;
		return (($stat.dev != $stat2.dev) || (($stat.dev == $stat2.dev) &&
			($stat.ino == $stat2.ino))) ? TRUE : FALSE;
		*/
	}

	/**
	 * Check if the associated mountpoint is mounted.
	 * @return TRUE if the mountpoint is mounted, otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function isMounted() {
		// If the directory is a mountpoint, then we can assume that the
		// associated filesystem is mounted.
		return $this->isMountPoint();
	}

	/**
	 * Mount the this mountpoint.
	 * @param options An array or comma separated string of additional mount
	 *   options. Defaults to "".
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function mount($options = "") {
		$cmdArgs = [];
		$cmdArgs[] = "-v";
		if (!empty($options)) {
			if (!is_array($options))
				$options = array($options);
			$cmdArgs[] = sprintf("-o %s", implode(",", $options));
		}
		$cmdArgs[] = sprintf("--target %s", escapeshellarg($this->getPath()));
		$cmd = new \OMV\System\Process("mount", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Unmount this mount point.
	 * @param force Set to TRUE to force unmount. Defaults to FALSE.
	 * @param lazy Set to TRUE to lazy unmount. Defaults to FALSE.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function umount($force = FALSE, $lazy = FALSE) {
		$cmdArgs = [];
		$cmdArgs[] = "-v";
		if (TRUE === $force)
			$cmdArgs[] = "-f";
		if (TRUE === $lazy)
			$cmdArgs[] = "-l";
		$cmdArgs[] = escapeshellarg($this->getPath());
		$cmd = new \OMV\System\Process("umount", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Get the device file that is mounted on the given directory.
	 * Note, the device MUST be mounted.
	 * @return The device file of that is mounted on the given
	 *   directory, e.g. '/dev/vda1'.
	 */
	public function getDeviceFile() {
		$info = $this->getInfo();
		return $info['source'];
	}

	/**
	 * Get various information about the mount point, e.g. 'target',
	 * 'source', 'fstype', 'label', 'uuid', 'size', 'avail', 'used',
	 * 'maj:min' or 'options'.
	 * @return Returns an object containing the filesystem information.
	 */
	public function getInfo() {
		$cmdArgs = [];
		$cmdArgs[] = "--output-all";
		$cmdArgs[] = "--nofsroot";
		$cmdArgs[] = "--json";
		// We do not want to process 'autofs' entries for filesystems
		// that are mounted by systemd via 'x-systemd.automount'.
		// {
		//   "filesystems": [
		//     {
		//       "target": "/srv/dev-disk-by-id-dm-name-sdb-crypt",
		//       "source": "systemd-1",
		//       "fstype": "autofs",
		//       "options": "rw,relatime,fd=44,pgrp=1,timeout=0,minproto=5,maxproto=5,direct,pipe_ino=12136"
		//     },
		//     {
		//       "target": "/srv/dev-disk-by-id-dm-name-sdb-crypt",
		//       "source": "/dev/mapper/sdb-crypt",
		//       "fstype": "ext4",
		//       "options": "rw,relatime,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group"
		//     }
		//   ]
		// }
		$cmdArgs[] = "--types=noautofs";
		$cmdArgs[] = escapeshellarg($this->getPath());
		$cmd = new \OMV\System\Process("findmnt", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$output = json_decode_safe(implode("", $output), TRUE);
		// {
		//   "filesystems": [{
		//     "source":"/dev/sda",
		//     "target":"/srv/dev-disk-by-label-alfa",
		//     "fstype":"btrfs",
		//     "options":"rw,relatime,space_cache,subvolid=257,subvol=/data",
		//     "vfs-options":"rw,relatime",
		//     "fs-options":"rw,space_cache,subvolid=257,subvol=/data",
		//     "label":"alfa",
		//     "uuid":"343dab1d-edb7-4d89-813a-b4ad57c51f32",
		//     "partlabel":null,
		//     "partuuid":null,
		//     "maj:min":"0:49",
		//     "size":"7.3T",
		//     "avail":"7.2T",
		//     "used":"34.4G",
		//     "use%":"0%",
		//     "fsroot":"/data",
		//     "tid":27897,
		//     "id":225,
		//     "opt-fields":"shared:123",
		//     "propagation":"shared",
		//     "freq":null,
		//     "passno":null
		//   }]
		// }
		return $output['filesystems'][0];
	}

	/**
	 * Get the mountpoint path where the file system should be mounted to.
	 * Note, this path is OMV specific: /srv/<FSUUID>.
	 * @param id string The device file or UUID of the file system, e.g.
	 *   <ul>
	 *   \li 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 *   \li 7A48-BA97 (FAT)
	 *   \li 2ED43920D438EC29 (NTFS)
	 *   \li /dev/sda2
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-id/wwn-0x5000cca211cc703c-part1
	 *   \li /dev/disk/by-id/md-name-vmpc01:data
	 *   \li /dev/disk/by-id/md-uuid-75de9de9:6beca92e:8442575c:73eabbc9
	 *   \li /dev/disk/by-label/xx\x20yy
	 *   </ul>
	 * @return The path where to mount the file system, e.g.
	 *   /srv/85732966-949a-4d8b-87d7-d7e6681f787e or
	 *   /srv/dev-disk-by-id-wwn-0x5000cca211cc703c-part1 or
	 *   /srv/dev-disk-by-id-md-name-vmpc01-data.
	 */
	public static function buildPath($id) {
		if (TRUE === is_devicefile($id)) {
			$id = unescape_path($id);
			$id = str_replace(["/", ":"], "-", trim($id, "/"));
		}
		return build_path(DIRECTORY_SEPARATOR, \OMV\Environment::get(
			"OMV_MOUNT_DIR"), $id);
	}

	/**
	 * Create a new instance by device file or UUID.
	 * @param id string The device file or UUID of the file system, e.g.
	 *   <ul>
	 *   \li 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 *   \li 7A48-BA97 (FAT)
	 *   \li 2ED43920D438EC29 (NTFS)
	 *   \li /dev/sda2
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-id/wwn-0x5000cca211cc703c-part1
	 *   \li /dev/disk/by-id/md-name-vmpc01:data
	 *   \li /dev/disk/by-id/md-uuid-75de9de9:6beca92e:8442575c:73eabbc9
	 *   \li /dev/disk/by-label/xx\x20yy
	 *   </ul>
	 */
	public static function fromId($id) {
		$dir = \OMV\System\MountPoint::buildPath($id);
		return new \OMV\System\MountPoint($dir);
	}
}
