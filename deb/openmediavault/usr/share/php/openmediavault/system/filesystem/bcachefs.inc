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

class Bcachefs extends Filesystem {
	protected $numTotalDeviceFiles = 0;
	protected $deviceFiles = [];

	/**
	 * Get the filesystem detailed information.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	protected function getData() {
		if (FALSE !== $this->isCached())
			return;

		parent::getData();

		// We need to have the UUID of the file system.
		if (!$this->hasUuid())
			return;

		// Reset flag to mark information has not been successfully read.
		$this->setCached(FALSE);

		// Get the file system information.
		$cmdArgs = [];
		$cmdArgs[] = "fs";
		$cmdArgs[] = "usage";
		$cmdArgs[] = "--by-uuid";
		$cmdArgs[] = escapeshellarg($this->uuid);
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);

		// Parse command output for devices
		$regex = "/Dev\s+([\/\w]+):/i";
		foreach ($output as $line) {
			if (preg_match($regex, $line, $matches)) {
				$this->deviceFiles[] = $matches[1];
				$this->numTotalDeviceFiles++;
			}
		}

		// Set flag to mark information has been successfully read.
		$this->setCached(TRUE);
	}

	/**
	 * See parent class definition.
	 */
	public function refresh() {
		parent::refresh();
		$this->setCached(FALSE);
		$this->getData();
	}

	/**
	 * See parent class definition.
	 */
	public function getDeviceFiles() {
		$this->getData();
		// Filter missing devices.
		$deviceFiles = array_filter(array_values($this->deviceFiles),
		  function($value) {
			  return !empty($value);
		  });
		// Sort the devices using a "natural order" algorithm.
		sort($deviceFiles, SORT_NATURAL);
		return $deviceFiles;
	}

	/**
	 * Get the list of all used device files.
	 * This is bcachefs specific.
	 * @return An array containing the used device files,
	 *   otherwise FALSE.
	 */
	public function getDeviceFilesAssoc() {
		$this->getData();
		return $this->deviceFiles;
	}

	/**
	 * Get the total number of used device files.
	 * @return The total number of device files, otherwise FALSE.
	 */
	public function getNumTotalDeviceFiles() {
		$this->getData();
		return $this->numTotalDeviceFiles;
	}

	/**
	 * See parent class definition.
	 */
	public function getStatistics() {
		if (FALSE === ($stats = parent::getStatistics()))
			return FALSE;

		$cmdArgs = [];
		$cmdArgs[] = "fs";
		$cmdArgs[] = "usage";
		$cmdArgs[] = escapeshellarg($this->getMountPoint());
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);

		// Parse output to extract filesystem stats
		$size = 0;
		$used = 0;
		$available = 0;

		// Parse bcachefs fs usage output for total, used, and remaining space
		foreach ($output as $line) {
			if (preg_match("/Total:\s+(\d+)\/(\d+)\s+(\d+\.\d+)%/i", $line, $matches)) {
				$used = $matches[1];
				$size = $matches[2];
				$available = $size - $used;
				break;
			}
		}

