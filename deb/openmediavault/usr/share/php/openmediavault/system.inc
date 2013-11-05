<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
require_once("openmediavault/globals.inc");
require_once("openmediavault/object.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/util.inc");

/**
 * Helper class that provides various functions regarding the Linux system.
 * @ingroup api
 */
class OMVSystem extends OMVObject {
	/**
	 * Get the device file where the operating system is installed on (e.g.
	 * /dev/sda1).
	 * @return Returns the device file where the operating system is installed
	 *   on or FALSE on failure.
	 */
	public static function getRootDeviceFile() {
		return realpath("/dev/root");
	}

	/**
	 * Get the /etc/login.defs configuration
	 * @return Array containing the configuration as key/value pairs or
	 *   FALSE on failure.
	 */
	public static function getLoginDefs() {
		$defs = array();
		// Extract the lines that are not commented out.
		// Parse file content:
		// #
		// # Min/max values for automatic uid selection in useradd
		// #
		// UID_MIN                  1000
		// UID_MAX                 60000
		// # System accounts
		// #SYS_UID_MIN              100
		// #SYS_UID_MAX              999
		$rows = file("/etc/login.defs");
		foreach($rows as $rowk => $rowv) {
			// Skip comments.
			if("#" == substr($rowv, 0, 1))
				continue;
			// Extract key/value.
			$regex = '/^(\S+)\s+(\S+)$/i';
			if(1 !== preg_match($regex, $rowv, $matches))
				continue;
			$defs[$matches[1]] = $matches[2];
		}
		return $defs;
	}

	/**
	 * Get the next free device name.
	 * @param type The type of the device, e.g. disk or iface
	 * @param name The device name, e.g. sda, hda, md, eth or bond
	 * @return The next free device name, e.g. md3
	 */
	public static function getNextDevice($type, $name) {
		$cmdList = array(
			"disk" => "cat /proc/partitions | awk '{print $4}'",
			"iface" => "netstat -i"
		);
		$cmd = sprintf("%s | grep '^%s[0-9]\{1,\}$' | sort -r | " .
		  "head -n 1", $cmdList[$type], $name);
		$deviceName = @OMVUtil::exec($cmd, $output);
		$deviceName = strrev($deviceName);
		return sprintf("%s%d", $name, !empty($deviceName) ?
		  intval($deviceName[0]) + 1 : 0);
	}

	/**
	 * Tell how long the system has been running.
	 * @param indexed Set to FALSE to get the uptime as string, otherwise an
	 *   indexed array with the fields \em seconds, \em minutes, \em hours
	 *   and \em days will be returned. Defaults to FALSE.
	 * @return The uptime as formated string, indexed array or FALSE on
	 *   failure.
	 */
	public static function uptime($indexed = FALSE) {
		$uptime = explode(" ", trim(file_get_contents("/proc/uptime")));
		$days = intval($uptime[0] / 86400);
		$hours = intval(($uptime[0] / 3600) % 24);
		$mins = intval(($uptime[0] / 60) % 60);
		$secs = intval($uptime[0] % 60);
		if(FALSE === $indexed)
			return sprintf("%d %s %d %s %d %s %d %s",
			  $days, ($days == 1) ? gettext("day") : gettext("days"),
			  $hours, ($hours == 1) ? gettext("hour") : gettext("hours"),
			  $mins, ($mins == 1) ? gettext("minute") : gettext("minutes"),
			  $secs, gettext("seconds"));
		else
			return array(
				"seconds" => $secs,
				"minutes" => $mins,
				"hours" => $hours,
				"days" => $days
			  );
	}

	/**
	 * Get load average in regard to both the CPU and IO over time.
	 * @return The CPU and IO utilization of the last one, five, and 10 minute
	 *   periods or FALSE on failure.
	 */
	public static function getLoadAverage() {
		$loadavg = explode(" ", trim(file_get_contents("/proc/loadavg")));
		return sprintf("%s, %s, %s", $loadavg[0], $loadavg[1], $loadavg[2]);
	}

	/**
	 * Get memory statistics.
	 * @return The memory statistics as array or FALSE on failure.
	 * array(
	 *   mem (
	 *     total => xxx, (bytes)
	 *     used => xxx, (bytes)
	 *     free => xxx, (bytes)
	 *     shared => xxx, (bytes)
	 *     buffers => xxx, (bytes)
	 *     cached => xxx, (bytes)
	 *   )
	 *   wobufferscache (
	 *     used => xxx, (bytes)
	 *     free => xxx, (bytes)
	 *   )
	 *   swap (
	 *     total => xxx, (bytes)
	 *     used => xxx, (bytes)
	 *     free => xxx, (bytes)
	 *   )
	 *   total (
	 *     total => xxx, (bytes)
	 *     used => xxx, (bytes)
	 *     free => xxx, (bytes)
	 *   )
	 * )
	 */
	public static function getMemoryStats() {
/*
		@OMVUtil::exec("cat /proc/meminfo", $output, $result);
		if($result !== 0) {
			return FALSE;
		}
		$result = array(
		  "total" => 0,
		  "free" => 0,
		  "used" => 0,
		  "percent" => 0
		);
		foreach($output as $outputk => $outputv) {
			if(preg_match('/^MemTotal:\s+(\d+)\s+kB$/i', $outputv,
			  $matches)) {
				$result['total'] = $matches[1] * 1024;
			} else if(preg_match('/^MemFree:\s+(\d+)\s+kB$/i', $outputv,
			  $matches)) {
				$result['free'] = $matches[1] * 1024;
			}
		}
		$result['used'] = $result['total'] - $result['free'];
		$result['percent'] = round(($result['used'] * 100) / $result['total']);
*/
		@OMVUtil::exec("free -b", $output, $result);
		if($result !== 0) {
			return FALSE;
		}
		$result = array(
			"mem" => array(
				"total" => 0,
				"used" => 0,
				"free" => 0,
				"shared" => 0,
				"buffers" => 0,
				"cached" => 0
			),
			"wobufferscache" => array(
				"used" => 0,
				"free" => 0
			),
			"swap" => array(
				"total" => 0,
				"used" => 0,
				"free" => 0
			),
			"total" => array(
				"total" => 0,
				"used" => 0,
				"free" => 0
			)
		);
		// Parse command output:
		//              total       used       free     shared    buffers     cached
		// Mem:     526987264  195010560  331976704          0   28565504  120373248
		// -/+ buffers/cache:   46071808  480915456
		// Swap:    401596416          0  401596416
		// Total:   928583680  195137536  733446144
		foreach($output as $outputk => $outputv) {
			if(preg_match('/^Mem:\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)$/',
			  $outputv, $matches)) {
				$result['mem'] = array(
					"total" => $matches[1],
					"used" => $matches[2],
					"free" => $matches[3],
					"shared" => $matches[4],
					"buffers" => $matches[5],
					"cached" => $matches[6]
				);
			} else if(preg_match('/^-\/\+ buffers\/cache:\s+(\d+)\s+(\d+)$/',
			  $outputv, $matches)) {
				$result['wobufferscache'] = array(
					"used" => $matches[1],
					"free" => $matches[2]
				);
			} else if(preg_match('/^Swap:\s+(\d+)\s+(\d+)\s+(\d+)$/',
			  $outputv, $matches)) {
				$result['swap'] = array(
					"total" => $matches[1],
					"used" => $matches[2],
					"free" => $matches[3]
				);
			} else if(preg_match('/^Total:\s+(\d+)\s+(\d+)\s+(\d+)$/',
			  $outputv, $matches)) {
				$result['swap'] = array(
					"total" => $matches[1],
					"used" => $matches[2],
					"free" => $matches[3]
				);
			}
		}
		return $result;
	}

	/**
	 * Get CPU statistics.
	 * @return The memory statistics as array or FALSE on failure.
	 * array(
	 *   modelname => xxx,
	 *   usage => xxx (percent)
	 * )
	 */
	public static function getCPUStats() {
		// Get the CPU model name. Take care about CPUs with multiple cores.
		@OMVUtil::exec("grep -Em1 '^(model name|Processor)' /proc/cpuinfo",
		  $output, $result);
		if($result !== 0) {
			return FALSE;
		}
		$output = explode(":", $output[0]);
		$modelName = trim($output[1]);
		$tprev = file("/proc/stat");
		sleep(1);
		$tnow = file("/proc/stat");
		// Extract values:
		// cpu  2255 34 2290 22625563 6290 127 456
		// * user: normal processes executing in user mode
		// * nice: niced processes executing in user mode
		// * system: processes executing in kernel mode
		// * idle: twiddling thumbs
		// * iowait: waiting for I/O to complete
		// * irq: servicing interrupts
		// * softirq: servicing softirqs
		$tprev = explode(" ", $tprev[0]);
		array_shift($tprev); array_shift($tprev);
		$tnow = explode(" ", $tnow[0]);
		array_shift($tnow); array_shift($tnow);
		// Calculate the total CPU time.
		$tprevTotal = array_sum($tprev);
		$tnowTotal = array_sum($tnow);
		// Calculate the CPU usage since we last checked.
		$diffIdle = $tnow[3] - $tprev[3];
		$diffTotal = $tnowTotal - $tprevTotal;
		return array(
			"modelname" => $modelName,
			"usage" => (($diffTotal - $diffIdle) / $diffTotal) * 100
		);
	}
}

/**
 * Helper class that implements functions regarding filesystems.
 * @ingroup api
 */
class OMVFilesystems extends OMVObject {
	/**
	 * Get a list of available/detected file systems.
	 * @return A list of file systems, otherwise FALSE. The array elements
	 *   contain the fields: \em devicefile, \em uuid, \em label and \em type.
	 *   The devicefile is used as key to access the array elements.
	 */
	public static function get() {
		$cmd = "export LANG=C; blkid -o full";
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			return FALSE;
		}
		$result = array();
		foreach($output as $outputk => $outputv) {
			// Parse command output:
			// /dev/sdb: UUID="e6a0f61e-d969-ffcb-d4c5-06e7711285a3" LABEL="dhcppc4:0" TYPE="linux_raid_member"
			// /dev/sdg1: LABEL="test" UUID="d19bcea0-a323-4aea-9791-c6578180c129" TYPE="jfs"
			$regex = '/^(\S+): (.+)$/i';
			if(1 !== preg_match($regex, $outputv, $matches))
				continue;
			$data = array(
				"devicefile" => $matches[1],
				"uuid" => "",
				"label" => "",
				"type" => ""
			);
			$output = explode(" ", $matches[2]);
			foreach($output as $outputk => &$outputv) {
				$keyValue = explode("=", $outputv);
				if(count($keyValue) != 2)
					continue;
				$data[strtolower($keyValue[0])] = substr($keyValue[1], 1, -1);
			}
			$result[$data['devicefile']] = $data;
/*
			// Alternative implementation:
			$regex = '/^(\S+): (LABEL="(\S+)" )?UUID="(\S+)" (LABEL="(\S+)" )?TYPE="(\S+)"$/i';
			if(1 !== preg_match($regex, $outputv, $matches))
				continue;
			$result[$matches[1]] = array(
				"devicefile" => $matches[1],
				"uuid" => $matches[4],
				"label" => !empty($matches[3]) ? $matches[3] : $matches[6],
				"type" => $matches[7]
			);
*/
		}
		return $result;
	}
}

/**
 * Class to get detailed informations about a given filesystem.
 * @ingroup api
 */
class OMVFilesystem extends OMVObject {
	protected $uuid = "";
	protected $type = "";
	protected $deviceFile = "";
	protected $label = "";
	protected $ptType = "";
	protected $usage = "";
	private $dataCached = FALSE;
	private $errStr = "";

	/**
	 * Constructor
	 * @param id The UUID or device path of the filesystem, e.g.
	 *   \li 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 *   \li 7A48-BA97 (DOS)
	 *   \li 2ED43920D438EC29 (NTFS)
	 *   \li /dev/sde1
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6-part1
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0-part2
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   \li /dev/cciss/c0d0p2
	 */
	public function __construct($id) {
		if(!is_devicefile($id))
			$this->uuid = $id;
		else
			$this->deviceFile = $id;
	}

	/**
	 * Get the error description of the last occured error.
	 * @return The error description of the last error if set.
	 */
	public function getError() {
		return implode("\n", $this->errStr);
	}

