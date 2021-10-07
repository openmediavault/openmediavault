<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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

		// Parse command output:
		// System, RAID1: total=8388608, used=4096
		// System, single: total=4194304, used=0
		// Data+Metadata, RAID1: total=67108864, used=28672
		// Data+Metadata, single: total=8388608, used=0
		// GlobalReserve, single: total=4194304, used=0
		// Initialize arrays.
		$allocated = $allocatedUnused = $used = $allocatedRaw =
		  $allocatedUnusedRaw = $usedRaw = [
			"Data" => 0,
			"Metadata" => 0,
			"Data+Metadata"	=> 0,
			"System" => 0,
			"GlobalReserve"	=> 0
		];
		$regex = "/^([\w\+]+), ([a-zA-Z0156]+): total=([0-9]+), ".
		  "used=([0-9]+)$/i";
		foreach (preg_filter($regex, "$1 $2 $3 $4", $output) as $rowv) {
			list($type, $profile, $t, $u) = explode(" ", $rowv);
			switch ($profile) {
				case "single":
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
			  bcmul(bcsub($t, $u, 0), $factor, 0));
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

		// Parse command output:
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
		// Data ratio is calculated based only on Data (or Data+Metadata).
		$dataRatio = bcdiv(
			array_sum(array_filter($usedRaw, function($type) {
				return in_array($type, [ 'Data', 'Data+Metadata' ]);
			}, ARRAY_FILTER_USE_KEY)),
			array_sum(array_filter($used, function($type) {
				return in_array($type, [ 'Data', 'Data+Metadata' ]);
			}, ARRAY_FILTER_USE_KEY)),
			2);
		// Catch and fix division-by-zero.
		if (is_null($dataRatio)) {
			$dataRatio = "1.0";
		}

		// Update the information.
		// Raw disk(s) size.
		$stats = array_merge($stats, [
			"size" => $totalBytes,
			"blocks" => bcdiv($totalBytes, "1024", 0),
			// Usage by all profiles (excluding redundancy).
			"used" => array_sum($used),
			// Space available for Data, adjusted for average data ratio.
			"available" => bcdiv($available, $dataRatio, 0),
			// Raw disk usage, all profiles, divided by raw size.
			"percentage" => bcmul(bcdiv(array_sum($usedRaw), $totalBytes, 3), 100, 0)
		]);

		return $stats;
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
}