		// Update the information.
		return array_merge($stats, [
			"size" => $size,
			"blocks" => bcdiv($size, "1024", 0),
			"used" => $used,
			"available" => $available,
			"percentage" => intval(($used / $size) * 100)
		]);
	}

	/**
	 * See parent class definition.
	 */
	public function getDetails() {
		$result = "";

		// Get fs usage details
		$cmdArgs = [];
		$cmdArgs[] = "fs";
		$cmdArgs[] = "usage";
		$cmdArgs[] = "--verbose";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$result = implode("\n", $output);

		// Get superblock info
		$output = [];
		$cmdArgs = [];
		$cmdArgs[] = "show-super";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$result = $result . "\n\n" . implode("\n", $output);

		// List filesystem replicas if it's a multi-device system
		if ($this->hasMultipleDevices()) {
			$output = [];
			$cmdArgs = [];
			$cmdArgs[] = "fs";
			$cmdArgs[] = "usage";
			$cmdArgs[] = "--replicas";
			$cmdArgs[] = escapeshellarg($this->getMountPoint());
			$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			$result = $result . "\n\n" . implode("\n", $output);
		}

		return $result;
	}

	/**
	 * Remove the filesystem.
	 * @return void
	 */
	public function remove() {
		parent::remove();
	}

	/**
	 * Unmount the file system.
	 * @param force Set to TRUE to force unmount. Defaults to FALSE.
	 * @param lazy Set to TRUE to lazy unmount. Defaults to FALSE.
	 * @param directory Set to TRUE to unmount the file system using
	 * the directory where it has been mounted, otherwise the device
	 * file is used. Defaults to FALSE.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function umount($force = FALSE, $lazy = FALSE, $directory = FALSE) {
		// For bcachefs, it's safer to use the mount point (directory)
		parent::umount($force, $lazy, TRUE);
	}

	/**
	 * Grow the filesystem.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	final public function grow() {
		$this->assertIsMounted();
		$cmdArgs = [];
		$cmdArgs[] = "device";
		$cmdArgs[] = "resize";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Add a device to the filesystem.
	 * @param string $deviceFile The device to add.
	 * @return void
	 */
	public function addDevice($deviceFile) {
		$this->assertIsMounted();
		$cmdArgs = [];
		$cmdArgs[] = "device";
		$cmdArgs[] = "add";
		$cmdArgs[] = escapeshellarg($this->getMountPoint());
		$cmdArgs[] = escapeshellarg($deviceFile);
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Remove a device from the filesystem.
	 * @param string $deviceFile The device to remove.
	 * @param bool $force Set to TRUE to force removal. Defaults to FALSE.
	 * @return void
	 */
	public function removeDevice($deviceFile, $force = FALSE) {
		$this->assertIsMounted();
		$cmdArgs = [];
		$cmdArgs[] = "device";
		$cmdArgs[] = "remove";
		if ($force) {
			$cmdArgs[] = "--force";
		}
		$cmdArgs[] = escapeshellarg($this->getMountPoint());
		$cmdArgs[] = escapeshellarg($deviceFile);
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Evacuate a device to prepare for removal.
	 * @param string $deviceFile The device to evacuate.
	 * @return void
	 */
	public function evacuateDevice($deviceFile) {
		$this->assertIsMounted();
		$cmdArgs = [];
		$cmdArgs[] = "device";
		$cmdArgs[] = "evacuate";
		$cmdArgs[] = escapeshellarg($this->getMountPoint());
		$cmdArgs[] = escapeshellarg($deviceFile);
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Set device state (rw, ro, failed, spare).
	 * @param string $deviceFile The device to set state for.
	 * @param string $state The state to set (rw|ro|failed|spare).
	 * @return void
	 */
	public function setDeviceState($deviceFile, $state) {
		$this->assertIsMounted();
		if (!in_array($state, ['rw', 'ro', 'failed', 'spare'])) {
			throw new \InvalidArgumentException("Invalid device state: $state");
		}
		$cmdArgs = [];
		$cmdArgs[] = "device";
		$cmdArgs[] = "set-state";
		$cmdArgs[] = escapeshellarg($state);
		$cmdArgs[] = escapeshellarg($this->getMountPoint());
		$cmdArgs[] = escapeshellarg($deviceFile);
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Online a device in a mounted filesystem.
	 * @param string $deviceFile The device to online.
	 * @return void
	 */
	public function onlineDevice($deviceFile) {
		$this->assertIsMounted();
		$cmdArgs = [];
		$cmdArgs[] = "device";
		$cmdArgs[] = "online";
		$cmdArgs[] = escapeshellarg($this->getMountPoint());
		$cmdArgs[] = escapeshellarg($deviceFile);
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Offline a device without removing it.
	 * @param string $deviceFile The device to offline.
	 * @param bool $force Set to TRUE to force offlining. Defaults to FALSE.
	 * @return void
	 */
	public function offlineDevice($deviceFile, $force = FALSE) {
		$this->assertIsMounted();
		$cmdArgs = [];
		$cmdArgs[] = "device";
		$cmdArgs[] = "offline";
		if ($force) {
			$cmdArgs[] = "--force";
		}
		$cmdArgs[] = escapeshellarg($this->getMountPoint());
		$cmdArgs[] = escapeshellarg($deviceFile);
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Check if the filesystem has multiple devices.
	 * @return bool Returns TRUE if the filesystem has multiple devices,
	 *   otherwise FALSE.
	 */
	public function hasMultipleDevices() {
		$this->getData();
		return $this->numTotalDeviceFiles > 1;
	}

	/**
	 * Data rereplicate - scan for extents with insufficient replicas.
	 * @return void
	 */
	public function dataRereplicate() {
		$this->assertIsMounted();
		$cmdArgs = [];
		$cmdArgs[] = "data";
		$cmdArgs[] = "rereplicate";
		$cmdArgs[] = escapeshellarg($this->getMountPoint());
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Unlock an encrypted bcachefs filesystem.
	 * @param string $passphrase The encryption passphrase.
	 * @return void
	 */
	public static function unlock($deviceFile, $passphrase) {
		// Implement passphrase handling, likely using expect or similar
		$cmdArgs = [];
		$cmdArgs[] = "unlock";
		$cmdArgs[] = escapeshellarg($deviceFile);
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		// Here you'd need to handle sending the passphrase to the command
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Resize journal on a device.
	 * @param string $deviceFile The device to resize journal on.
	 * @param int $size New journal size in bytes.
	 * @return void
	 */
	public function resizeJournal($deviceFile, $size) {
		$cmdArgs = [];
		$cmdArgs[] = "device";
		$cmdArgs[] = "resize-journal";
		$cmdArgs[] = escapeshellarg($deviceFile);
		$cmdArgs[] = escapeshellarg($size);
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Check if subvolumes are supported.
	 * @return Returns TRUE if subvolumes are supported,
	 *   otherwise FALSE.
	 */
	public static function hasSubvolSupport() {
		return TRUE;
	}

	/**
	 * Check if the specified path is a subvolume.
	 * @param string $path The path to check.
	 * @return Returns TRUE if the specified path is a subvolume,
	 *   otherwise FALSE.
	 */
	public static function isSubvolume($path) {
		$cmdArgs = [];
		$cmdArgs[] = "subvolume";
		$cmdArgs[] = "list";
		$cmdArgs[] = escapeshellarg(dirname($path));
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setQuiet(TRUE);
		$cmd->execute($output, $exitStatus);
		if ($exitStatus !== 0)
			return FALSE;

		$basename = basename($path);
		foreach ($output as $line) {
			if (strpos($line, $basename) !== FALSE) {
				return TRUE;
			}
		}
		return FALSE;
	}

	/**
	 * Create a subvolume.
	 * @param string $path The path of the subvolume.
	 */
	public static function createSubvolume($path) {
		$cmdArgs = [];
		$cmdArgs[] = "subvolume";
		$cmdArgs[] = "create";
		$cmdArgs[] = escapeshellarg($path);
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->execute();
	}

	/**
	 * Delete a subvolume.
	 * @param string $path The path of the subvolume.
	 */
	public static function deleteSubvolume($path) {
		$cmdArgs = [];
		$cmdArgs[] = "subvolume";
		$cmdArgs[] = "destroy";
		$cmdArgs[] = escapeshellarg($path);
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->execute();
	}

	/**
	 * Create a snapshot of the specified subvolume.
	 * @param string $path The path of the subvolume.
	 * @param string $target The path where the snapshot is stored.
	 * @param bool $readOnly Make the new snapshot readonly.
	 *   Defaults to FALSE.
	 * @return void
	 */
	public static function createSnapshot($path, $target, $readOnly = FALSE) {
		$cmdArgs = [];
		$cmdArgs[] = "subvolume";
		$cmdArgs[] = "snapshot";
		if (TRUE === $readOnly) {
			$cmdArgs[] = "--readonly";
		}
		$cmdArgs[] = escapeshellarg($path);
		$cmdArgs[] = escapeshellarg($target);
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->execute();
	}

	/**
	 * Run fsck on the filesystem.
	 * @param bool $fix Set to TRUE to fix errors. Defaults to FALSE.
	 * @param bool $verbose Set to TRUE for verbose output. Defaults to FALSE.
	 * @return void
	 */
	public function fsck($fix = FALSE, $verbose = FALSE) {
		$cmdArgs = [];
		$cmdArgs[] = "fsck";
		if ($fix) {
			$cmdArgs[] = "--fix";
		}
		if ($verbose) {
			$cmdArgs[] = "--verbose";
		}
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Run a scrub operation to check data integrity.
	 * @return void
	 */
	public function scrub() {
		$this->assertIsMounted();
		$cmdArgs = [];
		$cmdArgs[] = "data";
		$cmdArgs[] = "scrub";
		$cmdArgs[] = escapeshellarg($this->getMountPoint());
		$cmd = new \OMV\System\Process("bcachefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}
}