	/**
	 * Get the filesystem detailed informations.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if($this->dataCached !== FALSE)
			return TRUE;

		if(!empty($this->uuid)) {
			// Get the device name containing the file system. This is
			// required for the blkid low-level probing mode.
			// Do not use /dev/disk/by-uuid/xxx because this will cause
			// some problems:
			// - After creating an RAID the filesystem is not listed in
			//   /dev/disk/by-uuid.
			// - After a reboot a LVM2 logical volume is listed in
			//   /dev/disk/by-uuid but pointing to /dev/dm-X and not to
			//   /dev/mapper which is prefered.
			$cmd = sprintf("export LANG=C; findfs UUID=%s", $this->uuid);
			@OMVUtil::exec($cmd, $output, $result);
			if($result !== 0) {
				$this->errStr = $output;
				return FALSE;
			}
			$this->deviceFile = $output[0];
			unset($output);
		}

		// If the device file looks like /dev/disk/by-(id|label|path|uuid)/*
		// then it is necessary to get the /dev/xxx equivalent.
		// Note, limit this to the given types, otherwise unexpected behaviors
		// might occur, e.g. the name of the LVM device /dev/mapper/vg0-lv0
		// will be replaced by /dev/dm-0.
		if(1 == preg_match("/^\/dev\/disk\/by-.+$/", $this->deviceFile))
			$this->deviceFile = realpath($this->deviceFile);

		// Get the filesystem details.
		$cmd = sprintf("export LANG=C; blkid -p %s", $this->deviceFile);
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			$this->errStr = $output;
			return FALSE;
		}

		// Parse command output:
		// /dev/sda1: UUID="3d9be906-d592-449b-93c4-3dd8888801b2" TYPE="ext3"
		// /dev/sdg1: LABEL="test" UUID="d19bcea0-a323-4aea-9791-c6578180c129" TYPE="jfs"
		$regex = '/^(\S+): (.+)$/i';
		if(1 !== preg_match($regex, $output[0], $matches))
			return FALSE;

		// Set default values and extract key/value pairs.
		$data = array(
			"devicefile" => $matches[1],
			"uuid" => "",
			"label" => "",
			"type" => "",
			"pttype" => "",
			"usage" => ""
		);
		$output = preg_split("/(\S+=\\\"[^\\\"]+\\\")|[\s]+/", $matches[2],
		  -1, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE);
		foreach($output as $outputk => &$outputv) {
			$keyValue = explode("=", $outputv);
			if(count($keyValue) != 2)
				continue;
			$data[strtolower($keyValue[0])] = substr($keyValue[1], 1, -1);
		}

		$this->deviceFile = $data['devicefile'];
		$this->uuid = $data['uuid'];
		$this->label = $data['label'];
		$this->type = $data['type'];
		$this->ptType = $data['pttype'];
		$this->usage = $data['usage'];

		// Set flag to mark informations has been successfully read.
		$this->dataCached = TRUE;

		return TRUE;
	}

	/**
	 * Refresh the cached informations.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		if($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Checks if the filesystem exists.
	 * @return TRUE if the device exists, otherwise FALSE.
	 */
	public function exists() {
		if($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Get the UUID of the filesystem.
	 * @return The UUID of the filesystem, otherwise FALSE.
	 */
	public function getUuid() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->uuid;
	}

	/**
	 * Get the filesystem label.
	 * @return The filesystem label, otherwise FALSE.
	 */
	public function getLabel() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->label;
	}

	/**
	 * Get the filesystem type.
	 * @return The filesystem type, otherwise FALSE.
	 */
	public function getType() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->type;
	}

	/**
	 * Get the partition type, e.g. 'dos'.
	 * @return The filesystem type, otherwise FALSE.
	 */
	public function getPartitionType() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->ptType;
	}

	/**
	 * Get the usage, e.g. 'other' or 'filesystem'.
	 * @return The filesystem type, otherwise FALSE.
	 */
	public function getUsage() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->usage;
	}

	/**
	 * Get the device path of the filesystem, e.g /dev/sdb1.
	 * @return The device name, otherwise FALSE.
	 */
	public function getDeviceFile() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->deviceFile;
	}

	/**
	 * Get the device path by UUID, e.g.
	 * \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 * \li /dev/disk/by-uuid/2ED43920D438EC29 (NTFS)
	 * @return The device path (/dev/disk/by-uuid/xxx) if available,
	 *   otherwise /dev/xxx will be returned. In case of an error FALSE
	 *   will be returned.
	 */
	public function getDeviceFileByUuid() {
		if($this->getData() === FALSE)
			return FALSE;
		$deviceFileByUuid = sprintf("/dev/disk/by-uuid/%s",
		  $this->getUuid());
		if(is_block($deviceFileByUuid))
			return $deviceFileByUuid;
		return $this->getDeviceFile();
	}

	/**
	 * Get the device file of the storage device containing this
	 * file system.
	 * @return The device file of the underlying storage device,
	 *   otherwise FALSE.
	 */
	public function getParentDeviceFile() {
		if($this->getData() === FALSE)
			return FALSE;
		$conf = array(
			  $GLOBALS['OMV_REGEX_DEVNAME_HARDDISK'] => "/(\d+)$/",
			  $GLOBALS['OMV_REGEX_DEVNAME_SOFTWARERAID'] => "//",
			  $GLOBALS['OMV_REGEX_DEVNAME_HARDWARERAID'] => "/(p\d+)$/",
			  $GLOBALS['OMV_REGEX_DEVNAME_DEVICEMAPPER'] => "//",
			  $GLOBALS['OMV_REGEX_DEVNAME_LOOPDEVICE'] => "//"
		  );
		$storageDeviceFile = FALSE;
		$deviceFile = $this->getDeviceFile();
		// The following line is not necessary but will be kept to be safe.
		// If the device file looks like /dev/disk/by-(id|label|path|uuid)/*
		// then it is necessary to get the /dev/xxx equivalent.
		if(1 == preg_match("/^\/dev\/disk\/by-.+$/", $deviceFile))
			$deviceFile = realpath($deviceFile);
		// Truncate the partition appendix, e.g.:
		// - /dev/sdb1 => /dev/sdb
		// - /dev/cciss/c0d0p2 => /dev/cciss/c0d0
		// - /dev/mapper/vg0-lv0 => /dev/mapper/vg0-lv0
		// - /dev/dm-0 => /dev/dm-0
		// - /dev/md0 => /dev/md0
		// - /dev/loop0 => /dev/loop0
		$deviceName = str_replace("/dev/", "", $deviceFile);
		foreach($conf as $confk => $confv) {
			if(1 == preg_match("/^{$confk}(\d+|p\d+)?$/i", $deviceName)) {
				$storageDeviceFile = preg_replace($confv, "", $deviceFile);
				break;
			}
		}
		return $storageDeviceFile;
	}

	/**
	 * Get the filesystem block size.
	 * @return The block size, otherwise FALSE.
	 */
	public function getBlockSize() {
		if($this->getData() === FALSE)
			return FALSE;
		$cmd = sprintf("export LANG=C; blockdev --getbsz %s",
		  escapeshellarg($this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			$this->errStr = $output;
			return FALSE;
		}
		return intval($output[0]);
	}

	/**
	 * Get statistics from a mounted filesystem.
	 * @return The filesystem statistics if successful, otherwise FALSE. The
	 * following fields are included: \em devicefile, \em type, \em blocks,
	 * \em size, \em used, \em available, \em percentage and \em mountpoint.
	 * Please note, the fields \em size, \em used and \em available are
	 * strings and their unit is 'B' (bytes).
	 */
	public function getStatistics() {
		if($this->getData() === FALSE)
			return FALSE;
		$data = array();
		$cmd = sprintf("export LANG=C; df -PT %s", escapeshellarg(
		  $this->deviceFile));
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			$this->errStr = $output;
			return FALSE;
		}
		// Parse command output:
		// Filesystem    Type 1024-blocks      Used Available Capacity Mounted on
		// /dev/sdb1     ext4      100156      4164     90824       5% /media/f3c61f34-f640-472c-b27e-a8e441caf460
		$matches = preg_split("/[\s,]+/", array_pop($output));
		$data = array(
			"devicefile" => $this->deviceFile,
			"type" => $matches[1],
			"blocks" => $matches[2],
			"size" => bcmul($matches[2], "1024", 0),
			"used" => binary_convert($matches[3], "KiB", "B"),
			"available" => binary_convert($matches[4], "KiB", "B"),
			"percentage" => intval(trim($matches[5], "%")),
			"mountpoint" => $matches[6]
		);
		return $data;
	}

	/**
	 * Does the filesystem support POSIX ACL.
	 * @return TRUE if the filesystem supports POSIX ACL, otherwise FALSE.
	 */
	public function hasPosixAclSupport() {
		if($this->getData() === FALSE)
			return FALSE;
		return self::hasPosixAclSupportByType($this->getType());
	}

	/**
	 * Check if a filesystem is mounted.
	 * @return TRUE if the filesystem is mounted, otherwise FALSE.
	 */
	public function isMounted() {
		if($this->getData() === FALSE)
			return FALSE;
		return self::isMountedEx("filesystem", $this->getDeviceFile());
	}

	/**
	 * Mount the filesystem.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function mount() {
		$uuid = $this->getUuid();
		$cmd = sprintf("export LANG=C; mount -v %s", is_uuid($uuid) ?
		  "-U {$uuid}" : $this->getDeviceFile());
		@OMVUtil::exec($cmd, $this->errStr, $result);
		if($result !== 0)
			return FALSE;
		return TRUE;
	}

	/**
	 * Unmount the filesystem.
	 * @param force Set to TRUE to force unmount. Defaults to FALSE.
	 * @param lazy Set to TRUE to lazy unmount. Defaults to FALSE.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function umount($force = FALSE, $lazy = FALSE) {
		$cmdArgs = array("-v");
		if(TRUE === $force)
			$cmdArgs[] = "-f";
		if(TRUE === $lazy)
			$cmdArgs[] = "-l";
		$cmd = sprintf("export LANG=C; umount %s %s", implode(" ",
		  $cmdArgs), $this->getDeviceFile());
		@OMVUtil::exec($cmd, $this->errStr, $result);
		if($result !== 0)
			return FALSE;
		return TRUE;
	}

	/**
	 * Check if a filesystem is mounted.
	 * @param key The name of the attribute to search for. This can
	 *  be 'filesystem' or 'mountpoint'.
	 * @param value The value to be check for.
	 * @return TRUE if the filesystem is mounted, otherwise FALSE.
	 */
	public static function isMountedEx($key, $value) {
		$result = FALSE;
		foreach(file("/proc/mounts") as $rowk => $rowv) {
			// Parse command output:
			// rootfs / rootfs rw 0 0
			// none /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0
			// none /proc proc rw,nosuid,nodev,noexec,relatime 0 0
			// tmpfs /dev/shm tmpfs rw,nosuid,nodev,relatime 0 0
			// /dev/sde1 /home/ftp/data ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			// /dev/sde1 /media/46607e80-dc8e-427e-9939-7909985dfe5b ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			$matches = explode(" ", $rowv);
			switch($key) {
			case "filesystem":
				$result = ($matches[0] === $value);
				break;
			case "mountpoint":
				$result = ($matches[1] === $value);
				break;
			}
			if(TRUE === $result)
				break;
		}
		return $result;
	}

	/**
	 * Get the directory where the filesystem should be mounted to.
	 * @param uuid The UUID of the filesystem.
	 * @return The path where to mount the filesystem, e.g.
	 * /media/85732966-949a-4d8b-87d7-d7e6681f787e.
	 */
	public static function buildMountPath($uuid) {
		return sprintf("%s/%s", $GLOBALS['OMV_MOUNT_DIR'], $uuid);
	}

	/**
	 * Check if the given device file contains a file system.
	 * @param devicefile The devicefile to check.
	 * @return TRUE if the devicefile has a file system, otherwise FALSE.
	 */
	public static function hasFileSystem($devicefile) {
		// An alternative implementation is:
		// blkid -p -u filesystem <devicefile>
		// Scan output for tag PTTYPE.
		$cmd = sprintf("export LANG=C; blkid | grep -E '^%s[0-9]*:.+$'",
		  $devicefile);
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		return TRUE;
	}

	/**
	 * Does the filesystem support POSIX ACL.
	 * @param type The type of the filesystem, e.g. ext3, msdos.
	 * @return TRUE if the filesystem supports POSIX ACL, otherwise FALSE.
	 */
	public static function hasPosixAclSupportByType($type) {
		// The list of POSIX ACL supporting filesystems. See:
		// - http://de.wikipedia.org/wiki/Access_Control_List
		// - http://www.suse.de/~agruen/acl/linux-acls/online
		return in_array(strtolower($type), array("ext2", "ext3",
		  "ext4", "xfs", "jfs", "reiserfs", "btrfs"));
	}
}

