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
 * A class that implements a mount point.
 * @ingroup api
 */
class MountPoint {
	private $device = "";
	private $dir = "";

	/**
	 * Constructor
	 * @param device The block special device or remote file system,
	 *   e.g. /dev/sde1, /dev/disk/by-uuid/2218-DF1C or rootfs.
	 * @param dir The mount point for the file system, e.g. /media/2218-DF1C.
	 */
	public function __construct($device, $dir) {
		$this->device = $device;
		$this->dir = $dir;
	}

	/**
	 * Get the block special device or remote file system, e.g. /dev/sde1,
	 * /dev/disk/by-uuid/2218-DF1C or rootfs.
	 * @return The block special device or remote file system.
	 */
	public function getDevice() {
		return $this->device;
	}

	/**
	 * Get the mount point of the filesystem, e.g. /mnt/xyz.
	 * @return The mount point of the filesystem.
	 */
	public function getMountPoint() {
		return $this->dir;
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
		$cmdArgs[] = escapeshellarg($this->dir);
		$cmd = new \OMV\System\Process("mount", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
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
		$cmdArgs[] = escapeshellarg($this->dir);
		$cmd = new \OMV\System\Process("umount", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
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
		if (is_dir($this->dir))
			return;
		$cmdArgs = [];
		$cmdArgs[] = "--parents";
		$cmdArgs[] = sprintf("--mode=%s", $mode);
		$cmdArgs[] = escapeshellarg($this->dir);
		$cmd = new \OMV\System\Process("mkdir", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute();
	}

	/**
	 * Unlink the mount point of the file system.
	 * @param force Set to TRUE to ignore nonexistent files. Defaults to TRUE.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function unlinkDir($force = TRUE) {
		$cmdArgs = [];
		$cmdArgs[] = "--recursive";
		$cmdArgs[] = "--force";
		$cmdArgs[] = escapeshellarg($this->dir);
		$cmd = new \OMV\System\Process("rm", $cmdArgs);
		$cmd->setRedirect2to1(TRUE);
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
			if (($matches[0] == $this->device) || ($matches[1] == $this->dir)) {
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
		$cmdArgs = [];
		$cmdArgs[] = escapeshellarg($this->dir);
		$cmd = new \OMV\System\Process("mountpoint", $cmdArgs);
		$cmd->setQuiet(TRUE);
		$cmd->execute($output, $exitStatus);
		return ($exitStatus == 0) ? TRUE : FALSE;
	}
}
