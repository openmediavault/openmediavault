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

require_once("openmediavault/functions.inc");

/**
 * A class that implements a mount point.
 * @ingroup api
 */
class MountPoint {
	private $device = NULL;
	private $dir = NULL;

	/**
	 * Constructor
	 * @param device The name of the mounted file system, e.g. <ul>
	 *   \li /dev/sde1
	 *   \li /dev/disk/by-uuid/2218-DF1C
	 *   \li rootfs
	 *   \li efd877d8-3f79-11e6-84f9-0002b3a176b4
	 *   \li cc9097b9-8859-4455-ae82-a5dbf03d542a:1537599b-...-42678814a582
	 *   </ul>
	 * @param dir The file system path prefix, e.g. /media/2218-DF1C.
	 */
	public function __construct($device = NULL, $dir = NULL) {
		$this->device = $device;
		$this->dir = $dir;
	}

	/**
	 * Get the name of the mounted file system, e.g. <ul>
	 * \li /dev/sde1
	 * \li /dev/disk/by-uuid/2218-DF1C
	 * \li rootfs
	 * \li efd877d8-3f79-11e6-84f9-0002b3a176b4
	 * \li cc9097b9-8859-4455-ae82-a5dbf03d542a:1537599b-...-8202-42678814a582
	 * </ul>.
	 * @return The name of the mounted file system.
	 */
	public function getDevice() {
		return $this->device;
	}

	/**
	 * Set the name of the mounted file system.
	 * @return None.
	 */
		public function setDevice($device) {
		$this->device = $device;
	}

	public function hasDevice() {
		return !is_null($this->getDevice());
	}

	final public function assertDevice() {
		if (FALSE === $this->hasDevice()) {
			throw new \InvalidArgumentException("No device specified.");
		}
	}

	/**
	 * Get the file system path prefix, e.g. /mnt/xyz.
	 * @return The file system path prefix.
	 */
	public function getDir() {
		return $this->dir;
	}

	/**
	 * Set the file system path prefix, e.g. /mnt/xyz.
	 * @return None.
	 */
	public function setDir($dir) {
		$this->dir = $dir;
	}

	public function hasDir() {
		return !is_null($this->getDir());
	}

	final public function assertDir() {
		if (FALSE === $this->hasDir()) {
			throw new \InvalidArgumentException("No directory specified.");
		}
	}

	/**
	 * Mount the given file system.
	 * @param options Additional mount options. Defaults to "".
	 * @return None.
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
		if (TRUE === $this->hasDir()) {
			$cmdArgs[] = escapeshellarg($this->getDir());
		} else if (TRUE === $this->hasDevice()) {
			if (TRUE === is_uuid($this->getDevice()))
				$cmdArgs[] = sprintf("-U %s", $this->getDevice());
			else
				$cmdArgs[] = escapeshellarg($this->getDevice());
		} else {
			throw new \InvalidArgumentException(
			  "No device or directory specified.");
		}
		$cmd = new \OMV\System\Process("mount", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Unmount the given file system.
	 * @param force Set to TRUE to force unmount. Defaults to FALSE.
	 * @param lazy Set to TRUE to lazy unmount. Defaults to FALSE.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function umount($force = FALSE, $lazy = FALSE) {
		$cmdArgs = [];
		$cmdArgs[] = "-v";
		if (TRUE === $force)
			$cmdArgs[] = "-f";
		if (TRUE === $lazy)
			$cmdArgs[] = "-l";
		if (TRUE === $this->hasDir()) {
			$cmdArgs[] = escapeshellarg($this->getDir());
		} else if (TRUE === $this->hasDevice()) {
			if (TRUE === is_uuid($this->getDevice()))
				$cmdArgs[] = sprintf("-U %s", $this->getDevice());
			else
				$cmdArgs[] = escapeshellarg($this->getDevice());
		} else {
			throw new \InvalidArgumentException(
			  "No device or directory specified.");
		}
		$cmd = new \OMV\System\Process("umount", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Create the mount point of the file system.
	 * @param mode Set the file mode (as in chmod), e.g. '0755'.
	 *   Defaults to '0700'.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function mkDir($mode = "0700") {
		$this->assertDir();
		// Exit immediatelly if the directory already exists.
		if (TRUE === is_dir($this->getDir()))
			return;
		$cmdArgs = [];
		$cmdArgs[] = "--parents";
		$cmdArgs[] = sprintf("--mode=%s", $mode);
		$cmdArgs[] = escapeshellarg($this->getDir());
		$cmd = new \OMV\System\Process("mkdir", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Unlink the mount point of the file system.
	 * @param force Set to TRUE to ignore nonexistent files. Defaults to TRUE.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function unlinkDir($force = TRUE) {
		$this->assertDir();
		// Exit immediatelly if the directory already exists.
		if (FALSE === is_dir($this->getDir()))
			return;
		$cmdArgs = [];
		$cmdArgs[] = "--recursive";
		$cmdArgs[] = "--force";
		$cmdArgs[] = escapeshellarg($this->getDir());
		$cmd = new \OMV\System\Process("rm", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Check if the given file system is mounted.
	 * @return TRUE if the file system is mounted, otherwise FALSE.
	 */
	public function isMounted() {
		$result = FALSE;
		foreach (file("/proc/mounts") as $rowk => $rowv) {
			// Parse command output:
			// rootfs / rootfs rw 0 0
			// none /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0
			// none /proc proc rw,nosuid,nodev,noexec,relatime 0 0
			// tmpfs /dev/shm tmpfs rw,nosuid,nodev,relatime 0 0
			// /dev/sde1 /home/ftp/data ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			// /dev/sde1 /media/46607e80-dc8e-427e-9939-7909985dfe5b ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			$matches = explode(" ", $rowv);
			if (($matches[0] == $this->getDevice()) ||
			  ($matches[1] == $this->getDir())) {
				$result = TRUE;
				break;
			}
		}
		return $result;
	}

	/**
	 * See if the directory is a mount point.
	 * @return TRUE if the directory is a mount point, FALSE if not.
	 */
	public function isMountPoint() {
		$this->assertDir();
		$cmdArgs = [];
		$cmdArgs[] = escapeshellarg($this->getDir());
		$cmd = new \OMV\System\Process("mountpoint", $cmdArgs);
		$cmd->setQuiet(TRUE);
		$cmd->execute($output, $exitStatus);
		return ($exitStatus == 0) ? TRUE : FALSE;
	}
}