/**
 * A class that implements a mount point.
 * @ingroup api
 */
class OMVMntEnt extends OMVObject {
	private $fsname;
	private $dir;
	private $errStr = "";

	/**
	 * Constructor
	 * @param fsname The name of the mounted filesystem.
	 * @param dir The filesystem path prefix.
	 */
	public function __construct($fsname, $dir) {
		$this->fsname = $fsname;
		$this->dir = $dir;
	}

	/**
	 * Get the error description of the last occured error.
	 * @return The error description of the last error if set.
	 */
	public function getError() {
		return implode("\n", $this->errStr);
	}

	/**
	 * Mount the given filesystem.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function mount() {
		$cmd = sprintf("export LANG=C; mount -v %s", escapeshellarg(
		  $this->dir));
		@OMVUtil::exec($cmd, $this->errStr, $result);
		if($result !== 0)
			return FALSE;
		return TRUE;
	}

	/**
	 * Unmount the given filesystem.
	 * @param force Set to TRUE to force unmount. Defaults to FALSE.
	 * @param lazy Set to TRUE to lazy unmount. Defaults to FALSE.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function umount($force = FALSE, $lazy = FALSE) {
		$cmdArgs = array("-v");
		if(TRUE === $force)
			$cmdArgs[] = "-f";
		if(TRUE === $lazy)
			$cmdArgs[] = "-l";
		$cmd = sprintf("export LANG=C; umount %s %s", implode(" ",
		  $cmdArgs), escapeshellarg($this->dir));
		@OMVUtil::exec($cmd, $this->errStr, $result);
		if($result !== 0)
			return FALSE;
		return TRUE;
	}

	/**
	 * Create the mount point of the filesystem.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function mkDir() {
		if(is_dir($this->dir))
			return TRUE;
		return mkdir($this->dir, 0700, TRUE);
	}

	/**
	 * Unlink the mount point of the filesystem.
	 * @param force Set to TRUE to ignore nonexistent files. Defaults to TRUE.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function unlinkDir($force = TRUE) {
		$cmd = sprintf("export LANG=C; rm -rf %s 2>&1", escapeshellarg(
		  $this->dir));
		@OMVUtil::exec($cmd, $this->errStr, $result);
		if($result !== 0)
			return FALSE;
		return TRUE;
	}

	/**
	 * Check if the given filesystem is mounted.
	 * @return TRUE if the filesystem is mounted, otherwise FALSE.
	 */
	public function isMounted() {
		$result = FALSE;
		foreach(file("/proc/mounts") as $rowk => $rowv) {
			// Parse command output:
			// rootfs / rootfs rw 0 0
			// none /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0
			// none /proc proc rw,nosuid,nodev,noexec,relatime 0 0
			// tmpfs /dev/shm tmpfs rw,nosuid,nodev,relatime 0 0
			// /dev/sde1 /home/ftp/data ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			// /dev/sde1 /media/46607e80-dc8e-427e-9939-7909985dfe5b ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			$matches = explode(" ", $rowv);
			if(($matches[0] == $this->fsname) || ($matches[1] == $this->dir)) {
				$result = TRUE;
				break;
			}
		}
		return $result;
	}

	/**
	 * Get the OMVFilesystem object for the given mount point.
	 * @param dir The filesystem path prefix.
	 * @return The class object or NULL on failure.
	 */
	public static function getFilesystemByDir($dir) {
		$result = NULL;
		foreach(file("/proc/mounts") as $rowk => $rowv) {
			// Parse command output:
			// rootfs / rootfs rw 0 0
			// none /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0
			// none /proc proc rw,nosuid,nodev,noexec,relatime 0 0
			// tmpfs /dev/shm tmpfs rw,nosuid,nodev,relatime 0 0
			// /dev/sde1 /home/ftp/data ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			// /dev/sde1 /media/46607e80-dc8e-427e-9939-7909985dfe5b ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			$matches = explode(" ", $rowv);
			if($matches[1] == $dir) {
				$result = new OMVFilesystem($matches[0]);
				break;
			}
		}
		return $result;
	}

	/**
	 * Get the OMVFilesystem object for the given mount point.
	 * @param name The name of mounted file system.
	 * @return The class object or NULL on failure.
	 */
	public static function getFilesystemByName($name) {
		$result = NULL;
		foreach(file("/proc/mounts") as $rowk => $rowv) {
			// Parse command output:
			// rootfs / rootfs rw 0 0
			// none /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0
			// none /proc proc rw,nosuid,nodev,noexec,relatime 0 0
			// tmpfs /dev/shm tmpfs rw,nosuid,nodev,relatime 0 0
			// /dev/sde1 /home/ftp/data ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			// /dev/sde1 /media/46607e80-dc8e-427e-9939-7909985dfe5b ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			$matches = explode(" ", $rowv);
			if($matches[0] == $name) {
				$result = new OMVFilesystem($matches[0]);
				break;
			}
		}
		return $result;
	}
}

/**
 * Class implementing generic block devices.
 * @ingroup api
 */
interface OMVIBlockDevice {
	/**
	 * Checks if the device exists.
	 * @return TRUE if the device exists, otherwise FALSE.
	 */
	function exists();

	/**
	 * Get the device path, e.g. /dev/sda.
	 * @return The device path.
	 */
	function getDeviceFile();

	/**
	 * Get the device name, e.g. sda or hdb.
	 * @return The device name.
	 */
	function getDeviceName();

	/**
	 * Get the size of the device in bytes.
	 * @return The size of the device in bytes, otherwise FALSE.
	 */
	function getSize();

	/**
	 * Get the blocksize of the device in bytes.
	 * @return The blocksize of the device in bytes, otherwise FALSE.
	 */
	function getBlockSize();

	/**
	 * Get the sectorsize of the device in bytes.
	 * @return The sectorsize of the device in bytes, otherwise FALSE.
	 */
	function getSectorSize();

	/**
	 * Get the description of the device.
	 * @return The device description.
	 */
	function getDescription();
}

/**
 * Abstract class implementing generic block devices.
 * @ingroup api
 */
