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

require_once("openmediavault/functions.inc");

/**
 * Abstract class implementing generic file systems.
 * @ingroup api
 */
class FilesystemAbstract {
	protected $uuid = "";
	protected $type = "";
	protected $deviceFile = "";
	protected $label = "";
	protected $partEntry = NULL;
	protected $usage = "";
	private $dataCached = FALSE;
	private $backend = NULL;

	/**
	 * Constructor
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
	 */
	public function __construct($id) {
		if (!is_devicefile($id))
			$this->uuid = $id;
		else
			$this->deviceFile = $id;
	}

	protected function isCached() {
		return $this->dataCached;
	}

	protected function setCached($cached) {
		return $this->dataCached = $cached;
	}

	/**
	 * Get the filesystem detailed information.
	 * @private
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	protected function getData() {
		if (FALSE !== $this->isCached())
			return;

		if (!empty($this->uuid)) {
			// Get the device name containing the file system. This is
			// required for the blkid low-level probing mode.
			// Do not use /dev/disk/by-uuid/xxx because this will cause
			// some problems:
			// - After creating an RAID the filesystem is not listed in
			//   /dev/disk/by-uuid.
			// - After a reboot a LVM2 logical volume is listed in
			//   /dev/disk/by-uuid but pointing to /dev/dm-X and not to
			//   /dev/mapper which is prefered.
			$cmdArgs = [];
			$cmdArgs[] = sprintf("UUID=%s", escapeshellarg($this->uuid));
			$cmd = new \OMV\System\Process("findfs", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			$this->deviceFile = $output[0];
			unset($cmdArgs, $output);
		}

		// If the device file looks like /dev/disk/by-(id|label|path|uuid)/*
		// then it is necessary to get the /dev/xxx equivalent.
		// Note, limit this to the given types, otherwise unexpected behaviors
		// might occur, e.g. the name of the LVM device /dev/mapper/vg0-lv0
		// will be replaced by /dev/dm-0.
		if (1 == preg_match("/^\/dev\/disk\/by-.+$/", $this->deviceFile))
			$this->deviceFile = realpath($this->deviceFile);

		// Get the file system information.
		$cmdArgs = [];
		$cmdArgs[] = "-p";
		$cmdArgs[] = "-o full";
		$cmdArgs[] = escapeshellarg($this->deviceFile);
		$cmd = new \OMV\System\Process("blkid", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);

		// Parse command output:
		// /dev/sda1: UUID="3d9be906-d592-449b-93c4-3dd8888801b2" TYPE="ext3"
		// /dev/sdg1: LABEL="test" UUID="d19bcea0-a323-4aea-9791-c6578180c129" TYPE="jfs"
		// /dev/sda1: UUID="128917b7-718a-4361-ab6f-6b1725564cb8" VERSION="1.0" TYPE="ext4" USAGE="filesystem" PART_ENTRY_SCHEME="dos" PART_ENTRY_TYPE="0x83" PART_ENTRY_FLAGS="0x80" PART_ENTRY_NUMBER="1" PART_ENTRY_OFFSET="2048" PART_ENTRY_SIZE="30074880" PART_ENTRY_DISK="8:0"
		// /dev/sdc1: LABEL="xfs" UUID="83d116e4-8b95-4e49-86e4-369615079b33" TYPE="xfs" USAGE="filesystem" PART_ENTRY_SCHEME="gpt" PART_ENTRY_UUID="8856f019-67da-4813-ab75-9fc3c3fe056f" PART_ENTRY_TYPE="0fc63daf-8483-4772-8e79-3d69d8477de4" PART_ENTRY_NUMBER="1" PART_ENTRY_OFFSET="2048" PART_ENTRY_SIZE="206815" PART_ENTRY_DISK="8:32"
		// /dev/sdg1: LABEL="Traveller" UUID="2218-DF1C" VERSION="FAT32" TYPE="vfat" USAGE="filesystem" PART_ENTRY_SCHEME="dos" PART_ENTRY_TYPE="0xc" PART_ENTRY_FLAGS="0x80" PART_ENTRY_NUMBER="1" PART_ENTRY_OFFSET="62" PART_ENTRY_SIZE="15635532" PART_ENTRY_DISK="8:96"
		$regex = '/^(\S+): (.+)$/i';
		if (1 !== preg_match($regex, $output[0], $matches))
			return FALSE;
		// Set default values and extract key/value pairs.
		$data = [
			"devicefile" => $matches[1],
			"uuid" => "",
			"label" => "",
			"type" => "",
			"part_entry_scheme" => "",
			"part_entry_uuid" => "",
			"part_entry_type" => "",
			"part_entry_flags" => "",
			"part_entry_number" => "",
			"part_entry_offset" => "",
			"part_entry_size" => "",
			"part_entry_disk" => "",
			"usage" => ""
		];
		$parts = preg_split("/(\S+=\\\"[^\\\"]+\\\")|[\s]+/", $matches[2],
		  -1, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE);
		foreach ($parts as $partk => $partv) {
			$keyValue = explode("=", $partv);
			if (count($keyValue) != 2)
				continue;
			$data[strtolower($keyValue[0])] = substr($keyValue[1], 1, -1);
		}

		$this->deviceFile = $data['devicefile'];
		$this->uuid = $data['uuid'];
		$this->label = $data['label'];
		$this->type = $data['type'];
		$this->usage = $data['usage'];
		$this->partEntry = [
			"scheme" => $data['part_entry_scheme'],
			"uuid" => $data['part_entry_uuid'],
			"type" => $data['part_entry_type'],
			"flags" => $data['part_entry_flags'],
			"number" => $data['part_entry_number'],
			"offset" => $data['part_entry_offset'],
			"size" => $data['part_entry_size'],
			"disk" => $data['part_entry_disk']
		];

		// Set flag to mark information has been successfully read.
		$this->setCached(TRUE);
	}

	/**
	 * Refresh the cached information.
	 * @return None.
	 */
	public function refresh() {
		$this->setCached(FALSE);
		$this->getData();
	}

