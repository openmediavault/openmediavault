<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2017 Volker Theile
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
	private $dir;

	/**
	 * Constructor
	 * @param dir Path to the directory, e.g.
	 *   <ul>
	 *   \li /media/78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 *   \li /media/7A48-BA97 (FAT)
	 *   \li /media/2ED43920D438EC29 (NTFS)
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
	 *   \li /media/78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 *   \li /media/7A48-BA97 (FAT)
	 *   \li /media/2ED43920D438EC29 (NTFS)
	 *   </ul>
	 * @return None.
	 */
	public function setPath($dir) {
		$this->dir = $dir;
	}

	/**
	 * Create the mount point of the file system.
	 * @param mode Set the file mode (as in chmod), e.g. '0755'.
	 *   Defaults to '0700'.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function create($mode = "0700") {
		// Exit immediatelly if the directory already exists.
		if (TRUE === is_dir($this->getPath()))
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
	 * Unlink the mount point of the file system.
	 * @param force Set to TRUE to ignore nonexistent files. Defaults to TRUE.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function unlink($force = TRUE) {
		// Exit immediatelly if the directory already exists.
		if (FALSE === is_dir($this->getPath()))
			return;
		$cmdArgs = [];
		$cmdArgs[] = "--recursive";
		$cmdArgs[] = "--force";
		$cmdArgs[] = escapeshellarg($this->getPath());
		$cmd = new \OMV\System\Process("rm", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * See if the directory is a mount point.
	 * @return TRUE if the directory is a mount point, FALSE if not.
	 */
	public function isMountPoint() {
		$cmdArgs = [];
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
	 * Get the mount point path where the filesystem should be mounted to.
	 * Note, this path is OMV specific: /media/<FSUUID>.
	 * @param uuid The UUID of the filesystem, e.g.
	 *   <ul>
	 *   \li 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 *   \li 7A48-BA97 (FAT)
	 *   \li 2ED43920D438EC29 (NTFS)
	 *   </ul>
	 * @return The path where to mount the filesystem, e.g.
	 *   /media/85732966-949a-4d8b-87d7-d7e6681f787e.
	 */
	public static function buildPath($uuid) {
		return build_path(DIRECTORY_SEPARATOR, \OMV\Environment::get(
		  "OMV_MOUNT_DIR"), $uuid);
	}
}