abstract class OMVBlockDeviceAbstract extends OMVObject implements
  OMVIBlockDevice {
	protected $deviceFile = FALSE;
	protected $size = FALSE;
	protected $blockSize = FALSE;
	protected $sectorSize = FALSE;
	private $errStr = "";

	/**
	 * Constructor
	 * @param deviceFile The device file.
	 */
	public function __construct($deviceFile) {
		$this->deviceFile = $deviceFile;
	}

	/**
	 * Get the error description of the last occured error.
	 * @return The error description of the last error if set.
	 */
	public function getError() {
		return implode("\n", $this->errStr);
	}

	/**
	 * See interface definition.
	 */
	public function exists() {
		return is_block($this->getDeviceFile());
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceFile() {
		return $this->deviceFile;
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceName() {
		// Get the device file and extract the name, e.g. /dev/sda => sda.
		$deviceName = str_replace("/dev/", "", $this->getDeviceFile());
		return $deviceName;
	}

	/**
	 * See interface definition.
	 */
	public function getSize() {
		return $this->size;
	}

	/**
	 * See interface definition.
	 */
	public function getBlockSize() {
		return $this->blockSize;
	}

	/**
	 * See interface definition.
	 */
	public function getSectorSize() {
		return $this->sectorSize;
	}
}

/**
 * Abstract class implementing generic storage devices.
 * @ingroup api
 */
abstract class OMVStorageDeviceAbstract extends OMVBlockDeviceAbstract {
	protected $deviceFileById = FALSE;
	protected $model = FALSE;
	protected $vendor = FALSE;
	protected $udevInfo = FALSE;
	protected $smartDeviceType = "";

	/**
	 * Constructor
	 * @param deviceFile The device file, e.g. /dev/sdb or /dev/md1 or
	 * /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6.
	 */
	public function __construct($deviceFile) {
		if(preg_match("/^\/dev\/disk\/by-id\/.+$/", $deviceFile)) {
			$this->deviceFileById = $deviceFile;
		} else {
			$this->deviceFile = $deviceFile;
		}
	}

	/**
	 * See interface definition.
	 */
	public function exists() {
		return is_block(!$this->deviceFile ? $this->deviceFileById :
		  $this->deviceFile);
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceFile() {
		if(!$this->deviceFile)
			$this->deviceFile = realpath($this->deviceFileById);
		return $this->deviceFile;
	}

	/**
	 * Get the device path by ID, e.g.
	 * \li /dev/disk/by-id/wwn-0x5000cca211cc703c
	 * \li /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML
	 * @return The device path (/dev/disk/by-id/xxx) if available,
	 *   otherwise /dev/xxx will be returned. In case of an error FALSE
	 *   will be returned.
	 */
	public function getDeviceFileById() {
		if(!$this->deviceFileById) {
			// Sort in reverse order, thus SCSI/SATA entries are listed
			// on top.
			$cmd = sprintf("export LANG=C; test -d '/dev/disk/by-id' && ".
			  "ls -ogr '/dev/disk/by-id/' | grep '/%s$' | ".
			  "awk '{print $7}'", $this->getDeviceName());
			@OMVUtil::exec($cmd, $output, $result);
			// Command output:
			// ata-VMware_Virtual_IDE_Hard_Drive_00000000000000000001
			// scsi-SATA_VMware_Virtual_00000000000000000001
			if(($result !== 0) || empty($output)) {
				$this->deviceFileById = $this->deviceFile;
			} else {
				// Use first found entry.
				$this->deviceFileById = sprintf("/dev/disk/by-id/%s",
				  $output[0]);
			}
		}
		return $this->deviceFileById;
	}

	/**
	 * Get the size of the device in bytes.
	 * @return The size (64bit) of the device in bytes as string.
	 */
	public function getSize() {
		if($this->size === FALSE) {
			$cmd = sprintf("export LANG=C; blockdev --getsize64 %s",
			  escapeshellarg($this->getDeviceFile()));
			@OMVUtil::exec($cmd, $output, $result);
			if($result !== 0)
				return FALSE;
			$this->size = $output[0];
		}
		return $this->size;
	}

	/**
	 * Get the blocksize of the device in bytes.
	 * @return The blocksize of the device in bytes, otherwise FALSE.
	 */
	public function getBlockSize() {
		if($this->blockSize === FALSE) {
			$cmd = sprintf("export LANG=C; blockdev --getbsz %s",
			  escapeshellarg($this->getDeviceFile()));
			@OMVUtil::exec($cmd, $output, $result);
			if($result !== 0) {
				$this->errStr = $output;
				return FALSE;
			}
			$this->blockSize = intval($output[0]);
		}
		return $this->blockSize;
	}

	/**
	 * Get the sectorsize of the device in bytes.
	 * @return The sectorsize of the device in bytes, otherwise FALSE.
	 */
	public function getSectorSize() {
		if($this->sectorSize === FALSE) {
			$cmd = sprintf("export LANG=C; blockdev --getss %s",
			  escapeshellarg($this->getDeviceFile()));
			@OMVUtil::exec($cmd, $output, $result);
			if($result !== 0) {
				$this->errStr = $output;
				return FALSE;
			}
			$this->sectorSize = intval($output[0]);
		}
		return $this->sectorSize;
	}

	/**
	 * Get the device model.
	 * @return The device model, otherwise an empty string.
	 */
	public function getModel() {
		if($this->model === FALSE) {
			$filename = sprintf("/sys/block/%s/device/model",
			  $this->getDeviceName());
			if(file_exists($filename)) {
				$this->model = trim(file_get_contents($filename));
			} else {
				$this->model = "";
			}
		}
		return $this->model;
	}

	/**
	 * Get the device vendor.
	 * @return The device vendor, otherwise an empty string.
	 */
	public function getVendor() {
		if($this->vendor === FALSE) {
			$filename = sprintf("/sys/block/%s/device/vendor",
			  $this->getDeviceName());
			if(file_exists($filename)) {
				$this->vendor = trim(file_get_contents($filename));
			} else {
				$this->vendor = "";
			}
		}
		return $this->vendor;
	}

	/**
	 * Get the device serial number.
	 * @return The device serial number, otherwise an empty string.
	 */
	public function getSerialNumber() {
		if(FALSE === ($property = $this->getUdevProperty("ID_SERIAL_SHORT")))
			return "";
		return str_replace("_", " ", $property);
	}

	/**
	 * See interface definition.
	 */
	public function getDescription() {
		$model = $this->getModel();
		return sprintf("%s [%s, %s]", !empty($model) ? $model : gettext("n/a"),
		  $this->getDeviceFile(), binary_format($this->getSize()));
	}

	/**
	 * Check if the given device is a hardware/software RAID device.
	 * @return TRUE if the device is a hardware/software RAID, otherwise FALSE.
	 */
	public function isRaid() {
		return FALSE;
	}

	/**
	 * Check if the given device is a USB device.
	 * @return TRUE if the device is a connected via USB, otherwise FALSE.
	 */
	public function isUsb() {
		if(FALSE === ($property = $this->getUdevProperty("ID_BUS")))
			return FALSE;
		return ("usb" === $property);
	}

	/**
	 * Queries the udev database for device information stored in the udev
	 * database.
	 * @param id The name of the property, e.g. ID_VENDOR, ID_MODEL or
	 *   ID_SERIAL_SHORT.
	 * @return The requested property, otherwise FALSE.
	 */
	protected function getUdevProperty($id) {
		if($this->udevInfo === FALSE) {
			$cmd = sprintf("export LANG=C; sudo udevadm info ".
			  "--query=property --name=%s", escapeshellarg(
			  $this->getDeviceFile()));
			@OMVUtil::exec($cmd, $output, $result);
			if($result !== 0) {
				$this->errStr = $output;
				return FALSE;
			}
			// Parse output:
			// UDEV_LOG=3
			// DEVPATH=/devices/pci0000:00/0000:00:10.0/host2/target2:0:1/2:0:1:0/block/sdb
			// MAJOR=8
			// MINOR=16
			// DEVNAME=/dev/sdb
			// DEVTYPE=disk
			// SUBSYSTEM=block
			// ID_SCSI=1
			// ID_VENDOR=VMware_
			// ID_VENDOR_ENC=VMware\x2c\x20
			// ID_MODEL=VMware_Virtual_S
			// ID_MODEL_ENC=VMware\x20Virtual\x20S
			foreach($output as $row) {
				$data = explode("=", $row);
				if(empty($data))
					continue;
				$this->udevInfo[$data[0]] = $data[1];
			}
		}
		if(!array_key_exists($id, $this->udevInfo))
			return FALSE;
		return $this->udevInfo[$id];
	}

	/**
	 * Set the device type required by the smartctl command.
	 * @return Returns the device type.
	 */
	public function setSmartDeviceType($type) {
		$this->smartDeviceType = $type;
		return $this->smartDeviceType;
	}

	/**
	 * Identify the device type required by the smartctl command.
	 * @return Returns the identified device type or an empty string.
	 */
	public function getSmartDeviceType() {
		return $this->smartDeviceType;
	}
}

/**
 * The generic class that represents a storage device backend.
 * @ingroup api
 */
abstract class OMVStorageDeviceBackendAbstract extends OMVObject {
	/**
	 * Get the type of the storage device backend which is implemented
	 * by this class.
	 */
	abstract function getType();

	/**
	 * Get a list of devices of the given storage device backend.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	abstract function enumerate();

	/**
	 * Get a list of all devices that are used by devices of this
	 * storage device backend.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	function enumerateSlaves() { return array(); }

	/**
	 * Check whether the given device is represented by this storage
	 * device backend.
	 * @param deviceFile The device file, e.g. <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @return TRUE if successful, otherwise FALSE.
	 */
	function isTypeOf($deviceFile) { return FALSE; }

	/**
	 * Get an object of the class that represents and implements a device
	 * of this storage device backend.
	 * @param args The arguments to the class constructor.
	 * @return The object of the class implementing the given device type,
	 *   otherwise NULL.
	 */
	function getImpl($args) { return NULL; }

	/**
	 * Helper function to enumerate the devices represented by this storage
	 * device backend via the /proc filesystem.
	 * @param regex The regular expression used to identify the devices
	 *   represented by this storage backend.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	final protected function enumerateProcFs($regex) {
		$result = array();
		$regex = sprintf('/^\s+(\d+)\s+(\d+)\s+(\d+)\s+(%s)$/', $regex);
		foreach(file("/proc/partitions") as $outputk => $outputv) {
			if(1 !== preg_match($regex, $outputv, $matches))
				continue;
			$result[] = sprintf("/dev/%s", $matches[4]);
		}
		return $result;
	}

	/**
	 * Helper function to check whether the given device is represented by
	 * this storage device backend.
	 * @param deviceFile The device file.
	 * @param regex The regular expression used to identify the devices
	 *   represented by this storage backend.
	 */
	final protected function isTypeOfByName($deviceFile, $regex) {
		// If the device file looks like /dev/disk/by-(id|label|path|uuid)/*
		// then it is necessary to get the /dev/xxx equivalent.
		if(1 == preg_match("/^\/dev\/disk\/by-.+$/", $deviceFile))
			$deviceFile = realpath($deviceFile);
		// Extract the device name, e.g.:
		// - /dev/sdb => sdb
		// - /dev/cciss/c0d0 => cciss/c0d0
		// - /dev/mapper/vg0-lv0 => mapper/vg0-lv0
		// - /dev/loop1 => loop1
		$deviceName = str_replace("/dev/", "", $deviceFile);
		if(1 == preg_match("/^{$regex}$/i", $deviceName))
			return TRUE;
		return FALSE;
	}
}

class OMVStorageDeviceBackendLoopDevice extends
  OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_LOOPDEVICE;
	}

	function enumerate() {
		$regex = $GLOBALS['OMV_REGEX_DEVNAME_LOOPDEVICE'];
		return $this->enumerateProcFs($regex);
	}

	function isTypeOf($deviceFile) {
		$regex = $GLOBALS['OMV_REGEX_DEVNAME_LOOPDEVICE'];
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVLoopDevice($args);
	}
}

class OMVStorageDeviceBackendDeviceMapper extends
  OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_DEVICEMAPPER;
	}

	function enumerate() {
		$regex = $GLOBALS['OMV_REGEX_DEVNAME_DEVICEMAPPER'];
		return $this->enumerateProcFs($regex);
	}

	function enumerateSlaves() {
		$devs = $this->enumerate();
		if(FALSE === $devs)
			return FALSE;
		$result = array();
		foreach($devs as $devk => $devv) {
			$object = new OMVDeviceMapperDevice($devv);
			if(!$object->exists())
				continue;
			$result = array_merge($result, $object->getSlaves());
		}
		return $result;
	}

	function isTypeOf($deviceFile) {
		$regex = $GLOBALS['OMV_REGEX_DEVNAME_DEVICEMAPPER'];
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVDeviceMapperDevice($args);
	}
}

class OMVStorageDeviceBackendLVM extends OMVStorageDeviceBackendDeviceMapper {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_LVM;
	}

	function enumerate() {
		$devs = parent::enumerate();
		if(FALSE === $devs)
			return FALSE;
		$result = array();
		foreach($devs as $devk => $devv) {
			$object = new OMVDeviceMapperDevice($devv);
			if(!$object->exists())
				continue;
			// Build a device file name that contains the VG and LV name.
			$deviceFile = sprintf("/dev/mapper/%s", $object->getName());
			// Check if the given device is a LVM logical volume.
			$object = new OMVLvmLogicalVolume($deviceFile);
			if(!$object->exists())
				continue;
			$result[] = $object->getDeviceFile();
		}
		return $result;
	}

	function enumerateSlaves() {
		$devs = $this->enumerate();
		if(FALSE === $devs)
			return FALSE;
		$result = array();
		foreach($devs as $devk => $devv) {
			// Process LVM logical volumes.
			$object = new OMVLvmLogicalVolume($devv);
			if($object->exists())
				$result = array_merge($result, $object->getSlaves());
		}
		return $result;
	}

	function isTypeOf($deviceFile) {
		if(FALSE === ($result = parent::isTypeOf($deviceFile)))
			return FALSE;
		// Do some more detailed checks.
		$object = new OMVLvmLogicalVolume($deviceFile);
		return $object->exists();
	}

	function getImpl($args) {
		return new OMVLvmLogicalVolume($args);
	}
}

class OMVStorageDeviceBackendMdadm extends OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_SOFTWARERAID;
	}

	function enumerate() {
		$regex = $GLOBALS['OMV_REGEX_DEVNAME_SOFTWARERAID'];
		return $this->enumerateProcFs($regex);
	}

	function enumerateSlaves() {
		$devs = $this->enumerate();
		if(FALSE === $devs)
			return FALSE;
		$result = array();
		foreach($devs as $devk => $devv) {
			$object = new OMVSoftwareRaidDevice($devv);
			if(!$object->exists())
				continue;
			$result = array_merge($result, $object->getSlaves());
		}
		return $result;
	}

	function isTypeOf($deviceFile) {
		$regex = $GLOBALS['OMV_REGEX_DEVNAME_SOFTWARERAID'];
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVSoftwareRaidDevice($args);
	}
}

class OMVStorageDeviceBackendHBA extends OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HBA;
	}

	function enumerate() {
		$regex = $GLOBALS['OMV_REGEX_DEVNAME_HARDWARERAID'];
		return $this->enumerateProcFs($regex);
	}

	function isTypeOf($deviceFile) {
		$regex = $GLOBALS['OMV_REGEX_DEVNAME_HARDWARERAID'];
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVHardwareRaidDevice($args);
	}
}

class OMVStorageDeviceBackendHDD extends OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HDD;
	}

	function enumerate() {
		$regex = $GLOBALS['OMV_REGEX_DEVNAME_HARDDISK'];
		return $this->enumerateProcFs($regex);
	}

	function isTypeOf($deviceFile) {
		$regex = $GLOBALS['OMV_REGEX_DEVNAME_HARDDISK'];
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVHardDiskDevice($args);
	}
}

/**
 * Helper class to get all existing devices of the given storage type.
 * @ingroup api
 */
class OMVStorageDevices extends OMVObject {
	static private $backends = array();

	/**
	 * Register a storage device backend.
	 * @param backend The storage device backend object to register.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	final static function registerBackend(OMVStorageDeviceBackendAbstract $backend) {
		if(!isset($backend))
			return FALSE;
		self::$backends[] = $backend;
		return TRUE;
	}

	/**
	 * Enumerate devices matching the given storage device type.
	 * @param type Defines the storage device type, e.g. hard disk, hard or
	 *   Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	public static function enumerate($type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		$result = array();
		foreach(self::$backends as $backendk => $backendv) {
			if(!($type & $backendv->getType()))
				continue;
			if(FALSE === ($devs = $backendv->enumerate()))
				return FALSE;
			$result = array_unique(array_merge($result, $devs));
		}
		return $result;
	}

	/**
	 * Enumerate all unused devices. This list contains all devices of the
	 * given type except the devices that are used by other storage devices
	 * (e.g. LVM as physical volume or a member of a Software RAID).
	 * @param type Defines the storage device type, e.g. hard disk, hard or
	 *   Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	final public function enumerateUnused($type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		// Append all available storage devices.
		if(FALSE === ($result = self::enumerate($type)))
			return FALSE;
		// Remove used devices.
		foreach(self::$backends as $backendk => $backendv) {
			if(!($type & $backendv->getType()))
				continue;
			if(FALSE === ($devs = $backendv->enumerateSlaves()))
				return FALSE;
			if(empty($devs))
				continue;
			$result = array_diff($result, $devs);
		}
		return $result;
	}

	/**
	 * Enumerate all used devices. The list contains all those devices that
	 * are used by the given storage devices, e.g. all members of a LVM
	 * or Software RAID.
	 * @param type Defines the storage device type, e.g. hard disk, hard or
	 *   Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	final public function enumerateUsed($type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		$result = array();
		// Remove used devices.
		foreach(self::$backends as $backendk => $backendv) {
			if(!($type & $backendv->getType()))
				continue;
			if(FALSE === ($devs = $backendv->enumerateSlaves()))
				return FALSE;
			$result = array_unique(array_merge($result, $devs));
		}
		return $result;
	}

	/**
	 * Check if the given device is used/consumed by another storage device.
	 * @param deviceFile The device file, e.g. <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @param type Defines the storage device type, e.g. hard disk, hard or
	 *   Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return TRUE if the given device is used/consumed by another storage
	 *   device, otherwise FALSE.
	 */
	final public function isUsed($deviceFile, $type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		$result = FALSE;
		foreach(self::$backends as $backendk => $backendv) {
			if(!($type & $backendv->getType()))
				continue;
			if(FALSE === ($devs = $backendv->enumerateSlaves()))
				return FALSE;
			if(in_array($deviceFile, $devs)) {
				$result = TRUE;
				break;
			}
		}
		return $result;
	}