	/**
	 * Checks if the filesystem exists.
	 * @return TRUE if the device exists, otherwise FALSE.
	 */
	public function exists() {
		try {
			$this->getData();
		} catch(\Exception $e) {
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Check if the filesystem has an UUID, e.g. <ul>
	 * \li 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 * \li 7A48-BA97 (FAT)
	 * \li 2ED43920D438EC29 (NTFS)
	 * </ul>
	 * @see http://wiki.ubuntuusers.de/UUID
	 * @return Returns TRUE if the filesystem has an UUID, otherwise FALSE.
	 */
	public function hasUuid() {
		$uuid = $this->getUuid();
		return is_fs_uuid($uuid);
	}

	/**
	 * Assert that the filesystem has an UUID.
	 * @return None.
	 * @throw \OMV\AssertException
	 */
	public function assertUuid() {
		if (FALSE === $this->hasUuid()) {
			throw new \OMV\AssertException(
			  "Filesystem '%s' does not have an UUID.",
			  $this->getDeviceFile());
		}
	}

	/**
	 * Get the UUID of the filesystem.
	 * @see http://wiki.ubuntuusers.de/UUID
	 * @return The UUID of the filesystem.
	 */
	public function getUuid() {
		$this->getData();
		return $this->uuid;
	}

	/**
	 * Check if the filesystem has a label.
	 * @return Returns TRUE if the filesystem has a label, otherwise FALSE.
	 */
	public function hasLabel() {
		$label = $this->getLabel();
		return !empty($label);
	}

	/**
	 * Assert that the filesystem has a label.
	 * @return None.
	 * @throw \OMV\AssertException
	 */
	public function assertLabel() {
		if (FALSE === $this->hasLabel()) {
			throw new \OMV\AssertException(
			  "Filesystem '%s' does not have a label.",
			  $this->getDeviceFile());
		}
	}

	/**
	 * Get the filesystem label.
	 * @return The filesystem label.
	 */
	public function getLabel() {
		$this->getData();
		return $this->label;
	}

	/**
	 * Get the filesystem type, e.g. 'ext3' or 'vfat'.
	 * @return The filesystem type.
	 */
	public function getType() {
		$this->getData();
		return $this->type;
	}

	/**
	 * Get the partition scheme, e.g. 'gpt', 'mbr', 'apm' or 'dos'.
	 * @deprecated
	 * @return The filesystem type, otherwise FALSE.
	 */
	public function getPartitionType() {
		return $this->getPartitionScheme();
	}

	/**
	 * Get the partition scheme, e.g. 'gpt', 'mbr', 'apm' or 'dos'.
	 * @return The filesystem type.
	 */
	public function getPartitionScheme() {
		$this->getData();
		return $this->partEntry['scheme'];
	}

	/**
	 * Get the usage, e.g. 'other' or 'filesystem'.
	 * @return The filesystem type.
	 */
	public function getUsage() {
		$this->getData();
		return $this->usage;
	}

	/**
	 * Get the partition entry information.
	 * @return An array with the fields \em scheme, \em uuid, \em type,
	 *   \em flags, \em number, \em offset, \em size and \em disk.
	 */
	public function getPartitionEntry() {
		$this->getData();
		return $this->partEntry;
	}

	/**
	 * Get the device path of the filesystem, e.g /dev/sdb1.
	 * @return The device name.
	 */
	public function getDeviceFile() {
		$this->getData();
		return $this->deviceFile;
	}

	/**
	 * Get the canonical device file, e.g. <ul>
	 * \li /dev/root -> /dev/sde1
	 * \li /dev/disk/by-uuid/4B04EA317E4AA567 -> /dev/sdd1
	 * </ul>
	 */
	public function getCanonicalDeviceFile() {
		return realpath($this->deviceFile);
	}

	/**
	 * Get the device path by UUID, e.g. <ul>
	 * \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 * \li /dev/disk/by-uuid/2ED43920D438EC29 (NTFS)
	 * </ul>
	 * @return The device path (/dev/disk/by-uuid/xxx).
	 */
	public function getDeviceFileByUuid() {
		$this->getData();
		$this->assertUuid();
		$deviceFile = sprintf("/dev/disk/by-uuid/%s", $this->getUuid());
		if (FALSE === is_block($deviceFile)) {
			throw new \OMV\Exception(
			  "Filesystem '%s' does not have a disk/by-uuid device file.",
			  $this->getDeviceFile());
		}
		return $deviceFile;
	}

	/**
	 * Check whether the filesystem has a /dev/disk/by-uuid/xxx device path.
	 * @return Returns TRUE if a disk/by-uuid device path exists,
	 *   otherwise FALSE.
	 */
	public function hasDeviceFileByUuid() {
		try {
			$this->getDeviceFileByUuid();
		} catch(\Exception $e) {
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Get the device path by ID, e.g. <ul>
	 * \li /dev/disk/by-id/usb-Kingston_DataTraveler_G2_001CC0EC21ADF011C6A20E35-0:0-part1
	 * </ul>
	 * @return The device path (/dev/disk/by-id/xxx).
	 */
	public function getDeviceFileById() {
		$this->getData();
		// Get the storage device file of the filesystem.
		$sd = \OMV\System\Storage\StorageDevice::getStorageDevice(
		  $this->getDeviceFile());
		// Try to get the disk/by-id device file.
		$deviceFile = $sd->getDeviceFileById();
		if (FALSE === is_block($deviceFile)) {
			throw new \OMV\Exception(
			  "Filesystem '%s' does not have a disk/by-id device file.",
			  $this->getDeviceFile());
		}
		return $deviceFile;
	}

	/**
	 * Check whether the filesystem has a /dev/disk/by-id/xxx device path.
	 * @return Returns TRUE if a disk/by-id device path exists,
	 *   otherwise FALSE.
	 */
	public function hasDeviceFileById() {
		try {
			$this->getDeviceFileById();
		} catch(\Exception $e) {
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Get the device path by label, e.g. <ul>
	 * \li /dev/disk/by-label/data
	 * </ul>
	 * @return The device path (/dev/disk/by-label/xxx).
	 */
	public function getDeviceFileByLabel() {
		$this->getData();
		$this->assertLabel();
		$deviceFile = sprintf("/dev/disk/by-label/%s", $this->getLabel());
		if (FALSE === is_block($deviceFile)) {
			throw new \OMV\Exception(
			  "Filesystem '%s' does not have a disk/by-label device file.",
			  $this->getDeviceFile());
		}
		return $deviceFile;
	}

	/**
	 * Check whether the filesystem has a /dev/disk/by-label/xxx device path.
	 * @return Returns TRUE if a disk/by-label device path exists,
	 *   otherwise FALSE.
	 */
	public function hasDeviceFileByLabel() {
		try {
			$this->getDeviceFileByLabel();
		} catch(\Exception $e) {
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Get the device file of the storage device which contains this
	 * file system.
	 * Example:
	 * <ul>
	 * \li /dev/sdb1 => /dev/sdb
	 * \li /dev/cciss/c0d0p2 => /dev/cciss/c0d0
	 * </ul>
	 * @return The device file of the underlying storage device.
	 */
	public function getParentDeviceFile() {
		$this->getData();
		$deviceFile = $this->getDeviceFile();
		// The following line is not necessary but will be kept to be safe.
		// If the device file looks like /dev/disk/by-(id|label|path|uuid)/*
		// then it is necessary to get the /dev/xxx equivalent.
		if (1 == preg_match("/^\/dev\/disk\/by-.+$/", $deviceFile))
			$deviceFile = realpath($deviceFile);
		// Truncate the partition appendix, e.g.:
		// - /dev/sdb1 => /dev/sdb
		// - /dev/cciss/c0d0p2 => /dev/cciss/c0d0
		// - /dev/mapper/vg0-lv0 => /dev/mapper/vg0-lv0
		// - /dev/dm-0 => /dev/dm-0
		// - /dev/md0 => /dev/md0
		// - /dev/loop0 => /dev/loop0
		$mngr = \OMV\System\Storage\Backend\Manager::getInstance();
		if (NULL === ($backend = $mngr->getBackend($deviceFile)))
			return FALSE;
		$parentDeviceFile = $backend->baseDeviceFile($deviceFile);
		// Get the storage device file of the filesystems parent device, e.g.
		// - /dev/sdb
		$sd = \OMV\System\Storage\StorageDevice::getStorageDevice(
		  $parentDeviceFile);
		// Try to get the disk/by-id device file, e.g.
		// - /dev/disk/by-id/ata-WDC_WD15EARS-00MBWR0_WD-WMAZA2574223
		// - /dev/disk/by-id/wwn-0x50023ed655e21720
		if (TRUE === $sd->hasDeviceFileById())
			$parentDeviceFile = $sd->getDeviceFileById();
		return $parentDeviceFile;
	}

	/**
	 * Get all devices that make up the filesystem.
	 * @return Component devices of the filesystem, otherwise FALSE.
	 */
	public function getDeviceFiles() {
		$this->getData();
		// Get the device file in the following prefered order:
		// - /dev/disk/by-label/xxx
		// - /dev/disk/by-uuid/xxx
		// - /dev/disk/by-id/xxx
		// - /dev/xxx
		if (TRUE === $this->hasDeviceFileByLabel())
			$deviceFile = $this->getDeviceFileByLabel();
		else if (TRUE === $this->hasDeviceFileByUuid())
			$deviceFile = $this->getDeviceFileByUuid();
		else if (TRUE === $this->hasDeviceFileById())
			$deviceFile = $this->getDeviceFileById();
		else
			$deviceFile = $this->getDeviceFile();
		return [ $deviceFile ];
	}

	/**
	 * Check if the filesystem is a multi-device filesystem, e.g. (BTRFS).
	 * @return Returns TRUE if the filesystem has multiple devices,
	 *   otherwise FALSE.
	 */
	public function hasMultipleDevices() {
		if (FALSE === ($devs = $this->getDeviceFiles()))
			return FALSE;
		return (1 < count($devs));
	}

	/**
	 * Get the filesystem block size.
	 * @return The block size.
	 * @throw \OMV\ExecException
	 */
	public function getBlockSize() {
		$this->getData();
		$cmdArgs = [];
		$cmdArgs[] = "--getbsz";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("blockdev", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		return intval($output[0]);
	}

	/**
	 * Grow the filesystem.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function grow() {
		return FALSE;
	}

	/**
	 * Shrink the filesystem.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function shrink() {
		return FALSE;
	}

	/**
	 * Remove the filesystem.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function remove() {
		$this->getData();
		// Whether the partition schema is 'dos' then it is necessary to
		// erase the MBR before, otherwise 'wipefs' fails, e.g.:
		// wipefs: error: /dev/sdh1: appears to contain 'dos' partition table
		if (in_array($this->getPartitionScheme(), [ "dos", "vfat" ])) {
			// http://en.wikipedia.org/wiki/Master_boot_record
			$cmdArgs = [];
			$cmdArgs[] = "if=/dev/zero";
			$cmdArgs[] = sprintf("of=%s", escapeshellarg(
			  $this->getDeviceFile()));
			$cmdArgs[] = "count=1";
			$cmd = new \OMV\System\Process("dd", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute();
			unset($cmdArgs);
		}
		// Finally remove the filesystem.
		$cmdArgs = [];
		$cmdArgs[] = "-a";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("wipefs", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Get the mount point of the given filesystem.
	 * @return The mountpoint of the filesystem or FALSE.
	 * @throw \OMV\ExecException
	 */
	public function getMountPoint() {
		$this->getData();
		$cmdArgs = [];
		$cmdArgs[] = "-cnrv";
		$cmdArgs[] = "-o SOURCE,UUID,TARGET";
		$cmd = new \OMV\System\Process("findmnt", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		// Parse command output:
		// sysfs  /sys
		// proc  /proc
		// udev  /dev
		// devpts  /dev/pts
		// tmpfs  /run
		// /dev/sda1 df68201c-c816-4228-9bf9-5563b03e6bc1 /
		// tmpfs  /run/lock
		// tmpfs  /run/shm
		// tmpfs  /tmp
		// /dev/sde1 8c982ec2-8aa7-4fe2-a912-7478f0429e06 /media/8c982ec2-8aa7-4fe2-a912-7478f0429e06
		// /dev/sdc1 ea792ccb-cce6-409d-ab9b-0e3fd2bfbfc4 /media/ea792ccb-cce6-409d-ab9b-0e3fd2bfbfc4
		// /dev/md0 2853402e-fe8f-443b-abd5-c42892f25be1 /media/2853402e-fe8f-443b-abd5-c42892f25be1
		// rpc_pipefs  /var/lib/nfs/rpc_pipefs
		// /dev/sdf1 7A48-BA97 /media/7A48-BA97
		$result = FALSE;
		foreach ($output as $outputk => $outputv) {
			// Skip everything that does not look like '/dev/xxx'.
			$regex = '/^(\/dev\/\S+)\s+(\S+)\s+(\S+)$/i';
			if (1 !== preg_match($regex, $outputv, $matches))
				continue;
			if ($matches[2] !== $this->getUuid())
				continue;
			$result = $matches[3];
			break;
		}
		return $result;
	}

	/**
	 * Get statistics from a mounted filesystem.
	 * @return The filesystem statistics if successful, otherwise FALSE. The
	 *   following fields are included: \em devicefile, \em type, \em blocks,
	 *   \em size, \em used, \em available, \em percentage and \em mountpoint.
	 *   Please note, the fields \em size, \em used and \em available are
	 *   strings and their unit is 'B' (bytes).
	 * @throw \OMV\ExecException
	 */
	public function getStatistics() {
		$this->getData();
		// Get the mount point of the filesystem.
		if (FALSE === ($mountPoint = $this->getMountPoint()))
			return FALSE;
		$cmdArgs = [];
		$cmdArgs[] = "-PT";
		$cmdArgs[] = escapeshellarg($mountPoint);
		$cmd = new \OMV\System\Process("df", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		// Parse command output:
		// Filesystem                                             Type     1024-blocks    Used Available Capacity Mounted on
		// rootfs                                                 rootfs      14801380 1392488  12657020      10% /
		// udev                                                   devtmpfs       10240       0     10240       0% /dev
		// tmpfs                                                  tmpfs         102356     324    102032       1% /run
		// /dev/disk/by-uuid/128917b7-718a-4361-ab6f-6b1725564cb8 ext4        14801380 1392488  12657020      10% /
		// tmpfs                                                  tmpfs           5120       0      5120       0% /run/lock
		// tmpfs                                                  tmpfs         342320       0    342320       0% /run/shm
		// tmpfs                                                  tmpfs         511768       4    511764       1% /tmp
		// /dev/sdb1                                              ext4          100156    4164     95992       5% /media/b994e5e8-94ae-482c-ae54-70a0bbb2737e
		$result = FALSE;
		foreach ($output as $outputk => $outputv) {
			$matches = preg_split("/[\s,]+/", $outputv);
			if (0 !== strcasecmp($mountPoint, $matches[6]))
				continue;
			$result = [
				"devicefile" => $this->deviceFile,
				"type" => $matches[1],
				"blocks" => $matches[2],
				"size" => bcmul($matches[2], "1024", 0),
				"used" => binary_convert($matches[3], "KiB", "B"),
				"available" => binary_convert($matches[4], "KiB", "B"),
				"percentage" => intval(trim($matches[5], "%")),
				"mountpoint" => $matches[6]
			];
			break;
		}
		return $result;
	}

	/**
	 * Check if a filesystem is mounted.
	 * @return TRUE if the filesystem is mounted, otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function isMounted() {
		$this->getData();
		$cmdArgs = [];
		$cmdArgs[] = "-cnrv";
		$cmdArgs[] = "-o SOURCE,UUID";
		$cmd = new \OMV\System\Process("findmnt", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		// Parse command output:
		// sysfs
		// proc
		// udev
		// devpts
		// tmpfs
		// /dev/sda1 df68201c-c816-4228-9bf9-5563b03e6bc1
		// /dev/sde1 8c982ec2-8aa7-4fe2-a912-7478f0429e06
		// /dev/sdc1 ea792ccb-cce6-409d-ab9b-0e3fd2bfbfc4
		// /dev/md0 2853402e-fe8f-443b-abd5-c42892f25be1
		// /dev/sdf1 7A48-BA97
		$result = FALSE;
		foreach ($output as $outputk => $outputv) {
			// Skip everything that does not look like '/dev/xxx'.
			$regex = '/^(\/dev\/\S+)\s+(\S+)$/i';
			if (1 !== preg_match($regex, $outputv, $matches))
				continue;
			if ($matches[2] !== $this->getUuid())
				continue;
			$result = TRUE;
			break;
		}
		return $result;
	}

	/**
	 * Mount the filesystem by its device file or UUID.
	 * @param options Additional mount options. Defaults to "".
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function mount($options = "") {
		$cmdArgs = [ "-v" ];
		if (!empty($options)) {
			if (!is_array($options))
				$options = [ $options ];
			$cmdArgs[] = sprintf("-o %s", implode(",", $options));
		}
		if (TRUE === $this->hasUuid())
			$cmdArgs[] = sprintf("-U %s", $this->getUuid());
		else
			$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("mount", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Unmount the filesystem.
	 * @param force Set to TRUE to force unmount. Defaults to FALSE.
	 * @param lazy Set to TRUE to lazy unmount. Defaults to FALSE.
	 * @return None.
	 * @throw \OMV\ExecException
	 */
	public function umount($force = FALSE, $lazy = FALSE) {
		$cmdArgs = [ "-v" ];
		if (TRUE === $force)
			$cmdArgs[] = "-f";
		if (TRUE === $lazy)
			$cmdArgs[] = "-l";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("umount", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * See if a directory is a mountpoint.
	 * @param dir The directory path to check.
	 * @return TRUE if the directory is a mountpoint, otherwise FALSE.
	 */
	public static function isMountPoint($dir) {
		if (!is_dir($dir))
			return FALSE;
		if (FALSE === ($stat = stat($dir)))
			return FALSE;
		if (FALSE === ($stat2 = stat(sprintf("%s/..", $dir))))
			return FALSE;
		return (($stat.dev != $stat2.dev) || (($stat.dev == $stat2.dev) &&
		  ($stat.ino == $stat2.ino))) ? TRUE : FALSE;
	}

	/**
	 * Get the directory where the filesystem should be mounted to. Note,
	 * this path is OMV specific: /media/<FSUUID>.
	 * @param uuid The UUID of the filesystem.
	 * @return The path where to mount the filesystem, e.g.
	 *   /media/85732966-949a-4d8b-87d7-d7e6681f787e.
	 */
	public static function buildMountPath($uuid) {
		return build_path(DIRECTORY_SEPARATOR, \OMV\Environment::get(
		  "OMV_MOUNT_DIR"), $uuid);
	}

	/**
	 * Check if the given device file contains a file system.
	 * @param deviceFile The devicefile to check.
	 * @return TRUE if the devicefile has a file system, otherwise FALSE.
	 */
	public static function hasFileSystem($deviceFile) {
		// An alternative implementation is:
		// blkid -p -u filesystem <devicefile>
		// Scan output for tag PTTYPE.
		$cmdArgs = sprintf("export LANG=C; blkid | grep -E '^%s.*:".
		  ".+\sTYPE=.+$'", $deviceFile);
		$cmd = new \OMV\System\Process($cmdArgs);
		$cmd->setQuiet(TRUE);
		$cmd->execute($output, $exitStatus);
		if ($exitStatus !== 0)
			return FALSE;
		return TRUE;
	}

	/**
	 * Set the backend of this filesystem.
	 */
	final public function setBackend(Backend\BackendAbstract $backend) {
		$this->backend = $backend;
	}

	/**
	 * Get the backend of this filesystem.
	 */
	final public function getBackend() {
		return $this->backend;
	}
}
