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

class Btrfs extends Filesystem {
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
		$cmdArgs[] = "fi";
		$cmdArgs[] = "show";
		$cmdArgs[] = escapeshellarg($this->uuid);
		$cmd = new \OMV\System\Process("btrfs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);

		// Parse command output:
		// warning devid 3 not found already
		// Label: none  uuid: b304658e-0e41-45a9-bf54-c4939e151819
		//        Total devices 3 FS bytes used 896.00KiB
		//        devid    1 size 10.00GiB used 2.03GiB path /dev/sdb
		//        devid    2 size 10.00GiB used 2.01GiB path /dev/sdc
		//        *** Some devices missing
		$regex = "/Total devices (\d+)/";
		preg_match($regex, implode("\n", $output), $matches);
		$this->numTotalDeviceFiles = intval($matches[1]);
		$this->deviceFiles = array_fill(1, $this->numTotalDeviceFiles, "");
		$regex = "/^\s+devid\s+(\d+)\s+size\s+\S+\s+used\s+\S+\s+path\s+(\S+)$/";
		foreach (preg_filter($regex, "$1=$2", $output) as $rowk => $rowv) {
			list($devId, $deviceFile) = explode('=', $rowv);
			$this->deviceFiles[$devId] = $deviceFile;
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
	 * Get the list of all used device files. Note, the array index is the
	 * device ID, thus the array starts with index 1.
	 * This is BTRFS specific.
	 * @return An associative array containing the used device files,
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
	 *
	 * For a btrfs filesystem, \em size is the raw size of the
	 * filesystem (i.e. sum of all device sizes), \em used is the size of
	 * data stored after accounting for DUP/RAID level, and \em available
	 * is an estimate of remaining free space based on current data ratio.
	 * e.g. Consider 25 GiB of data stored in a RAID1 filesystem made up of
	 * two 100 GiB devices:
	 *     Size:         200 GiB (total size of filesystem, 2x100 GiB)
	 *     Used:         25 GiB  (data ratio: 2.0, so actually occupies 50 GiB)
	 *     Available:    75 GiB  (raw space remaining: 150 GiB, but based on
	 *                         current ratio will fit only 75 GiB of data)
	 */
	public function getStatistics() {
		if (FALSE === ($stats = parent::getStatistics()))
			return FALSE;

		$cmdArgs = [];
		$cmdArgs[] = "fi";
		$cmdArgs[] = "df";
		$cmdArgs[] = "-b";
		$cmdArgs[] = escapeshellarg($this->getMountPoint());
		$cmd = new \OMV\System\Process("btrfs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);

		// Initialize arrays.
		$allocated = $allocatedUnused = $used = $allocatedRaw =
		  $allocatedUnusedRaw = $usedRaw = [
			"Data" => 0,
			"Metadata" => 0,
			"Data+Metadata"	=> 0,
			"System" => 0,
			"GlobalReserve"	=> 0
		];

		// Parse command output.
		// Example 1:
		// System, RAID1: total=8388608, used=4096
		// System, single: total=4194304, used=0
		// Data+Metadata, RAID1: total=67108864, used=28672
		// Data+Metadata, single: total=8388608, used=0
		// GlobalReserve, single: total=4194304, used=0
		//
		// Example 2:
		// Data, single: total=712704000, used=712704000
		// System, DUP: total=8388608, used=16384
		// Metadata, DUP: total=171048960, used=1458176
		// GlobalReserve, single: total=3407872, used=0
		$regex = "/^([\w\+]+), ([a-zA-Z0156]+): total=([0-9]+), ".
		  "used=([0-9]+)$/i";
		foreach (preg_filter($regex, "$1 $2 $3 $4", $output) as $rowv) {
			list($type, $profile, $t, $u) = explode(" ", $rowv);
			switch (mb_strtoupper($profile)) {
				case "SINGLE":
				case "RAID0":
					$factor = 1.0;
					break;
				case "DUP":
				case "RAID1":
				case "RAID10":
					$factor = 2.0;
					break;
				case "RAID5":
					$numDevices = $this->getNumTotalDeviceFiles();
					$efficiency = 1 - (1 / $numDevices);
					$factor = 1 / $efficiency;
					break;
				case "RAID6":
					$numDevices = $this->getNumTotalDeviceFiles();
					$efficiency = 1 - (2 / $numDevices);
					$factor = 1 / $efficiency;
					break;
			}

			// Allocated extents size.
			$allocated[$type] = bcadd($allocated[$type], $t);
			// Allocated but unused extents size.
			$allocatedUnused[$type] = bcadd($allocatedUnused[$type],
			  bcsub($t, $u, 0));
			// Actual data size.
			$used[$type] = bcadd($used[$type], $u);

			// Raw disk used by allocated extents.
			$allocatedRaw[$type] = bcadd($allocatedRaw[$type],
			  bcmul($t, strval($factor), 0));
			// Raw disk used by allocated but unused extents.
			$allocatedUnusedRaw[$type] = bcadd($allocatedUnusedRaw[$type],
			  bcmul(bcsub($t, $u, 0), strval($factor), 0));
			// Raw disk data usage, i.e. including redundancy.
			$usedRaw[$type] = bcadd($usedRaw[$type], bcmul($u,
			  strval($factor), 0));
		}

		unset($cmd, $output);

		$cmdArgs = [];
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("btrfs inspect-internal dump-super", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);

		// Parse command output.
		// Example 1:
		// superblock: bytenr=65536, device=/dev/sdb
		// ---------------------------------------------------------
		// csum			0x2013cfbe [match]
		// bytenr			65536
		// flags			0x1
		// ...
		// total_bytes		213909504
		// bytes_used		32768
		// sectorsize		4096
		// nodesize		4096
		// leafsize		4096
		// ...
		// num_devices		2
		// ...
		//
		// Example 2:
		// superblock: bytenr=65536, device=/dev/sdb1
        // ---------------------------------------------------------
        // csum_type		0 (crc32c)
        // csum_size		4
        // csum			0xc03b8452 [match]
        // bytenr			65536
		// ...
        // generation		266
        // root			30998528
        // sys_array_size		129
        // chunk_root_generation	73
        // root_level		0
        // chunk_root		22167552
        // chunk_root_level	0
        // ...
        // total_bytes		1072672768
        // bytes_used		714178560
        // sectorsize		4096
        // nodesize		16384
        // leafsize (deprecated)	16384
        // stripesize		4096
        // root_dir		6
        // num_devices		1
        // ...
		$regex = "/total_bytes\s+(\d+)/";
		preg_match($regex, implode("\n", $output), $matches);
		$totalBytes = $matches[1];	// Raw fs size

		// Unallocated raw disk space.
		$unallocated = bcsub($totalBytes, array_sum($allocatedRaw));

		// Available space is calculated as all unallocated space,
		// plus allocated but unused space in Data or Data+Metadata.
		// Allocated but unused space in Metadata alone (or System
		// or GlobalReserve, although they are small) is not included
		// as it is not directly available for data usage.
		$available = bcadd(
			$unallocated,
			array_sum(array_filter($allocatedUnusedRaw, function($type) {
				return in_array($type, [ 'Data', 'Data+Metadata' ]);
			}, ARRAY_FILTER_USE_KEY)));
		// Ensure available space is not negative.
		if (1 == bccomp("0", $available)) {
			$available = "0";
		}

		// Data ratio is calculated based only on Data (or Data+Metadata).
		$dataRatio = "1.00";
		$usedDataSum = array_sum(array_filter($used, function($type) {
				return in_array($type, [ 'Data', 'Data+Metadata' ]);
			}, ARRAY_FILTER_USE_KEY));
		if (0 < $usedDataSum) {
			$dataRatio = bcdiv(
				array_sum(array_filter($usedRaw, function($type) {
					return in_array($type, [ 'Data', 'Data+Metadata' ]);
				}, ARRAY_FILTER_USE_KEY)),
				$usedDataSum,
				2);
		}

		// Calculate the correct device size.
		// https://github.com/digint/btrbk/commit/b69e9ebf349c5430fbfa8d1b31a80b254eda0441#diff-82762b70e27ed444b151409dbf03955775fe33e3231cc0ec8786062522b8b2aaR786
		$totalBytesRatio = bcdiv($totalBytes, $dataRatio, 0);

		// Update the information.
		// Raw disk(s) size.
		return array_merge($stats, [
			"size" => $totalBytesRatio,
			"blocks" => bcdiv($totalBytesRatio, "1024", 0),
			// Usage by all profiles (excluding redundancy).
			"used" => array_sum($used),
			// Space available for Data, adjusted for average data ratio.
			"available" => bcdiv($available, $dataRatio, 0),
			// Raw disk usage, all profiles, divided by raw size.
			"percentage" => intval(bcmul(
				bcdiv(array_sum($usedRaw), $totalBytes, 3), 100, 0)),
			"dataratio" => $dataRatio
		]);
	}

	/**
	 * See parent class definition.
	 */
	public function getDetails() {
		$result = "";

		$cmdArgs = [];
		$cmdArgs[] = "filesystem";
		$cmdArgs[] = "show";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("btrfs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$result = implode("\n", $output);

		$output = [];
		$cmdArgs = [];
		$cmdArgs[] = "filesystem";
		$cmdArgs[] = "df";
		$cmdArgs[] = escapeshellarg($this->getMountPoint());
		$cmd = new \OMV\System\Process("btrfs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$result = $result . "\n" . implode("\n", $output);

		$output = [];
		$cmdArgs = [];
		$cmdArgs[] = "device";
		$cmdArgs[] = "stats";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("btrfs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$result = $result . "\n\n" . implode("\n", $output);

		return $result;
	}

	/**
	 * Remove the filesystem.
	 * @return void
	 */
	public function remove() {
		$this->getData();
		if ($this->hasMultipleDevices() === TRUE) {
			while ($this->getDeviceFile()) {
				parent::remove();
				sleep(1);
				$this->refresh();
			}
		} else {
			parent::remove();
		}
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
		// Sometimes the canonical device file of a BTRFS file system
		// is not the same as the one that is listed in the kernel table
		// of mounted file systems.
		//
		// Example:
		// # blkid
		// ...
		// /dev/sdb: UUID="bc3fd444-4d06-48ef-8bdf-a86a35b0b491" UUID_SUB="ebbcd1e6-be6d-42cd-9511-5b7ae5ba46b1" BLOCK_SIZE="4096" TYPE="btrfs"
		// /dev/sda: UUID="bc3fd444-4d06-48ef-8bdf-a86a35b0b491" UUID_SUB="acf73633-7c1a-4962-abf0-5749013dadbd" BLOCK_SIZE="4096" TYPE="btrfs"
		// /dev/sdc: UUID="bc3fd444-4d06-48ef-8bdf-a86a35b0b491" UUID_SUB="9c95f736-93db-4b96-8be3-6058c46a5493" BLOCK_SIZE="4096" TYPE="btrfs"
		// # ls -alh /dev/disk/by-id/scsi-0QEMU_QEMU_HARDDISK_drive-scsi0-0-1
		// lrwxrwxrwx 1 root root 9 Oct  6 20:26 /dev/disk/by-id/scsi-0QEMU_QEMU_HARDDISK_drive-scsi0-0-1 -> ../../sdc
		// # readlink --canonicalize /dev/disk/by-id/scsi-0QEMU_QEMU_HARDDISK_drive-scsi0-0-1
		// /dev/sdc
		// # findfs UUID=bc3fd444-4d06-48ef-8bdf-a86a35b0b491
		// /dev/sdc
		// # udevadm info --query=property --name=/dev/sdc
		// ...
		// ID_PATH=pci-0000:00:03.0-scsi-0:0:1:0
		// ID_PATH_TAG=pci-0000_00_03_0-scsi-0_0_1_0
		// ID_FS_UUID=bc3fd444-4d06-48ef-8bdf-a86a35b0b491
		// ID_FS_UUID_ENC=bc3fd444-4d06-48ef-8bdf-a86a35b0b491
		// ID_FS_UUID_SUB=9c95f736-93db-4b96-8be3-6058c46a5493
		// ID_FS_UUID_SUB_ENC=9c95f736-93db-4b96-8be3-6058c46a5493
		// ID_FS_TYPE=btrfs
		// ID_FS_USAGE=filesystem
		// ID_BTRFS_READY=1
		// DEVLINKS=/dev/disk/by-path/pci-0000:00:03.0-scsi-0:0:1:0 /dev/disk/by-uuid/bc3fd444-4d06-48ef-8bdf-a86a35b0b491 /dev/disk/by-id/scsi-0QEMU_QEMU_HARDDISK_drive-scsi0-0-1
		// # mount
		// ...
		// /dev/sda on /srv/dev-disk-by-id-scsi-0QEMU_QEMU_HARDDISK_drive-scsi0-0-1 type btrfs (rw,relatime,space_cache,subvolid=5,subvol=/)
		// # umount /dev/disk/by-id/scsi-0QEMU_QEMU_HARDDISK_drive-scsi0-0-1
		// umount: /dev/disk/by-id/scsi-0QEMU_QEMU_HARDDISK_drive-scsi0-0-1: not mounted.
		// # cat /etc/fstab
		// ...
		// /dev/disk/by-id/scsi-0QEMU_QEMU_HARDDISK_drive-scsi0-0-1		/srv/dev-disk-by-id-scsi-0QEMU_QEMU_HARDDISK_drive-scsi0-0-1	btrfs	defaults,nofail	0 2
		//
		// To workaround this problem we use the mount point to unmount
		// the file system.
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
		$cmdArgs[] = "filesystem";
		$cmdArgs[] = "resize";
		$cmdArgs[] = "max";
		$cmdArgs[] = escapeshellarg($this->getMountPoint());
		$cmd = new \OMV\System\Process("btrfs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
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
		$cmdArgs[] = "show";
		$cmdArgs[] = escapeshellarg($path);
		$cmd = new \OMV\System\Process("btrfs", $cmdArgs);
		$cmd->setQuiet(TRUE);
		$cmd->execute($output, $exitStatus);
		if ($exitStatus !== 0)
			return FALSE;
		return TRUE;
	}

	/**
	 * Get information about the specified subvolume.
	 * @param string $path The path of the subvolume.
	 * @return Returns an array containing the subvolume information,
	 *   otherwise FALSE.
	 */
	public static function getSubvolumeInfo($path) {
		$cmdArgs = [];
		$cmdArgs[] = "subvolume";
		$cmdArgs[] = "show";
		$cmdArgs[] = escapeshellarg($path);
		$cmd = new \OMV\System\Process("btrfs", $cmdArgs);
		$cmd->setQuiet(TRUE);
		$cmd->execute($output, $exitStatus);
		if ($exitStatus !== 0)
			return FALSE;
		$result = [];
		// Parse the output:
		//
		// test01
		//     Name:            test01
		//     UUID:            9b0fa98a-c1aa-934a-83c7-e48ec4d806db
		//     Parent UUID:     -
		//     Received UUID:   -
		//     Creation time:   2023-02-01 16:12:53 +0300
		//     Subvolume ID:    259
		//     Generation:      32
		//     Gen at creation: 12
		//     Parent ID:       5
		//     Top level ID:    5
		//     Flags:           -
		//     Snapshot(s):
		//                      .snapshots/test01-foo-bar
		foreach ($output as $outputk => $outputv) {
			$regex = "/^\s+([\w ]+):\s+(.+)$/";
			if (1 !== preg_match($regex, $outputv, $matches)) {
				continue;
			}
			$key = mb_strtolower(str_replace(" ", "_", $matches[1]));
			$result[$key] = $matches[2];
		}
		return $result;
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
		$cmd = new \OMV\System\Process("btrfs", $cmdArgs);
		$cmd->execute();
	}

	/**
	 * Delete a subvolume.
	 * @param string $path The path of the subvolume.
	 */
	public static function deleteSubvolume($path) {
		$cmdArgs = [];
		$cmdArgs[] = "subvolume";
		$cmdArgs[] = "delete";
		$cmdArgs[] = "--verbose";
		$cmdArgs[] = escapeshellarg($path);
		$cmd = new \OMV\System\Process("btrfs", $cmdArgs);
		$cmd->execute();
	}

	/**
	 * Get a list of snapshots for the specified subvolume.
	 * @param string $path The path of the subvolume.
	 * @return Returns a list of snapshots.
	 */
	public static function listSnapshots($path) {
		$cmdArgs = [];
		$cmdArgs[] = "subvolume";
		$cmdArgs[] = "list";
		$cmdArgs[] = "-s";
		$cmdArgs[] = "-q";
		$cmdArgs[] = "-u";
		$cmdArgs[] = escapeshellarg($path);
		$cmd = new \OMV\System\Process("btrfs", $cmdArgs);
		$cmd->execute($output);
		$result = [];
		// Parse the output:
		//
		// ID 262 gen 32 cgen 32 top level 5 otime 2023-02-02 15:01:14 parent_uuid 9b0fa98a-c1aa-934a-83c7-e48ec4d806db uuid d1cad0c2-10f7-8441-a978-c5e1c6ecf40b path .snapshots/test01-foo-bar
		foreach ($output as $outputk => $outputv) {
			$regex = "/^ID (\d+) gen (\d+) cgen (\d+) top level (\d+) otime (.+) parent_uuid (.+) uuid (.+) path (.+)$/";
			if (1 !== preg_match($regex, $outputv, $matches)) {
				continue;
			}
			$result[] = [
				"id" => $matches[1],
				"gen" => $matches[2],
				"cgen" => $matches[3],
				"top_level" => $matches[4],
				"otime" => $matches[5],
				"parent_uuid" => $matches[6],
				"uuid" => $matches[7],
				"path" => $matches[8],
				"name" => basename($matches[8])
			];
		}
		return $result;
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
			$cmdArgs[] = "-r";
		}
		$cmdArgs[] = escapeshellarg($path);
		$cmdArgs[] = escapeshellarg($target);
		$cmd = new \OMV\System\Process("btrfs", $cmdArgs);
		$cmd->execute();
	}

	/**
	 * Delete a snapshot of a specified subvolume.
	 * @param string $path The path of the subvolume.
	 * @param string|int $id The ID of the subvolume. Defaults to NULL.
	 * @return void
	 */
	public static function deleteSnapshot($path, $id = NULL) {
		$cmdArgs = [];
		$cmdArgs[] = "subvolume";
		$cmdArgs[] = "delete";
		if (!is_null($id)) {
			$cmdArgs[] = "--subvolid";
			$cmdArgs[] = $id;
		}
		$cmdArgs[] = escapeshellarg($path);
		$cmd = new \OMV\System\Process("btrfs", $cmdArgs);
		$cmd->execute();
	}
}