	/**
	 * Get the object of the class which implements the given storage backend.
	 * @param deviceFile The device file, e.g. <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @return The object of the class implementing the given storage backend,
	 *   otherwise NULL.
	 */
	public static function getImpl($deviceFile) {
		$result = NULL;
		foreach(self::$backends as $backendk => $backendv) {
			if(FALSE === $backendv->isTypeOf($deviceFile))
				continue;
			// Create a new class object.
			$result = $backendv->getImpl($deviceFile);
			break;
		}
		return $result;
	}
}

/**
 * Factory to create objects of storage device classes.
 * @ingroup api
 */
class OMVStorageDeviceFactory extends OMVObject {
	/**
	 * Get the object of the class implementing the given device type.
	 * @param deviceFile The device file, e.g. <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @return The object of the class implementing the given device type,
	 *   otherwise NULL.
	 */
	public static function get($deviceFile) {
		return OMVStorageDevices::getImpl($deviceFile);
	}
}

/**
 * This class provides a simple interface to handle ATA/SATA and SCSI
 * hard disk devices. Try to use sysfs only to get the required informations,
 * otherwise the disk will spin-up if it is in sleep mode.
 * @ingroup api
 */
class OMVHardDiskDevice extends OMVStorageDeviceAbstract {
	protected $identityInfo = FALSE;
	protected $smartInfo = NULL;

	/**
	 * Refresh the cached informations.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		// Mark variables as dirty, thus the cached command output will be
		// refreshed when the class method is called the next time.
 		$this->identityInfo = FALSE;
		$this->smartInfo = NULL;
		return TRUE;
	}

	/**
	 * Get the temperature in °C (value only, no unit).
	 * @return The temperature value, otherwise FALSE.
	 */
	public function getTemperature() {
		if(FALSE === ($stats = $this->getSmartStats()))
			return FALSE;
		$temp = FALSE;
		$found = FALSE;
		foreach($stats['attributes'] as $statv) {
			switch($statv['id']) {
			case 194:
			case 231:
				// The value may look like: '43 (Lifetime Min/Max 16/61)'
				$regex = '/^(\d+)(\s+(.+))*$/';
				if(1 !== preg_match($regex, $statv['rawvalue'], $matches))
					continue;
				$temp = $matches[1];
				$found = TRUE;
				break;
			}
			if($found)
				break;
		}
		// If the SMART attributes are not present then it may be an older
		// SCSI device. Then the command output looks like:
		// Device: SEAGATE  ST336605LSUN36G  Version: 0238
		// Serial number: 3FP1J35V00007241EEC7
		// Device type: disk
		// Local Time is: Thu Apr  5 15:41:56 2012 CEST
		// Device supports SMART and is Enabled
		// Temperature Warning Enabled
		// SMART Health Status: OK
		//
		// Current Drive Temperature:     34 C
		// Drive Trip Temperature:        65 C
		// ...
		if(FALSE === $found) {
			foreach($this->smartInfo as $smartInfov) {
				$regex = '/^Current Drive Temperature:\s+(\d+)\s+C$/';
				if(1 == preg_match($regex, $smartInfov, $matches)) {
					$temp = $matches[1];
					break;
				}
			}
		}
		return $temp;
	}

	/**
	 * Get the device firmware version.
	 * @return The device firmware version, otherwise an empty string.
	 */
	public function getFirmwareVersion() {
		if(FALSE === ($ii = $this->getIdentityInfo()))
			return "";
		return $ii['firmwareversion'];
	}

	/**
	 * Get the S.M.A.R.T. informations for the given disk device.
	 * @return All S.M.A.R.T. information for the given device,
	 * otherwise FALSE.
	 */
	public function getSmartStats() {
		if(!$this->exists())
			return FALSE;
		// S.M.A.R.T. informations already cached?
		if(is_null($this->smartInfo)) {
			$cmdArgs = array();
			$cmdArgs[] = sprintf("-a %s", escapeshellarg(
			  $this->getDeviceFile()));
			if("" !== ($deviceType = $this->getSmartDeviceType())) {
				$cmdArgs[] = sprintf("-d %s", $deviceType);
			}
			$cmd = sprintf("export LANG=C; smartctl %s", implode(" ",
			  $cmdArgs));
			@OMVUtil::exec($cmd, $output, $result);
			// Bit 0: Command line did not parse
			// Bit 1: Device open failed, or device did not return an
			//        IDENTIFY DEVICE structure
			if(($result !== 0) && (($result & 0) || ($result & 1)))
				return FALSE;
			$this->smartInfo = $output;
		}
		// Set default values
		$attributes = array();
		$selfTestLogs = array();
		$information = $information = array(
			"devicemodel" => "",
			"serialnumber" => "",
			"firmwareversion" => ""
		);
		// Process command output
		foreach($this->smartInfo as $smartInfov) {
			// SMART Attributes Data Structure
			// Parse command output:
			// ID# ATTRIBUTE_NAME          FLAG     VALUE WORST THRESH TYPE      UPDATED  WHEN_FAILED RAW_VALUE
			//   1 Raw_Read_Error_Rate     0x000b   100   100   016    Pre-fail  Always       -       0
			//   8 Seek_Time_Performance   0x0005   100   100   020    Pre-fail  Offline      -       0
			// 194 Temperature_Celsius     0x0002   139   139   000    Old_age   Always       -       43 (Lifetime Min/Max 16/61)
			$regex = '/^\s*(\d+)\s+(\S+)\s+(\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s+'.
			  '(\S+)\s+(\S+)\s+(\S+)\s+(.+)$/';
			if(1 == preg_match($regex, $smartInfov, $matches)) {
				$attributes[] = array(
					"id" => $matches[1],
					"attrname" => $matches[2],
					"flag" => $matches[3],
					"value" => $matches[4],
					"worst" => $matches[5],
					"treshold" => $matches[6],
					"type" => $matches[7],
					"updated" => $matches[8],
					"whenfailed" => $matches[9],
					"rawvalue" => $matches[10]
				);
			}
			// SMART Self-test log structure
			// Parse command output:
			// Num  Test_Description    Status                  Remaining  LifeTime(hours)  LBA_of_first_error
			// # 1  Extended offline    Completed: read failure       90%       670         57217755
			// # 2  Short captive       Interrupted (host reset)      80%      1392         -
			$regex = '/^# (\d+)\s+(Short offline|Extended offline|'.
			  'Short captive|Extended captive)\s+(.+)\s+(\d+)%\s+(\d+)'.
			  '\s+(.+)$/';
			if(1 == preg_match($regex, $smartInfov, $matches)) {
				$selfTestLogs[] = array(
					"num" => $matches[1],
					"description" => $matches[2],
					"status" => $matches[3],
					"remaining" => $matches[4],
					"lifetime" => $matches[5],
					"lbaoffirsterror" => $matches[6]
				);
			}
		}
		// INFORMATION SECTION
		// Parse command output:
		// === START OF INFORMATION SECTION ===
		// Model Family:     Hitachi Deskstar T7K500
		// Device Model:     Hitachi HDT725032VLA360
		// Serial Number:    VFD200R2CWBTLL
		// Firmware Version: V54OA52A
		// User Capacity:    320.072.933.376 bytes
		$output = implode("|", $this->smartInfo);
		$regex = '/^.+\|Device Model:\s+(.+)\|Serial Number:\s+(.+)'.
		  '\|Firmware Version:\s+(.+)\|User Capacity:\s+(.+) bytes\|.+$/';
		if(1 == preg_match($regex, $output, $matches)) {
			$this->identityInfo = $information = array(
				"devicemodel" => $matches[1],
				"serialnumber" => $matches[2],
				"firmwareversion" => $matches[3]
			);
		}
		return array(
			"attributes" => $attributes,
			"selftestlogs" => $selfTestLogs,
			"information" => $information
		);
	}

	/**
	 * Get identity information for the device.
	 * @return The identity information for the device if
	 * successful, otherwise FALSE.
	 * array(
	 *   devicemodel => xxx,
	 *   serialnumber => xxx,
	 *   firmwareversion => xxx
	 * )
	 */
	private function getIdentityInfo() {
		if(!$this->exists())
			return FALSE;
		if($this->identityInfo === FALSE) {
			$cmdArgs = array();
			$cmdArgs[] = sprintf("-i %s", escapeshellarg(
			  $this->getDeviceFile()));
			if("" !== ($deviceType = $this->getSmartDeviceType())) {
				$cmdArgs[] = sprintf("-d %s", $deviceType);
			}
			$cmd = sprintf("export LANG=C; smartctl %s", implode(" ",
			  $cmdArgs));
			@OMVUtil::exec($cmd, $output, $result);
			if($result !== 0)
				return FALSE;
			$output = implode("|", $output);
			$regex = '/^.+\|Device Model:\s+(.+)\|Serial Number:\s+(.+)'.
			  '\|Firmware Version:\s+(.+)\|User Capacity:\s+(.+) bytes\|.+$/';
			if(0 >= preg_match($regex, $output, $matches))
				return FALSE;
			$this->identityInfo = array(
				"devicemodel" => $matches[1],
				"serialnumber" => $matches[2],
				"firmwareversion" => $matches[3]
			);
		}
		return $this->identityInfo;
	}

	/**
	 * Identify the device type required by the smartctl command.
	 * @return Returns the identified device type or an empty string.
	 */
	public function getSmartDeviceType() {
		$type = parent::getSmartDeviceType();
		if(empty($type)) {
			// TODO: The device type identification must be improved.
			if(TRUE === $this->isUsb()) {
				$type = $this->setSmartDeviceType("sat");
			}
		}
		return $type;
	}
}

/**
 * This class provides a simple interface to handle hardware RAID devices,
 * e.g. HP's Smart Array hardware RAID controller.
 * @ingroup api
 */
class OMVHardwareRaidDevice extends OMVStorageDeviceAbstract {
	/**
	 * See interface definition.
	 */
	public function getDeviceName() {
		// Get the device name and convert '/' character to '!', e.g.
		// cciss/c0d0 => cciss!c0d0.
		return strtr(OMVStorageDeviceAbstract::getDeviceName(), "/", "!");
	}

	/**
	 * See abstract class definition.
	 */
	public function isRaid() {
		return TRUE;
	}

	/**
	 * Identify the device type required by the smartctl command.
	 * @return Returns the identified device type or an empty string.
	 */
	public function getSmartDeviceType() {
		$type = parent::getSmartDeviceType();
		// Note, 'auto' attempts to guess the device type from the device
		// name or from controller type info provided by the operating system
		// or from a matching USB ID entry in the drive database. Thus we
		// have to do the job ourself for various hardware RAID devices.
		if(empty($type) || ("auto" === $type)) {
			if(1 == preg_match("/^cciss!c(\d)d(\d)$/", $this->getDeviceName(),
			  $matches)) {
				$type = $this->setSmartDeviceType(sprintf("cciss,%s",
				  $matches[2]));
			}
		}
		return $type;
	}
}

/**
 * This class provides a simple interface to handle software RAID devices.
 * @ingroup api
 */
class OMVSoftwareRaidDevice extends OMVStorageDeviceAbstract {
	protected $name = "";
	protected $level = FALSE;
	protected $devices = array();
	protected $numDevices = FALSE;
	protected $uuid = FALSE;
	protected $state = "unknown";
	protected $hasWriteIntentBitmap = FALSE;
	protected $details = array();
	private $dataCached = FALSE;

	/**
	 * Get the software RAID device detailed informations.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if($this->dataCached !== FALSE)
			return TRUE;

		// ARRAY /dev/md0 level=raid5 num-devices=3 metadata=1.2 name=xxxx:0 UUID=a4266bf7:c671b343:c3d6e535:ca455e37
		//    devices=/dev/sdb,/dev/sdc,/dev/sdd
		$cmd = sprintf("export LANG=C; mdadm --detail --brief " .
		  "--verbose %s", escapeshellarg($this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;

		$attributes = array(
			"name" => "",
			"level" => "",
			"num-devices" => -1,
			"uuid" => "",
			"devices" => ""
		);
		$output = explode(" ", implode(" ", $output));
		foreach($output as $outputk => &$outputv) {
			$keyValue = explode("=", $outputv);
			if(count($keyValue) != 2)
				continue;
			$attributes[strtolower($keyValue[0])] = $keyValue[1];
		}

		$this->name = $attributes['name'];
		$this->level = $attributes['level'];
		$this->numDevices = intval($attributes['num-devices']);
		$this->uuid = $attributes['uuid'];
		$this->devices = explode(",", $attributes['devices']);

		// Get more informations.
		$cmd = sprintf("export LANG=C; mdadm --detail %s",
		  escapeshellarg($this->getDeviceFile()));
		unset($output);
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;

		// Store the whole command output.
		array_shift($output);
		$this->details = $output;

		// Parse command output:
		// /dev/md0:
		//         Version : 1.2
		//   Creation Time : Tue Dec 25 21:58:20 2012
		//      Raid Level : raid5
		//      Array Size : 207872 (203.03 MiB 212.86 MB)
		//   Used Dev Size : 103936 (101.52 MiB 106.43 MB)
		//    Raid Devices : 3
		//   Total Devices : 3
		//     Persistence : Superblock is persistent
		// 
		//   Intent Bitmap : Internal
		// 
		//     Update Time : Tue Dec 25 22:31:32 2012
		//           State : active
		//  Active Devices : 3
		// Working Devices : 3
		//  Failed Devices : 0
		//   Spare Devices : 0
		// 
		//          Layout : left-symmetric
		//      Chunk Size : 512K
		// 
		//            Name : dhcppc2:0  (local to host dhcppc2)
		//            UUID : 9d85a4f6:afff2cb6:b8a5f4dc:75f3cfd3
		//          Events : 37
		// 
		//     Number   Major   Minor   RaidDevice State
		//        0       8       16        0      active sync   /dev/sdb
		//        1       8       48        1      active sync   /dev/sdd
		//        2       8       64        2      active sync   /dev/sde
		foreach($output as $outputk => $outputv) {
			if(empty($outputv))
				continue;
			$parts = explode(":", $outputv);
			if(empty($parts))
				continue;
			$parts = array_map("trim", $parts);
			switch(strtolower($parts[0])) {
			case "state":
				$this->state = $parts[1];
				break;
			case "intent bitmap":
				$this->hasWriteIntentBitmap = (0 == strcasecmp($parts[1],
				  "Internal")) ? TRUE : FALSE;
				break;
			}
		}

		// Set flag to mark informations has been successfully read
		$this->dataCached = TRUE;

		return TRUE;
	}

	/**
	 * Refresh the cached informations.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		if($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * See interface definition.
	 */
	public function exists() {
		if($this->getData() === FALSE)
			return FALSE;
		return ($this->level !== FALSE);
	}

	/**
	 * Get the array name.
	 * @return The array name, otherwise FALSE.
	 */
	public function getName() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->name;
	}

	/**
	 * Get the level of the array.
	 * @return The level of the array, otherwise FALSE.
	 */
	public function getLevel() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->level;
	}

	/**
	 * Get the number of active devices in the array.
	 * @return The number of active devices in the array, otherwise FALSE.
	 */
	public function getNumDevices() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->numDevices;
	}

	/**
	 * Get the UUID of the array.
	 * @return The UUID of the array, otherwise FALSE.
	 */
	public function getUuid() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->uuid;
	}

	/**
	 * See abstract class definition.
	 */
	public function getSerialNumber() {
		return $this->getUuid();
	}

	/**
	 * Get the list of device files used in the array.
	 * @return A list of device files used in the array, otherwise FALSE.
	 */
	public function getSlaves() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->devices;
	}

	public function getState() {
		if($this->getData() === FALSE)
			return FALSE;
		// Parse command output:
		// Personalities : [linear] [multipath] [raid0] [raid1] [raid6] [raid5] [raid4] [raid10]
		// md1 : active raid5 sde[2] sdg[1] sdf[0]
		//       207872 blocks super 1.2 level 5, 512k chunk, algorithm 2 [3/3] [UUU]
		//       [=========>...........]  resync = 45.0% (47488/103936) finish=0.0min speed=47488K/sec
		//
		// md0 : active (auto-read-only) raid5 sdd[2] sdc[1] sdb[0]
		//       207872 blocks super 1.2 level 5, 512k chunk, algorithm 2 [3/3] [UUU]
		//       	resync=PENDING
		//
		// md0 : active raid5 sdf[4] sde[3] sdd[2] sdc[1] sdb[0]
		//       311808 blocks super 1.2 level 5, 512k chunk, algorithm 2 [5/5] [UUUUU]
		//       [====>................]  reshape = 20.5% (21504/103936) finish=0.0min speed=21504K/sec
		//
		// md0 : inactive sdb[0](S) sdc[2](S) sdd[1](S)
		//       311808 blocks super 1.2
		//
		// md0 : active raid0 sdd[0] sde[1]
		//       311296 blocks super 1.2 512k chunks
		//
		// unused devices: <none>
		$regex = sprintf('/^(%s\s*:.*)^\s*$/ims', $this->getDeviceName());
		if(1 !== preg_match($regex, file_get_contents("/proc/mdstat"),
		  $matches))
			return FALSE;
		$mdstat = $matches[1];
		// Extract the resync state.
		$resyncState = "unknown";
		if(1 == preg_match("/^.+(resync|reshape)\s*=\s*(.+)$/im", $mdstat,
		  $matches)) {
			$resyncState = trim($matches[2]);
		}
		$state = $this->state;
		if((FALSE !== stristr($state, "resyncing")) ||
		  (FALSE !== stristr($state, "recovering"))) {
			$state = sprintf("%s (%s)", $state, $resyncState);
		} else if(FALSE !== stristr($mdstat, "resync")) {
			$state = sprintf("%s, resyncing (%s)", $state, $resyncState);
		}
		return $state;
	}

	/**
	 * Get detail of the md device.
	 * @return The detail of the md device, otherwise FALSE.
	 */
	public function getDetail() {
		if($this->getData() === FALSE)
			return FALSE;
		return implode("\n", $this->details);
	}

	/**
	 * See interface definition.
	 */
	public function getDescription() {
		return sprintf("Software RAID %s [%s, %s, %s]", $this->getName(),
		  $this->getDeviceFile(), $this->getLevel(),
		  binary_format($this->getSize()));
	}

	/**
	 * See abstract class definition.
	 */
	public function isRaid() {
		return TRUE;
	}

	/**
	 * Check whether the RAID device has write-intent bitmap enabled.
	 * Note, only 'internal' bitmaps are detected and reported.
	 * @see https://raid.wiki.kernel.org/index.php/Write-intent_bitmap
	 * @return TRUE if write-intent bitmap is enabled, otherwise FALSE.
	 */
	public function hasWriteIntentBitmap() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->hasWriteIntentBitmap;
	}
}

/**
 * This class provides a simple interface to handle device-mapper devices.
 * See http://en.wikipedia.org/wiki/Device_mapper for more details.
 * @ingroup api
 */
class OMVDeviceMapperDevice extends OMVStorageDeviceAbstract {
	/**
	 * Get the name of the device mapper device.
	 * @return The name of the device mapper device.
	 */
	public function getName() {
		$path = sprintf("/sys/block/%s/dm/name", $this->getDeviceName());
		if(!file_exists($path))
			return FALSE;
		return trim(file_get_contents($path));
	}

	/**
	 * Get the UUID of the device mapper device.
	 * @return The UUID of the device mapper device, otherwise FALSE.
	 */
	public function getUuid() {
		$path = sprintf("/sys/block/%s/dm/uuid", $this->getDeviceName());
		if(!file_exists($path))
			return FALSE;
		return trim(file_get_contents($path));
	}

	/**
	 * Get the slave devices of the device mapper device.
	 * @return An array of device files, otherwise FALSE.
	 */
	public function getSlaves() {
		$path = sprintf("/sys/block/%s/slaves", $this->getDeviceName());
		if(!file_exists($path))
			return FALSE;
		$result = array();
		$dir = new DirectoryIterator($path);
		foreach($dir as $item) {
			if($item->isDot() || !$item->isFile())
				continue;
			$result[] = sprintf("/dev/%s", $item->getFilename());
		}
		return $result;
	}
}

/**
 * This class provides a simple interface to handle loop devices.
 * @ingroup api
 */
class OMVLoopDevice extends OMVStorageDeviceAbstract {
	// Nothing to do here.
}

/**
 * Helper class that provides various functions regarding the systems network
 * configuration.
 * @ingroup api
 */
class OMVNetwork extends OMVObject {
	/**
	 * Get the systems host name.
	 * @param options Additional command parameters
	 * @return The host name if successful, otherwise FALSE.
	 */
	public static function getHostname($options) {
		$cmd = sprintf("export LANG=C; hostname %s", $options);
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		return $output[0];
	}

	/**
	 * Get the systems domain name.
	 * @return The domain name if successful, otherwise FALSE.
	 */
	public static function getDomain() {
		$cmd = "export LANG=C; hostname -d";
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		return $output[0];
	}
}

/**
 * Helper class to get all existing Linux network interfaces.
 * @ingroup api
 */
class OMVNetworkInterfaces extends OMVObject {
	/**
	 * Get a list of all network interfaces.
	 * @return A list of network interfaces, otherwise FALSE.
	 */
	public static function get() {
		$cmd = "ls /sys/class/net | grep -E '^[a-z]+[0-9]+|lo$'";
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		return $output;
	}

	/**
	 * Get a list of network interfaces by type.
	 * @param type The type of interface, e.g. eth, lo or bond.
	 * @return A list of network interfaces, otherwise FALSE.
	 */
	public static function getByType($type) {
		$cmd = sprintf("ls /sys/class/net | grep %s", escapeshellarg(
		  $type));
		OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		return $output;
	}
}

/**
 * This class provides a simple interface to handle Linux network interfaces.
 * @ingroup api
 */
class OMVNetworkInterface extends OMVObject {
	protected $name = "";
	protected $ifconfig = null;
	protected $ip = null;
	protected $ethtool = null;
	protected $regex = array(
		"ipv4" => '\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}',
		"ipv6" => '[a-f0-9:]+',
		"ipv6cidr" => '([a-f0-9:]+)\/(\d{1,2})',
		"state" => 'UP|DOWN|UNKNOWN'
	);

	/**
	 * Constructor
	 * @param name The network interface name, e.g. eth0, ethx, ...
	 */
	public function __construct($name) {
		$this->name = $name;
	}

	/**
	 * Get the network interface configuration
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData($type) {
		$result = FALSE;
		switch($type) {
		case "ifconfig":
			if(!is_null($this->ifconfig))
				return TRUE;
			$cmd = sprintf("export LANG=C; ifconfig %s",
			  $this->getDeviceName());
			@OMVUtil::exec($cmd, $output, $result);
			if(($result !== 0) || empty($output[0]))
				return FALSE;
			$this->ifconfig = implode("|", $output);
			$result = TRUE;
			break;
		case "ip":
			if(!is_null($this->ip))
				return TRUE;
			$cmd = sprintf("export LANG=C; ip addr show dev %s",
			  $this->getDeviceName());
			@OMVUtil::exec($cmd, $output, $result);
			if(($result !== 0) || empty($output[0]))
				return FALSE;
			foreach($output as $outputk => &$outputv) {
				$outputv = trim($outputv);
			}
			$this->ip = implode("|", $output);
			$result = TRUE;
			break;
		case "ethtool":
			if(!is_null($this->ethtool))
				return TRUE;
			$cmd = sprintf("export LANG=C; ethtool %s",
			  $this->getDeviceName());
			@OMVUtil::exec($cmd, $output, $result);
			if(($result !== 0) || empty($output[0]))
				return FALSE;
			foreach($output as $outputk => &$outputv) {
				$outputv = trim($outputv);
			}
			$this->ethtool = implode("|", $output);
			$result = TRUE;
			break;
		}
		return $result;
	}

	/**
	 * Get the network interface name, e.g. eth0 or ethx.
	 * @return The network interface name.
	 */
	public function getDeviceName() {
		return $this->name;
	}

	/**
	 * Check whether the network interface exists.
	 * @return TRUE if the network interface exists, otherwise FALSE.
	 */
	public function exists() {
		if($this->getData("ifconfig") === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Get the network interface IPv4 address.
	 * @return The network interface IPv4 address, otherwise FALSE.
	 */
	public function getIP() {
		if($this->getData("ifconfig") === FALSE)
			return FALSE;
		if(1 !== preg_match("/inet addr:\s*({$this->regex['ipv4']})/i",
		  $this->ifconfig, $matches))
			return FALSE;
		return $matches[1];
	}

	/**
	 * Get the network interface IPv6 address.
	 * @return The network interface IPv6 address, otherwise FALSE.
	 */
	public function getIP6() {
		if($this->getData("ifconfig") === FALSE)
			return FALSE;
		if(1 !== preg_match("/inet6 addr:\s*({$this->regex['ipv6cidr']})".
		  "\s*Scope:Global/i", $this->ifconfig, $matches))
			return FALSE;
		return $matches[2];
	}

	/**
	 * Get the network interface mask.
	 * @return The network interface mask, otherwise FALSE.
	 */
	public function getMask() {
		if($this->getData("ifconfig") === FALSE)
			return FALSE;
		if(1 !== preg_match("/Mask:\s*({$this->regex['ipv4']})/i",
		  $this->ifconfig, $matches))
			return FALSE;
		return $matches[1];
	}

	/**
	 * Get the network interface IPv6 mask/prefix length.
	 * @return The network interface IPv6 mask/prefix length as integer,
	 *   otherwise FALSE.
	 */
	public function getMask6() {
		if($this->getData("ifconfig") === FALSE)
			return FALSE;
		if(1 !== preg_match("/inet6 addr:\s*({$this->regex['ipv6cidr']})".
		  "\s*Scope:Global/i", $this->ifconfig, $matches))
			return FALSE;
		return intval($matches[3]);
	}

	/**
	 * Get the network interface MAC address.
	 * @return The network interface MAC address, otherwise FALSE.
	 */
	public function getMAC() {
		return trim(file_get_contents(sprintf("/sys/class/net/%s/address",
		  $this->getDeviceName())));
	}

	/**
	 * Get the network interface MTU.
	 * @return The network interface MTU, otherwise FALSE.
	 */
	public function getMTU() {
		return trim(file_get_contents(sprintf("/sys/class/net/%s/mtu",
		  $this->getDeviceName())));
	}

	/**
	 * Get the network interface IPv4 default gateway.
	 * @return The interface default gateway, or FALSE on failure.
	 */
	public function getGateway() {
		$cmd = sprintf("export LANG=C; ip -4 route show dev %s",
		  $this->getDeviceName());
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		$output = implode("\n", $output);
		// Parse command output:
		// 192.168.178.0/24  proto kernel  scope link  src 192.168.178.21 
		// default via 192.168.178.1
		$regex = sprintf('/default via\s+(%s)/im', $this->regex['ipv4']);
		if(1 !== preg_match($regex, $output, $matches))
			return FALSE;
		return $matches[1];
	}

	/**
	 * Get the network interface IPv6 default gateway.
	 * @return The interface default gateway, or FALSE on failure.
	 */
	public function getGateway6() {
		$cmd = sprintf("export LANG=C; ip -6 route show dev %s",
		  $this->getDeviceName());
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		$output = implode("\n", $output);
		// Parse command output:
		// fe80::/64  proto kernel  metric 256  mtu 1500 advmss 1440 hoplimit 4294967295
		// default via fe80::21e:13ff:fef9:af0  metric 1024  mtu 1500 advmss 1440 hoplimit 4294967295
		$regex = sprintf('/default via\s+(%s)\s+/im', $this->regex['ipv6']);
		if(1 !== preg_match($regex, $output, $matches))
			return FALSE;
		return $matches[1];
	}

	/**
	 * Get the network interface state.
	 * @return The network interface state, otherwise FALSE.
	 */
	public function getState() {
		if($this->getData("ethtool") === FALSE)
			return FALSE;
		if(1 !== preg_match("/state {$this->regex['state']}/i",
		  $this->ethtool, $matches))
			return FALSE;
		return $matches[0];
	}

	/**
	 * Get the network interface link state.
	 * @return TRUE if link is established, otherwise FALSE.
	 */
	public function getLink() {
		if($this->getData("ethtool") === FALSE)
			return FALSE;
		if(1 !== preg_match("/Link detected: yes/i",
		  $this->ethtool, $matches))
			return FALSE;
		return TRUE;
	}

	/**
	 * Get the network interface statistics.
	 * @return The network interface statistics, otherwise FALSE.
	 * array(
	 *   rx_bytes => xxx,
	 *   rx_packets => xxx,
	 *   rx_errors => xxx,
	 *   rx_dropped => xxx,
	 *   rx_fifo_errors => xxx,
	 *   rx_frame_errors => xxx,
	 *   rx_compressed => xxx,
	 *   rx_multicast => xxx,
	 *   tx_bytes => xxx,
	 *   tx_packets => xxx,
	 *   tx_errors => xxx,
	 *   tx_dropped => xxx,
	 *   tx_fifo_errors => xxx,
	 *   tx_collisions => xxx,
	 *   tx_carrier_errors => xxx,
	 *   tx_compressed => xxx
	 * )
	 */
	public function getStatistics() {
		// Parse command output:
		// Inter-|   Receive                                                |  Transmit
		//  face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
		//     lo:  156976     947    0    0    0     0          0         0   156976     947    0    0    0     0       0          0
		//   eth0:162137708  168681    0    0    0     0          0         0 15969317  103565    0    0    0     0       0          0
		//   eth1:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
		$regex = sprintf('/^\s*%s:(.*)$/im', $this->getDeviceName());
		if(1 !== preg_match($regex, file_get_contents("/proc/net/dev"),
		  $matches))
			return FALSE;
		$data = preg_split("/[\s]+/", $matches[1]);
		return array(
			  "rx_bytes" => $data[0],
			  "rx_packets" => $data[1],
			  "rx_errors" => $data[2],
			  "rx_dropped" => $data[3],
			  "rx_fifo_errors" => $data[4],
			  "rx_frame_errors" => $data[5],
			  "rx_compressed" => $data[6],
			  "rx_multicast" => $data[7],
			  "tx_bytes" => $data[8],
			  "tx_packets" => $data[9],
			  "tx_errors" => $data[10],
			  "tx_dropped" => $data[11],
			  "tx_fifo_errors" => $data[12],
			  "tx_collisions" => $data[13],
			  "tx_carrier_errors" => $data[14],
			  "tx_compressed" => $data[15]
		  );
	}
}

/**
 * Helper class to get the groups on the system.
 * @ingroup api
 */
class OMVUserGroups extends OMVObject {
	/**
	 * Get a list of system group names.
	 * @return An array of system group names, otherwise FALSE.
	 */
	public static function get() {
		$cmd = "export LANG=C; getent group";
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		$list = array();
		// Parse command output:
		// shadow:x:42:openmediavault
		// snmp:x:112:
		// sambashare:x:113:
		// openmediavault:x:999:
		// nut:x:114:
		foreach($output as $outputv) {
			$data = explode(":", $outputv);
			if(empty($data))
				continue;
			$list[] = $data[0];
		}
		return $list;
	}
}

/**
 * This class provides a simple interface to handle a system group.
 * @ingroup api
 */
class OMVUserGroup extends OMVObject {
	protected $name = null;
	protected $gid = null;
	protected $members = null;

	/**
	 * Constructor
	 * @param id The name or GID of the system group.
	 */
	public function __construct($id) {
		if(is_int($id))
			$this->gid = $id;
		else
			$this->name = $id;
	}

	/**
	 * Get the system group data.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if(!is_null($this->members))
			return TRUE;
		// http://www.cyberciti.biz/faq/understanding-etcgroup-file
		$cmd = sprintf("export LANG=C; getent group %s", !is_null(
		  $this->gid) ? strval($this->gid) : escapeshellarg($this->name));
		@OMVUtil::exec($cmd, $output, $result);
		if(($result !== 0) || empty($output[0]))
			return FALSE;
		// Parse output:
		// users:x:100:
		// ssh:x:116:test1,test2
		$output = explode(":", $output[0]);
		$this->name = $output[0];
		$this->gid = intval($output[2]);
		$this->members = empty($output[3]) ? array() : explode(
		  ",", $output[3]);
		return TRUE;
	}

	/**
	 * Check whether the system group exists.
	 * @return TRUE if the system group exists, otherwise FALSE.
	 */
	public function exists() {
		if($this->getData() === FALSE)
			return FALSE;
		return !is_null($this->members);
	}

	/**
	 * Get the group name.
	 * @return The group name.
	 */
	public function getName() {
		if(!is_null($this->name))
			return $this->name;
		if($this->getData() === FALSE)
			return FALSE;
		return $this->name;
	}

	/**
	 * Get the group ID.
	 * @return The group ID, otherwise FALSE.
	 */
	public function getGid() {
		if(!is_null($this->gid))
			return $this->gid;
		if($this->getData() === FALSE)
			return FALSE;
		return $this->gid;
	}

	/**
	 * Get the user names who are member of this group.
	 * @return An array of user names that are member of this group,
	 * otherwise FALSE.
	 */
	public function getMembers() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->members;
	}

	/**
	 * Get the group quotas.
	 * @return An array containing the quotas.
	 */
	public function getQuotas() {
		$cmd = sprintf("export LANG=C; EDITOR=cat edquota -g %s",
		  $this->getName());
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;

		// Parse command output:
		// Filesystem                   blocks       soft       hard     inodes     soft     hard
		// /dev/sdb1                     10188          0      12288          4        0        0
		// /dev/sdc1                         0          0      45056          0        0        0
		$result = array();
		foreach($output as $outputk => $outputv) {
			if(preg_match("/^\s+(\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+".
			  "(\d+)\s+(\d+)$/i", $outputv, $matches)) {
				$result[$matches[1]] = array(
					"devicefile" => $matches[1],
					"blocks" => $matches[2],
					"bsoft" => $matches[3],
					"bhard" => $matches[4],
					"inodes" => $matches[5],
					"isoft" => $matches[6],
					"ihard" => $matches[7]
				);
			}
		}
		return $result;
	}
}

/**
 * Helper class to get the users on the system.
 * @ingroup api
 */
class OMVUsers extends OMVObject {
	/**
	 * Get a list of system user names.
	 * @return An array of system user names, otherwise FALSE.
	 */
	public static function get() {
		$cmd = "export LANG=C; getent passwd";
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		$list = array();
		// Parse command output:
		// proftpd:x:109:65534::/var/run/proftpd:/bin/false
		// ftp:x:110:65534::/home/ftp:/bin/false
		// openmediavault:x:999:999::/home/openmediavault:/bin/sh
		// admin:x:998:100:WebGUI administrator:/home/admin:/usr/sbin/nologin
		// nut:x:111:114::/var/lib/nut:/bin/false
		// test:x:1001:100:sdfds:/home/test:/bin/dash
		foreach($output as $outputv) {
			$data = explode(":", $outputv);
			if(empty($data))
				continue;
			$list[] = $data[0]; // User name
		}
		return $list;
	}
}

/**
 * This class provides a simple interface to handle a system user.
 * @ingroup api
 */
class OMVUser extends OMVObject {
	protected $name = null;
	protected $uid = null;
	protected $gid = null;
	protected $password = null;
	protected $gecos = null;
	protected $dir = null;
	protected $shell = null;
	protected $lastchanged = null;
	protected $minimum = null;
	protected $maximum = null;
	protected $warn = null;
	protected $inactive = null;
	protected $expire = null;
	protected $reserved = null;
	protected $groups = null;

	/**
	 * Constructor
	 * @param id The name or UID of the system user.
	 */
	public function __construct($id) {
		if(is_int($id))
			$this->uid = $id;
		else
			$this->name = $id;
	}

	/**
	 * Get the system group data.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData($type) {
		$result = FALSE;
		switch($type) {
		case "shadow":
			if(!is_null($this->password))
				return TRUE;
			// http://www.cyberciti.biz/faq/understanding-etcshadow-file
			$cmd = sprintf("export LANG=C; getent shadow %s", escapeshellarg(
			  $this->getName()));
			@OMVUtil::exec($cmd, $output, $result);
			if(($result !== 0) || empty($output[0]))
				return FALSE;
			$output = explode(":", $output[0]);
			$this->password = $output[1];
			$this->lastchanged = $output[2];
			$this->minimum = $output[3];
			$this->maximum = $output[4];
			$this->warn = $output[5];
			$this->inactive = $output[6];
			$this->expire = $output[7];
			$this->reserved = $output[8];
			$result = TRUE;
			break;
		case "passwd":
			if(!is_null($this->gid))
				return TRUE;
			// http://www.cyberciti.biz/faq/understanding-etcpasswd-file-format
			// http://en.wikipedia.org/wiki/Passwd_(file)
			$cmd = sprintf("export LANG=C; getent passwd %s", !is_null(
			  $this->uid) ? strval($this->uid) : escapeshellarg($this->name));
			@OMVUtil::exec($cmd, $output, $result);
			if(($result !== 0) || empty($output[0]))
				return FALSE;
			$output = explode(":", $output[0]);
			$this->name = $output[0];
			$this->uid = intval($output[2]);
			$this->gid = intval($output[3]);
			$this->gecos = $output[4];
			$this->dir = $output[5];
			$this->shell = $output[6];
			$result = TRUE;
			break;
		case "groups":
			if(!is_null($this->groups))
				return TRUE;
			// http://www.howtogeek.com/howto/ubuntu/see-which-groups-your-linux-user-belongs-to
			$cmd = sprintf("export LANG=C; groups %s", escapeshellarg(
			  $this->getName()));
			@OMVUtil::exec($cmd, $output, $result);
			if(($result !== 0) || empty($output[0]))
				return FALSE;
			// Parse command output:
			// test : users adm audio www-data avahi
			$output = trim(substr(strrchr($output[0], ":"), 1));
			$this->groups = empty($output) ? array() : explode(" ", $output);
			$result = TRUE;
			break;
		}
		return $result;
	}

	/**
	 * Check whether the system user exists.
	 * @return TRUE if the system user exists, otherwise FALSE.
	 */
	public function exists() {
		if($this->getData("passwd") === FALSE)
			return FALSE;
		return !is_null($this->gid);
	}

	/**
	 * Get the group name.
	 * @return The group name.
	 */
	public function getName() {
		if(!is_null($this->name))
			return $this->name;
		if($this->getData("passwd") === FALSE)
			return FALSE;
		return $this->name;
	}

	/**
	 * Get the user ID.
	 * @return The user ID, otherwise FALSE.
	 */
	public function getUid() {
		if(!is_null($this->uid))
			return $this->uid;
		if($this->getData("passwd") === FALSE)
			return FALSE;
		return $this->uid;
	}

	/**
	 * Get the users group ID.
	 * @return The user group ID, otherwise FALSE.
	 */
	public function getGid() {
		if($this->getData("passwd") === FALSE)
			return FALSE;
		return $this->gid;
	}

	/**
	 * Get the comment.
	 * @return The comment, otherwise FALSE.
	 */
	public function getGecos() {
		if($this->getData("passwd") === FALSE)
			return FALSE;
		return $this->gecos;
	}

	/**
	 * Get the home directory.
	 * @return The home directory, otherwise FALSE.
	 */
	public function getHomeDirectory() {
		if($this->getData("passwd") === FALSE)
			return FALSE;
		return $this->dir;
	}

	/**
	 * Get the shell.
	 * @return The shell, otherwise FALSE.
	 */
	public function getShell() {
		if($this->getData("passwd") === FALSE)
			return FALSE;
		return $this->shell;
	}

	/**
	 * Get the encrypted password.
	 * @return The encrypted password, otherwise FALSE.
	 */
	public function getPassword() {
		if($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->password;
	}

	public function getLastChanged() {
		if($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->lastchanged;
	}

	public function getMinimum() {
		if($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->minimum;
	}

	public function getMaximum() {
		if($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->maximum;
	}

	public function getWarn() {
		if($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->warn;
	}

	public function getInactive() {
		if($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->inactive;
	}

	public function getExpire() {
		if($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->expire;
	}

	public function getReserved() {
		if($this->getData("shadow") === FALSE)
			return FALSE;
		return $this->reserved;
	}

	/**
	 * Get the groups the user is in.
	 * @return An array of groups the user is in, otherwise FALSE.
	 */
	public function getGroups() {
		if($this->getData("groups") === FALSE)
			return FALSE;
		return $this->groups;
	}

	/**
	 * Get the user quotas.
	 * @return An array containing the quotas.
	 */
	public function getQuotas() {
		$cmd = sprintf("export LANG=C; EDITOR=cat edquota -u %s",
		  $this->getName());
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;

		// Parse command output:
		// Filesystem                   blocks       soft       hard     inodes     soft     hard
		// /dev/sdb1                     10188          0      12288          4        0        0
		// /dev/sdc1                         0          0      45056          0        0        0
		$result = array();
		foreach($output as $outputk => $outputv) {
			if(preg_match("/^\s+(\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+".
			  "(\d+)\s+(\d+)$/i", $outputv, $matches)) {
				$result[$matches[1]] = array(
					"devicefile" => $matches[1],
					"blocks" => $matches[2],
					"bsoft" => $matches[3],
					"bhard" => $matches[4],
					"inodes" => $matches[5],
					"isoft" => $matches[6],
					"ihard" => $matches[7]
				);
			}
		}
		return $result;
	}

	/**
	 * Authenticate user via PAM. Note, the calling process must have the
	 * privilege to read the user shadow file.
	 * @param password The password.
	 * @return TRUE if the authentication was successful, otherwise FALSE.
	 */
	public function authenticate($password) {
		return pam_auth($this->getName(), $password);
	}
}

/**
 * This class provides a simple interface to handle a LVM physical volume.
 * @ingroup api
 */
class OMVLvmPhysicalVolumes extends OMVObject {
	/**
	 * Get a list of physical volumes.
	 * @return A list of physical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/sdb
	 *   1 => /dev/sdd
	 * )
	 */
	public static function get() {
		$cmd = "export LANG=C; pvdisplay --noheadings -C -o pv_name ".
		  "2>/dev/null";
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			return FALSE;
		}
		$list = array();
		// Parse command output:
		//   /dev/sdc      
		//   /dev/sdd      
		//   unknown device
		foreach($output as $outputk => $outputv) {
			$deviceFile = trim($outputv);
			if(!is_devicefile($deviceFile))
				continue;
			$list[] = $deviceFile;
		}
		return $list;
	}
}

/**
 * Helper class to get the LVM logical volumes.
 * @ingroup api
 */
class OMVLvmLogicalVolumes extends OMVObject {
	/**
	 * Get a list of logical volumes.
	 * @return A list of logical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/mapper/vgName-lvName
	 *   1 => /dev/mapper/...
	 * )
	 */
	public static function get() {
		$cmd = "export LANG=C; lvdisplay --noheadings --separator '|' ".
		  "-C -o vg_name,lv_name 2>/dev/null";
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			return FALSE;
		}
		$list = array();
		foreach($output as $outputk => $outputv) {
			$outputv = explode("|", trim($outputv));
			// Replace '-' with '--' in volume group name.
			$list[] = sprintf("/dev/mapper/%s-%s",
			  str_replace("-", "--", $outputv[0]),
			  str_replace("-", "--", $outputv[1]));
			// Alternative device path: /dev/<vg_name>/<lv_name>
			//$list[] = sprintf("/dev/%s/%s", $outputv[0], $outputv[1]);
		}
		return $list;
	}
}

/**
 * This class provides a simple interface to handle a LVM logical volume.
 * @ingroup api
 */
class OMVLvmLogicalVolume extends OMVDeviceMapperDevice {
	protected $deviceFile = "";
	protected $name = "";
	protected $uuid = "";
	protected $size = FALSE;
	protected $attr = "";
	protected $kernelMajor = "";
	protected $kernelMinor = "";
	protected $vgName = "";
	private $dataCached = FALSE;

	/**
	 * Constructor
	 * @param deviceFile The device file, e.g. /dev/vg0/lvol0 or
	 * /dev/mapper/vg0-lvol0.
	 */
	public function __construct($deviceFile) {
		$this->deviceFile = $deviceFile;
	}

	/**
	 * Get the logical volume detailed informations.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if($this->dataCached !== FALSE)
			return TRUE;

		// Parse command output:
		// VOSP6y-Lw75-pbTs-3v8A-6Qek-XBKJ-Md2G1q|vltest|test|12582912B
		$cmd = sprintf("export LANG=C; lvdisplay --noheadings ".
		  "--separator '|' -C -o lv_uuid,lv_name,vg_name,lv_size,lv_attr,".
		  "lv_kernel_major,lv_kernel_minor --unit b %s",
		  escapeshellarg($this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;

		$output = explode("|", trim($output[0]));

		$this->uuid = $output[0];
		$this->name = $output[1];
		$this->vgName = $output[2];
		$this->size = substr($output[3], 0, -1);
		$this->attr = $output[4];
		$this->kernelMajor = intval($output[5]);
		$this->kernelMinor = intval($output[6]);

		// Set flag to mark informations has been successfully read
		$this->dataCached = TRUE;

		return TRUE;
	}

	/**
	 * Refresh the cached informations.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		if($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Checks if the logical volume exists.
	 * @return TRUE if the logical volume exists, otherwise FALSE.
	 */
	public function exists() {
		if($this->getData() === FALSE)
			return FALSE;
		return !empty($this->uuid);
	}

	/**
	 * Get the device path, e.g. /dev/vg0/lvol0 or /dev/mapper/vg0-lvol0.
	 * @return The device path.
	 */
	public function getDeviceFile() {
		return $this->deviceFile;
	}

	/**
	 * Get the name of the volume group.
	 * @return The volume group name, FALSE on failure.
	 */
	public function getVGName() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->vgName;
	}

	/**
	 * Get the name of the logical volume.
	 * @return The logical volume name, e.g. lvol0, or FALSE on failure.
	 */
	public function getName() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->name;
	}

	/**
	 * Get the path of the logical volume.
	 * @return The logical volume path, e.g. /dev/vg0/lvol0, or FALSE on
	 * failure.
	 */
	public function getPath() {
		if($this->getData() === FALSE)
			return FALSE;
		return sprintf("/dev/%s/%s", $this->getVGName(), $this->getName());
	}

	/**
	 * Get the size of the logical volume in bytes.
	 * @return The size of the logical volume in bytes as string,
	 * FALSE on failure.
	 */
	public function getSize() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->size;
	}

	/**
	 * Get the UUID of the array.
	 * @return The UUID of the array, FALSE on failure.
	 */
	public function getUuid() {
		if($this->getData() === FALSE)
			return FALSE;
		return $this->uuid;
	}

	/**
	 * Get the description of the logical volume.
	 * @return The logical volume description, FALSE on failure.
	 */
	public function getDescription() {
		if($this->getData() === FALSE)
			return FALSE;
		return sprintf(gettext("LVM logical volume %s [%s, %s]"),
		  $this->getName(), $this->getDeviceFile(),
		  binary_format($this->getSize()));
	}
}

///////////////////////////////////////////////////////////////////////////////
// Register default storage device backends.
///////////////////////////////////////////////////////////////////////////////
OMVStorageDevices::registerBackend(new OMVStorageDeviceBackendHDD());
OMVStorageDevices::registerBackend(new OMVStorageDeviceBackendHBA());
OMVStorageDevices::registerBackend(new OMVStorageDeviceBackendMdadm());
OMVStorageDevices::registerBackend(new OMVStorageDeviceBackendLVM());
// Any devices of the form /dev/dm-n are for internal use only and should
// never be used. See for more information:
// https://access.redhat.com/site/documentation/en-US/Red_Hat_Enterprise_Linux/6/html-single/DM_Multipath
//OMVStorageDevices::registerBackend(new OMVStorageDeviceBackendDeviceMapper());
OMVStorageDevices::registerBackend(new OMVStorageDeviceBackendLoopDevice());
?>
