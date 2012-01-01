<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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

class OMVSystem {
	/**
	 * Get the partition where '/' is installed on.
	 * @return Returns the partition where '/' is installed (e.g. /dev/sda1)
	 * or FALSE on failure.
	 */
	public static function getRootPartition() {
		$cmd = "export LANG=C; mount | grep -e 'on / '";
		$data = @OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		$data = explode(" ", $data);
		return $data[0];
	}

	/**
	 * Get the /etc/login.defs configuration
	 * @return Array containing the configuration as key/value pairs or
	 * FALSE on failure.
	 */
	public static function getLoginDefs() {
		// Extract the lines that are not commented out. Use 'awk' to convert
		// them to 'xxx=yyy' to use explode to extract the key/value pairs
		// instead of another regex.
		$cmd = 'cat /etc/login.defs | grep -e \'^[^#].\{1,\}$\' | '.
		  'awk \'{printf("%s=%s\n",$1,$2)}\'';
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		$defs = array();
		foreach ($output as $row) {
			$data = explode("=", $row);
			if (empty($data))
				continue;
			$defs[$data[0]] = $data[1];
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
		$cmd = sprintf("sudo %s | grep -e '^%s[0-9]\{1,\}$' | sort -r | " .
		  "head -n 1", $cmdList[$type], $name);
		$deviceName = @OMVUtil::exec($cmd, $output);
		$deviceName = strrev($deviceName);
		return sprintf("%s%d", $name, !empty($deviceName) ?
		  intval($deviceName[0]) + 1 : 0);
	}

	/**
	 * Tell how long the system has been running.
	 * @return The uptime as formated string or FALSE on failure.
	 */
	public static function uptime() {
		@OMVUtil::exec("cat /proc/uptime", $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		$uptime = explode(" ", $output[0]);
		$days = intval($uptime[0] / 86400);
		$hours = intval(($uptime[0] / 3600) % 24);
		$mins = intval(($uptime[0] / 60) % 60);
		$secs = intval($uptime[0] % 60);
		return sprintf("%d %s %d %s %d %s %d %s",
		  $days, ($days == 1) ? gettext("day") : gettext("days"),
		  $hours, ($hours == 1) ? gettext("hour") : gettext("hours"),
		  $mins, ($mins == 1) ? gettext("minute") : gettext("minutes"),
		  $secs, gettext("seconds"));
	}

	/**
	 * Get system date and time.
	 * @return An array containing the system date and time in local format,
	 * ISO 8601 (without numeric timezone) and UNIX timestamp if successful,
	 * otherwise FALSE.
	 */
	public static function date() {
		// If system date and time has been modified via NTP or manually
		// the PHP date functions do not return correct values until the
		// Apache server has not been restarted.
		$cmd = "date; date +'%Y-%m-%dT%H:%M:%S'; date +'%s'";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		return array(
			"local" => $output[0],
			"ISO8601" => $output[1],
			"timestamp" => $output[2]
		);
	}

	/**
	 * Get load average in regard to both the CPU and IO over time.
	 * @return The CPU and IO utilization of the last one, five, and 10 minute
	 * periods or FALSE on failure.
	 */
	public static function getLoadAverage() {
		@OMVUtil::exec("cat /proc/loadavg", $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		$loadavg = explode(" ", $output[0]);
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
		if ($result !== 0) {
			return FALSE;
		}
		$result = array(
		  "total" => 0,
		  "free" => 0,
		  "used" => 0,
		  "percent" => 0
		);
		foreach ($output as $outputk => $outputv) {
			if (preg_match('/^MemTotal:\s+(\d+)\s+kB$/i', $outputv,
			  $matches)) {
				$result['total'] = $matches[1] * 1024;
			} else if (preg_match('/^MemFree:\s+(\d+)\s+kB$/i', $outputv,
			  $matches)) {
				$result['free'] = $matches[1] * 1024;
			}
		}
		$result['used'] = $result['total'] - $result['free'];
		$result['percent'] = round(($result['used'] * 100) / $result['total']);
*/
		@OMVUtil::exec("free -b", $output, $result);
		if ($result !== 0) {
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
		// Parse output:
		//              total       used       free     shared    buffers     cached
		// Mem:     526987264  195010560  331976704          0   28565504  120373248
		// -/+ buffers/cache:   46071808  480915456
		// Swap:    401596416          0  401596416
		// Total:   928583680  195137536  733446144
		foreach ($output as $outputk => $outputv) {
			if (preg_match('/^Mem:\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)$/',
			  $outputv, $matches)) {
				$result['mem'] = array(
					"total" => $matches[1],
					"used" => $matches[2],
					"free" => $matches[3],
					"shared" => $matches[4],
					"buffers" => $matches[5],
					"cached" => $matches[6]
				);
			} else if (preg_match('/^-\/\+ buffers\/cache:\s+(\d+)\s+(\d+)$/',
			  $outputv, $matches)) {
				$result['wobufferscache'] = array(
					"used" => $matches[1],
					"free" => $matches[2]
				);
			} else if (preg_match('/^Swap:\s+(\d+)\s+(\d+)\s+(\d+)$/',
			  $outputv, $matches)) {
				$result['swap'] = array(
					"total" => $matches[1],
					"used" => $matches[2],
					"free" => $matches[3]
				);
			} else if (preg_match('/^Total:\s+(\d+)\s+(\d+)\s+(\d+)$/',
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
		@OMVUtil::exec("cat /proc/cpuinfo | grep 'model name'", $output,
		  $result);
		if ($result !== 0) {
			return FALSE;
		}
		$output = explode(":", $output[0]);
		$modelName = trim($output[1]);
		@OMVUtil::exec("cat /proc/stat", $tprev, $result);
		if ($result !== 0) {
			return FALSE;
		}
		sleep(1);
		@OMVUtil::exec("cat /proc/stat", $tnow, $result);
		if ($result !== 0) {
			return FALSE;
		}
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
 * Generic class to handle RAID devices.
 */
class OMVFilesystems extends OMVObject {
	/**
	 * Get a list of available/detected file systems.
	 * @return A list of file systems, otherwise FALSE. The array elements
	 * contain the fields: \em devicefile, \em uuid, \em label and \em type.
	 * The devicefile is used as key to access the array elements.
	 */
	public static function get() {
		$cmd = "export LANG=C; sudo blkid -o full";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		$result = array();
		foreach ($output as $outputk => $outputv) {
			// Parse command output:
			// /dev/sdb: UUID="e6a0f61e-d969-ffcb-d4c5-06e7711285a3" LABEL="dhcppc4:0" TYPE="linux_raid_member"
			// /dev/sdg1: LABEL="test" UUID="d19bcea0-a323-4aea-9791-c6578180c129" TYPE="jfs"
			$regex = '/^(\S+): (.+)$/i';
			if (1 !== preg_match($regex, $outputv, $matches))
				continue;
			$data = array(
				"devicefile" => $matches[1],
				"uuid" => "",
				"label" => "",
				"type" => ""
			);
			$output = explode(" ", $matches[2]);
			foreach ($output as $outputk => &$outputv) {
				$keyValue = explode("=", $outputv);
				if (count($keyValue) != 2)
					continue;
				$data[strtolower($keyValue[0])] = substr($keyValue[1], 1, -1);
			}
			$result[$data['devicefile']] = $data;
/*
			// Alternative implementation:
			$regex = '/^(\S+): (LABEL="(\S+)" )?UUID="(\S+)" (LABEL="(\S+)" )?TYPE="(\S+)"$/i';
			if (1 !== preg_match($regex, $outputv, $matches))
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
 */
class OMVFilesystem extends OMVObject {
	protected $uuid = "";
	protected $type = "";
	protected $deviceFile = "";
	protected $label = "";
	protected $ptType = "";
	protected $usage = "";
	private $dataCached = FALSE;

	/**
	 * Constructor
	 * @param uuid The UUID or device path of the filesystem, e.g.
	 * /dev/sde1, 78b669c1-9183-4ca3-a32c-80a4e2c61e2d, 7A48-BA97 (DOS)
	 * or 2ED43920D438EC29 (NTFS).
	 */
	public function __construct($id) {
		if (!is_devicefile($id))
			$this->uuid = $id;
		else
			$this->deviceFile = $id;
	}

	/**
	 * Get the filesystem detailed informations.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if ($this->dataCached !== FALSE)
			return TRUE;

		if (!empty($this->uuid)) {
			// Get the device name containing the file system. This is
			// required for the blkid low-level probing mode.
			$cmd = sprintf("export LANG=C; sudo findfs UUID=%s", $this->uuid);
			@OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0)
				return FALSE;
			$this->deviceFile = $output[0];
		}

		unset($output);
		$cmd = sprintf("export LANG=C; sudo blkid -p %s", $this->deviceFile);
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;

		// Parse command output:
		// /dev/sda1: UUID="3d9be906-d592-449b-93c4-3dd8888801b2" TYPE="ext3"
		// /dev/sdg1: LABEL="test" UUID="d19bcea0-a323-4aea-9791-c6578180c129" TYPE="jfs"
		$regex = '/^(\S+): (.+)$/i';
		if (1 !== preg_match($regex, $output[0], $matches))
			return FALSE;
		$data = array(
			"devicefile" => $matches[1],
			"uuid" => "",
			"label" => "",
			"type" => "",
			"pttype" => "",
			"usage" => ""
		);
		$output = explode(" ", $matches[2]);
		foreach ($output as $outputk => &$outputv) {
			$keyValue = explode("=", $outputv);
			if (count($keyValue) != 2)
				continue;
			$data[strtolower($keyValue[0])] = substr($keyValue[1], 1, -1);
		}

		$this->deviceFile = $data['devicefile'];
		$this->uuid = $data['uuid'];
		$this->label = $data['label'];
		$this->type = $data['type'];
		$this->ptType = $data['pttype'];
		$this->usage = $data['usage'];

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
		if ($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Checks if the filesystem exists.
	 * @return TRUE if the device exists, otherwise FALSE.
	 */
	public function exists() {
		if ($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Get the UUID of the filesystem.
	 * @return The UUID of the filesystem, otherwise FALSE.
	 */
	public function getUuid() {
		return $this->uuid;
	}

	/**
	 * Get the filesystem label.
	 * @return The filesystem label, otherwise FALSE.
	 */
	public function getLabel() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->label;
	}

	/**
	 * Get the filesystem type.
	 * @return The filesystem type, otherwise FALSE.
	 */
	public function getType() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->type;
	}

	/**
	 * Get the partition type, e.g. 'dos'.
	 * @return The filesystem type, otherwise FALSE.
	 */
	public function getPartitionType() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->ptType;
	}

	/**
	 * Get the usage, e.g. 'other' or 'filesystem'.
	 * @return The filesystem type, otherwise FALSE.
	 */
	public function getUsage() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->usage;
	}

	/**
	 * Get the device path of the filesystem, e.g /dev/sdb1.
	 * @return The device name, otherwise FALSE.
	 */
	public function getDeviceFile() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->deviceFile;
	}

	/**
	 * Get the filesystem block size.
	 * @return The block size, otherwise FALSE.
	 */
	public function getBlockSize() {
		if ($this->getData() === FALSE)
			return FALSE;
		$cmd = sprintf("sudo blockdev --getbsz %s ", $this->deviceFile);
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		return intval($output[0]);
	}

	/**
	 * Get filesystem statistics.
	 * @return The filesystem statistics if successful, otherwise FALSE. The
	 * following fields are included: \em devicefile, \em type, \em blocks,
	 * \em used, \em available, \em percentage and \em mountpoint. Please
	 * note, the fields \em used and \em available are strings and their unit
	 * is 'B' (bytes).
	 */
	public function getStatistics() {
		if ($this->getData() === FALSE)
			return FALSE;
		$data = array();
		$cmd = sprintf("export LANG=C; sudo df -P %s", $this->deviceFile);
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		// Parse command output:
		// Filesystem         1024-blocks      Used Available Capacity Mounted on
		// /dev/sdf1               103180       148    103032       1% /media/f4f01ecc-548b-4e96-8fc4-874c6fdc9be0
		$matches = preg_split("/[\s,]+/", array_pop($output));
		$data = array(
			"devicefile" => $this->deviceFile,
			"type" => $this->deviceFile,
			"blocks" => $matches[1],
			"used" => binary_convert($matches[2], "KiB", "B"),
			"available" => binary_convert($matches[3], "KiB", "B"),
			"percentage" => intval(trim($matches[4], "%")),
			"mountpoint" => $matches[5]
		);
		return $data;
	}

	/**
	 * Check if a filesystem is mounted.
	 * @return TRUE if the filesystem is mounted, otherwise FALSE.
	 */
	public function isMounted() {
		if ($this->getData() === FALSE)
			return FALSE;
		return self::isMountedEx("filesystem", $this->getDeviceFile());
	}

	/**
	 * Check if a filesystem is mounted.
	 * @param key The name of the attribute to search for. This can
	 *  be 'filesystem' or 'mountpoint'.
	 * @param value The value to be check for.
	 * @return TRUE if the filesystem is mounted, otherwise FALSE.
	 */
	public static function isMountedEx($key, $value) {
		OMVUtil::exec("export LANG=C; sudo mount", $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		$result = FALSE;
		foreach ($output as $outputv) {
			$regex = "/^(\S+) on (\S+) type (\S+) \((\S+)\)$/";
			if (1 !== preg_match($regex, $outputv, $matches))
				continue;
			// Parse command output:
			// tmpfs on /dev/shm type tmpfs (rw,nosuid,nodev)
			// devpts on /dev/pts type devpts (rw,noexec,nosuid,gid=5,mode=620)
			// /dev/sdf1 on /media/f4f01ecc-548b-4e96-8fc4-874c6fdc9be0 type jfs (rw)
			switch ($key) {
			case "filesystem":
				$result = ($matches[1] === $value);
				break;
			case "mountpoint":
				$result = ($matches[2] === $value);
				break;
			}
			if (TRUE === $result)
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
		$cmd = sprintf("export LANG=C; sudo blkid | grep -E '^%s[0-9]*:.+$'",
		  $devicefile);
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		return TRUE;
	} 
}

/**
 * Generic class for storage devices.
 */
class OMVStorageDevice extends OMVObject {
	protected $deviceFile = FALSE;
	protected $deviceFileById = FALSE;
	protected $size = FALSE;

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
	 * Checks if the device exists.
	 * @return TRUE if the device exists, otherwise FALSE.
	 */
	public function exists() {
		$cmd = sprintf("sudo test -b %s", escapeshellarg(!$this->deviceFile ?
		  $this->deviceFileById : $this->deviceFile));
		@OMVUtil::exec($cmd, $output, $result);
		return ($result == 0) ? TRUE : FALSE;
	}

	/**
	 * Get the device path, e.g. /dev/sda.
	 * @return The device path.
	 */
	public function getDeviceFile() {
		if(!$this->deviceFile) {
			$cmd = sprintf("export LANG=C; readlink -e %s",
			  escapeshellarg($this->deviceFileById));
			@OMVUtil::exec($cmd, $output, $result);
			if($result !== 0)
				return FALSE;
			$this->deviceFile = $output[0];
		}
		return $this->deviceFile;
	}

	/**
	 * Get device path, e.g. /dev/disk/by-id/wwn-0x5000cca211cc703c or
	 * /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML.
	 * @return The device path (/dev/disk/by-id/xxx) if available, otherwise
	 * /dev/xxx will be returned. In case of an error FALSE will be returned.
	 */
	public function getDeviceFileById() {
		if(!$this->deviceFileById) {
			// Sort in reverse order, thus SCSI/SATA entries are listed
			// on top.
			$cmd = sprintf("export LANG=C; test -d '/dev/disk/by-id' && ".
			  "ls -ogr '/dev/disk/by-id/' | grep -e '/%s$' | ".
			  "awk '{print $7}'", $this->getDeviceName());
			@OMVUtil::exec($cmd, $output, $result);
			if($result !== 0)
				return FALSE;
			// Command output:
			// ata-VMware_Virtual_IDE_Hard_Drive_00000000000000000001
			// scsi-SATA_VMware_Virtual_00000000000000000001
			if(empty($output)) {
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
	 * Get the device name, e.g. sda or hdb.
	 * @return The device name.
	 */
	public function getDeviceName() {
		$deviceFile = $this->getDeviceFile();
		return substr($deviceFile, strrpos($deviceFile, "/") + 1);
	}

	/**
	 * Get the size of the device in bytes.
	 * @return The size of the device in bytes, otherwise FALSE.
	 */
	public function getSize() {
		return $this->size;
	}

	/**
	 * Get the capacity in the highest possible unit.
	 * @return The capacity of the device, otherwise FALSE.
	 */
	public function getCapacity() {
		if (FALSE === ($size = $this->getSize()))
			return FALSE;
		return binary_format($size);
	}

	/**
	 * Get the description of the device.
	 * @return The device description.
	 */
	public function getDescription() {
		return "";
	}

	/**
	 * Check if the given device is a USB device.
	 */
	public function isUsb() {
		$cmd = sprintf("export LANG=C; sudo udevadm info --name=%s " .
		  "--attribute-walk | grep 'SUBSYSTEMS==\"usb\"'",
		  $this->getDeviceFile());
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		return empty($result);
	}
}

/**
 * Generic class to handle hard disks devices.
 */
class OMVHardDiskDevices extends OMVObject {
	/**
	 * Get a list device paths from all hard disk devices.
	 * @return A list of device files, otherwise FALSE.
	 */
	public static function get() {
		$cmd = "sudo cat /proc/partitions";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		$list = array();
		foreach ($output as $outputk => $outputv) {
			$regex = "/^\s+(\d+)\s+(\d+)\s+(\d+)\s+((h|s|x?v)d[a-z]+)$/";
			if (1 !== preg_match($regex, $outputv, $matches))
				continue;
			$list[] = sprintf("/dev/%s", $matches[4]);
		}
		return $list;
	}
}

/**
 * Class to get detailed informations about a hard disk device. Try to use
 * sysfs only to get the required informations, otherwise the disk will
 * spin-up if it is in sleep mode.
 */
class OMVHardDiskDevice extends OMVStorageDevice {
	protected $model = FALSE;
	protected $identityInfo = FALSE;
	protected $vendor = FALSE;

	/**
	 * Get the size of the device in bytes.
	 * @return The size (64bit) of the device in bytes as string.
	 */
	public function getSize() {
		if ($this->size === FALSE) {
			$cmd = sprintf("sudo blockdev --getsize64 %s",
			  $this->getDeviceFile());
			@OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0)
				return FALSE;
			$this->size = $output[0];
		}
		return $this->size;
	}

	/**
	 * Get the device model.
	 * @return The device model, otherwise FALSE.
	 */
	public function getModel() {
		if ($this->model === FALSE) {
			if (FALSE === ($ii = $this->getIdentityInfo())) {
				$cmd = sprintf("sudo cat /sys/block/%s/device/model",
				  $this->getDeviceName());
				@OMVUtil::exec($cmd, $output, $result);
				if ($result !== 0)
					return FALSE;
				$this->model = $output[0];
			} else {
				$this->model = $ii['devicemodel'];
			}
		}
		return $this->model;
	}

	/**
	 * Get the device vendor.
	 * @return The device vendor, otherwise FALSE.
	 */
	public function getVendor() {
		if ($this->vendor === FALSE) {
			$cmd = sprintf("sudo cat /sys/block/%s/device/vendor",
			  $this->getDeviceName());
			@OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0)
				return FALSE;
			$this->vendor = $output[0];
		}
		return $this->vendor;
	}

	/**
	 * Get the description of the device.
	 * @return The device description.
	 */
	public function getDescription() {
		$model = $this->getModel();
		return sprintf("%s [%s, %s]", !empty($model) ? $model : gettext("n/a"),
		  $this->getDeviceFile(), $this->getCapacity());
	}

	/**
	 * Get the temperature in °C (value only, no unit).
	 * @return The temperature value, otherwise FALSE.
	 */
	public function getTemperature() {
		if (FALSE === ($stats = $this->getSmartStats()))
			return FALSE;
		$temp = FALSE;
		foreach ($stats['attributes'] as $statv) {
			$found = false;
			switch ($statv['id']) {
			case 194:
			case 231:
				// The value may look like: '43 (Lifetime Min/Max 16/61)'
				$regex = "/^(\d+)(\s+(.+))*$/";
				if (1 !== preg_match($regex, $statv['rawvalue'], $matches))
					continue;
				$temp = $matches[1];
				$found = true;
				break;
			}
			if ($found)
				break;
		}
		return $temp;
	}

	/**
	 * Get the device serial number.
	 * @return The device serial number.
	 */
	public function getSerialNumber() {
		if (FALSE === ($ii = $this->getIdentityInfo()))
			return gettext("n/a");
		return $ii['serialnumber'];
	}

	/**
	 * Get the device firmware version.
	 * @return The device firmware version.
	 */
	public function getFirmwareVersion() {
		if (FALSE === ($ii = $this->getIdentityInfo()))
			return gettext("n/a");
		return $ii['firmwareversion'];
	}

	/**
	 * Get the S.M.A.R.T. informations for the given disk device.
	 * @return All S.M.A.R.T. information for the given device,
	 * otherwise FALSE.
	 */
	public function getSmartStats() {
		if (!$this->exists())
			return FALSE;
		$cmd = sprintf("export LANG=C; sudo smartctl -a %s",
		  $this->getDeviceFile());
		@OMVUtil::exec($cmd, $output, $result);
		// Bit 0: Command line did not parse
		// Bit 1: Device open failed, or device did not return an
		//        IDENTIFY DEVICE structure
		if (($result !== 0) && (($result & 0) || ($result & 1)))
			return FALSE;
		$attributes = array();
		$selfTestLogs = array();
		foreach ($output as $outputv) {
			// Parse command output:
			// 194 Temperature_Celsius     0x0002   139   139   000    Old_age   Always       -       43 (Lifetime Min/Max 16/61)
			$regex = "/^\s*(\d+)\s+(\S+)\s+(\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s+" .
			  "(\S+)\s+(\S+)\s+(\S+)\s+(.+)$/";
			if (1 == preg_match($regex, $outputv, $matches)) {
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
			// Parse command output:
			// # 2  Short captive       Interrupted (host reset)      80%      1392         -
			$regex = "/^# (\d+)\s+(Short offline|Extended offline|" .
			  "Short captive|Extended captive)\s+(.+)\s+(\d+)%\s+(\d+)" .
			  "\s+(.+)$/";
			if (1 == preg_match($regex, $outputv, $matches)) {
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
		return array(
			"attributes" => $attributes,
			"selftestlogs" => $selfTestLogs
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
		if (!$this->exists())
			return FALSE;
		if ($this->identityInfo === FALSE) {
			$cmd = sprintf("export LANG=C; sudo smartctl -i %s",
			  $this->getDeviceFile());
			@OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0)
				return FALSE;
			$output = implode("|", $output);
			$regex = "/^.+\|Device Model:\s+(.+)\|Serial Number:\s+(.+)" .
			  "\|Firmware Version:\s+(.+)\|User Capacity:\s+(.+) bytes\|.+$/";
			if (0 >= preg_match($regex, $output, $matches))
				return FALSE;
			$this->identityInfo = array(
				"devicemodel" => $matches[1],
				"serialnumber" => $matches[2],
				"firmwareversion" => $matches[3]
			);
		}
		return $this->identityInfo;
	}
}

/**
 * Generic class to handle RAID devices.
 */
class OMVRaidDevices extends OMVObject {
	/**
	 * Get a list of RAID device paths from all RAID devices.
	 * @return A list of device files, otherwise FALSE.
	 */
	public static function get() {
		$cmd = "sudo cat /proc/partitions";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		$list = array();
		foreach ($output as $outputk => $outputv) {
			$regex = "/^\s+(\d+)\s+(\d+)\s+(\d+)\s+(md\d+)$/";
			if (1 !== preg_match($regex, $outputv, $matches))
				continue;
			$list[] = sprintf("/dev/%s", $matches[4]);
		}
		return $list;
	}
}

/**
 * Class to get detailed informations about a RAID device.
 */
class OMVRaidDevice extends OMVStorageDevice {
	protected $name = "";
	protected $level = FALSE;
	protected $devices = array();
	protected $numDevices = FALSE;
	protected $uuid = FALSE;
	private $dataCached = FALSE;

	/**
	 * Get the RAID device detailed informations.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if ($this->dataCached !== FALSE)
			return TRUE;

		// ARRAY /dev/md0 level=raid5 num-devices=3 metadata=1.2 name=xxxx:0 UUID=a4266bf7:c671b343:c3d6e535:ca455e37
		//    devices=/dev/sdb,/dev/sdc,/dev/sdd
		$cmd = sprintf("export LANG=C; sudo mdadm --detail --brief " .
		  "--verbose %s", $this->getDeviceFile());
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;

		$attributes = array(
			"name" => gettext("n/a"),
			"level" => "",
			"num-devices" => -1,
			"uuid" => "",
			"devices" => ""
		);
		$output = explode(" ", implode(" ", $output));
		foreach ($output as $outputk => &$outputv) {
			$keyValue = explode("=", $outputv);
			if (count($keyValue) != 2)
				continue;
			$attributes[strtolower($keyValue[0])] = $keyValue[1];
		}

		$this->name = $attributes['name'];
		$this->level = $attributes['level'];
		$this->numDevices = intval($attributes['num-devices']);
		$this->uuid = $attributes['uuid'];
		$this->devices = explode(",", $attributes['devices']);

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
		if ($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Checks if the device exists.
	 * @return TRUE if the device exists, otherwise FALSE.
	 */
	public function exists() {
		if ($this->getData() === FALSE)
			return FALSE;
		return ($this->level !== FALSE);
	}

	/**
	 * Get the size of the device in bytes.
	 * @return The size of the device in bytes.
	 */
	public function getSize() {
		if ($this->size === FALSE) {
			$cmd = sprintf("sudo blockdev --getsize64 %s",
			  $this->getDeviceFile());
			@OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0)
				return FALSE;
			$this->size = $output[0];
		}
		return $this->size;
	}

	/**
	 * Get the array name.
	 * @return The array name, otherwise FALSE.
	 */
	public function getName() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->name;
	}

	/**
	 * Get the level of the array.
	 * @return The level of the array, otherwise FALSE.
	 */
	public function getLevel() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->level;
	}

	/**
	 * Get the number of active devices in the array.
	 * @return The number of active devices in the array, otherwise FALSE.
	 */
	public function getNumDevices() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->numDevices;
	}

	/**
	 * Get the UUID of the array.
	 * @return The UUID of the array, otherwise FALSE.
	 */
	public function getUuid() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->uuid;
	}

	/**
	 * Get the list of device files used in the array.
	 * @return A list of device files used in the array, otherwise FALSE.
	 */
	public function getDevices() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->devices;
	}

	/**
	 * Get the status of the array.
	 * @return The status of the array, otherwise FALSE.
	 */
	public function getState() {
		$cmd = sprintf("export LANG=C; sudo mdadm --detail %s | " .
		  "grep -e 'State :'", $this->getDeviceFile());
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		$status = trim(substr($output[0], strpos($output[0], ":") + 1));
		// Check /proc/mdstat
		// md1 : active raid5 sde[2] sdg[1] sdf[0]
		//       207872 blocks super 1.2 level 5, 512k chunk, algorithm 2 [3/3] [UUU]
		//       [=========>...........]  resync = 45.0% (47488/103936) finish=0.0min speed=47488K/sec
		// md0 : active (auto-read-only) raid5 sdd[2] sdc[1] sdb[0]
		//       207872 blocks super 1.2 level 5, 512k chunk, algorithm 2 [3/3] [UUU]
		//       	resync=PENDING
		// md0 : active raid5 sdf[4] sde[3] sdd[2] sdc[1] sdb[0]
		//       311808 blocks super 1.2 level 5, 512k chunk, algorithm 2 [5/5] [UUUUU]
		//       [====>................]  reshape = 20.5% (21504/103936) finish=0.0min speed=21504K/sec
		$cmd = sprintf("cat /proc/mdstat | sed -n '/^%s.*:/,/^[ ]*$/p'",
		  $this->getDeviceName());
		unset($output);
		@OMVUtil::exec($cmd, $output, $result);
		$output = implode(" ", $output);
		if ($result !== 0)
			return FALSE;
		// Extract the resync state
		$resyncState = "";
		if (1 == preg_match("/^.+(resync|reshape)\s*=\s*(.+)$/i", $output,
		  $matches)) {
			$resyncState = trim($matches[2]);
		}
		if ((FALSE !== stristr($status, "resyncing")) ||
		  (FALSE !== stristr($status, "recovering"))) {
			$status = sprintf("%s (%s)", $status, $resyncState);
		} else if (FALSE !== stristr($output, "resync")) {
			$status = sprintf("%s, resyncing (%s)", $status, $resyncState);
		}
		return $status;
	}

	/**
	 * Get detail of the md device.
	 * @return The detail of the md device, otherwise FALSE.
	 */
	public function getDetail() {
		$cmd = sprintf("export LANG=C; sudo mdadm --detail %s",
		  $this->getDeviceFile());
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		array_shift($output);
		return implode("\n", $output);
	}

	/**
	 * Get the description of the device.
	 * @return The device description.
	 */
	public function getDescription() {
		return sprintf("Software RAID %s [%s, %s, %s]", $this->getName(),
		  $this->getDeviceFile(), $this->getLevel(),
		  $this->getCapacity());
	}
}

class OMVNetwork extends OMVObject {
	/**
	 * Get the host name.
	 * @param options Additional command parameters
	 * @return The host name if successful, otherwise FALSE.
	 */
	function getHostname($options) {
		$cmd = sprintf("export LANG=C; sudo hostname %s", $options);
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		return $output[0];
	}

	/**
	 * Get the domain.
	 * @return The domain if successful, otherwise FALSE.
	 */
	function getDomain() {
		$cmd = "export LANG=C; sudo hostname -d";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		return $output[0];
	}
}

class OMVNetworkInterfaces extends OMVObject {
	/**
	 * Get a list of all network interfaces.
	 * @return A list of network interfaces, otherwise FALSE.
	 */
	public static function get() {
		$cmd = "sudo ls /sys/class/net | grep -E '^[a-z]+[0-9]+|lo$'";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		return $output;
	}

	/**
	 * Get a list of network interfaces by type.
	 * @param type The type of interface, e.g. eth, lo or bond.
	 * @return A list of network interfaces, otherwise FALSE.
	 */
	public static function getByType($type) {
		$cmd = sprintf("sudo ls /sys/class/net | grep %s", $type);
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		return $output;
	}
}

/**
 * Network interfaces class.
 */
class OMVNetworkInterface extends OMVObject {
	protected $name = "";
	protected $ifconfig = null;
	protected $ip = null;
	protected $ethtool = null;
	protected $regex = array(
		"ipv4" => "\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}",
		"ipv6" => "(.+)\/(\d{1,2})",
		"state" => "UP|DOWN|UNKNOWN"
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
		switch ($type) {
		case "ifconfig":
			if (!is_null($this->ifconfig))
				return TRUE;
			$cmd = sprintf("export LANG=C; sudo ifconfig %s",
			  $this->getDeviceName());
			@OMVUtil::exec($cmd, $output, $result);
			if (($result !== 0) || empty($output[0]))
				return FALSE;
			$this->ifconfig = implode("|", $output);
			$result = TRUE;
			break;
		case "ip":
			if (!is_null($this->ip))
				return TRUE;
			$cmd = sprintf("export LANG=C; sudo ip addr show dev %s",
			  $this->getDeviceName());
			@OMVUtil::exec($cmd, $output, $result);
			if (($result !== 0) || empty($output[0]))
				return FALSE;
			foreach ($output as $outputk => &$outputv) {
				$outputv = trim($outputv);
			}
			$this->ip = implode("|", $output);
			$result = TRUE;
			break;
		case "ethtool":
			if (!is_null($this->ethtool))
				return TRUE;
			$cmd = sprintf("export LANG=C; sudo ethtool %s",
			  $this->getDeviceName());
			@OMVUtil::exec($cmd, $output, $result);
			if (($result !== 0) || empty($output[0]))
				return FALSE;
			foreach ($output as $outputk => &$outputv) {
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
		if ($this->getData("ifconfig") === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Get the network interface IPv4 address.
	 * @return The network interface IPv4 address, otherwise FALSE.
	 */
	public function getIPv4() {
		if ($this->getData("ifconfig") === FALSE)
			return FALSE;
		if (1 !== preg_match("/inet addr:({$this->regex['ipv4']})/i",
		  $this->ifconfig, $matches))
			return FALSE;
		return $matches[1];
	}

	/**
	 * Get the network interface IPv6 address.
	 * @return The network interface IPv6 address, otherwise FALSE.
	 */
	public function getIPv6() {
		if ($this->getData("ifconfig") === FALSE)
			return FALSE;
		if (1 !== preg_match("/inet6 addr:\s*({$this->regex['ipv6']})/i",
		  $this->ifconfig, $matches))
			return FALSE;
		return $matches[2];
	}

	/**
	 * Get the network interface mask.
	 * @return The network interface mask, otherwise FALSE.
	 */
	public function getMask() {
		if ($this->getData("ifconfig") === FALSE)
			return FALSE;
		if (1 !== preg_match("/Mask:({$this->regex['ipv4']})/i",
		  $this->ifconfig, $matches))
			return FALSE;
		return $matches[1];
	}

	/**
	 * Get the network interface MAC address.
	 * @return The network interface MAC address, otherwise FALSE.
	 */
	public function getMAC() {
		$cmd = sprintf("sudo cat /sys/class/net/%s/address",
		  $this->getDeviceName());
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		return $output[0];
	}

	/**
	 * Get the network interface MTU.
	 * @return The network interface MTU, otherwise FALSE.
	 */
	public function getMTU() {
		$cmd = sprintf("sudo cat /sys/class/net/%s/mtu",
		  $this->getDeviceName());
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		return $output[0];
	}

	/**
	 * Get the network interface default gateway.
	 * @return The interface default gateway, or FALSE on failure.
	 */
	public function getGateway() {
		// Build the regex to search the default gateway for the
		// given interface
		$regex = sprintf("^0\.0\.0\.0.*%s$", $this->getDeviceName());
		$cmd = sprintf("export LANG=C; sudo route -ne | grep -e '%s' | " .
		  "awk '{print $2}'", $regex);
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		return implode($output);
	}

	/**
	 * Get the network interface state.
	 * @return The network interface state, otherwise FALSE.
	 */
	public function getState() {
		if ($this->getData("ethtool") === FALSE)
			return FALSE;
		if (1 !== preg_match("/state {$this->regex['state']}/i",
		  $this->ethtool, $matches))
			return FALSE;
		return $matches[0];
	}

	/**
	 * Get the network interface link state.
	 * @return TRUE if link is established, otherwise FALSE.
	 */
	public function getLink() {
		if ($this->getData("ethtool") === FALSE)
			return FALSE;
		if (1 !== preg_match("/Link detected: yes/i",
		  $this->ethtool, $matches))
			return FALSE;
		return TRUE;
	}

	/**
	 * Get the network interface statistics.
	 * @return The network interface MTU, otherwise FALSE.
	 * array(
	 *   rx_bytes => xxx,
	 *   rx_packets => xxx,
	 *   rx_errors => xxx,
	 *   rx_dropped => xxx,
	 *   rx_fifo_errors => xxx,
	 *   rx_frame_errors => xxx,
	 *   rx_compressed => xxx,
	 *   multicast => xxx,
	 *   tx_bytes => xxx,
	 *   tx_packets => xxx,
	 *   tx_errors => xxx,
	 *   tx_dropped => xxx,
	 *   tx_fifo_errors => xxx,
	 *   collisions => xxx,
	 *   tx_carrier_errors => xxx,
	 *   tx_compressed => xxx
	 * )
	 */
	public function getStatistics() {
		$cmd = sprintf("sudo cat /proc/net/dev | grep %s",
		  $this->getDeviceName());
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		$regex = "/^\s*([a-zA-Z]+\d+):\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)" .
		  "\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)" .
		  "\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)$/";
		if (1 !== preg_match($regex, $output[0], $matches))
			return FALSE;
		$stats = array(
			"rx_bytes" => $matches[2],
			"rx_packets" => $matches[3],
			"rx_errors" => $matches[4],
			"rx_dropped" => $matches[5],
			"rx_fifo_errors" => $matches[6],
			"rx_frame_errors" => $matches[7],
			"rx_compressed" => $matches[8],
			"multicast" => $matches[9],
			"tx_bytes" => $matches[10],
			"tx_packets" => $matches[11],
			"tx_errors" => $matches[12],
			"tx_dropped" => $matches[13],
			"tx_fifo_errors" => $matches[14],
			"collisions" => $matches[15],
			"tx_carrier_errors" => $matches[16],
			"tx_compressed" => $matches[17]
		);
		return $stats;
	}
}

/**
 * System groups class.
 */
class OMVSystemGroups extends OMVObject {
	/**
	 * Get a list of system groups.
	 * @return An array of system groups, otherwise FALSE.
	 */
	public static function get() {
		$cmd = "getent group";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		$list = array();
		foreach ($output as $outputv) {
			if (1 !== preg_match("/^(\S+):x:(\d+):.*$/", $outputv, $matches))
				continue;
			$list[] = $matches[1];
		}
		return $list;
	}
}

/**
 * System group class.
 */
class OMVSystemGroup extends OMVObject {
	protected $name = "";
	protected $gid = -1;

	/**
	 * Constructor
	 * @param name The name of the system group.
	 */
	public function __construct($name) {
		$this->name = $name;
	}

	/**
	 * Get the system group data.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if ($this->gid !== -1)
			return TRUE;
		$cmd = sprintf("getent group | grep '%s'", $this->getName());
		@OMVUtil::exec($cmd, $output, $result);
		if (($result !== 0) || empty($output[0]))
			return FALSE;
		if (1 !== preg_match("/^(\S+):x:(\d+):.*$/", $output[0], $matches))
			return FALSE;
		$this->gid = intval($matches[2]);
		return TRUE;
	}

	/**
	 * Check whether the system group exists.
	 * @return TRUE if the system group exists, otherwise FALSE.
	 */
	public function exists() {
		if ($this->getData() === FALSE)
			return FALSE;
		return ($this->gid !== -1);
	}

	/**
	 * Get the group name.
	 * @return The group name.
	 */
	public function getName() {
		return $this->name;
	}

	/**
	 * Get the group id.
	 * @return The group id, otherwise FALSE.
	 */
	public function getId() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->gid;
	}

	/**
	 * Get the group quotas.
	 * @return An array containing the quotas.
	 */
	public function getQuotas() {
		$cmd = sprintf("export LANG=C; sudo EDITOR=cat edquota -g %s",
		  $this->getName());
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;

		// Parse command output:
		// Filesystem                   blocks       soft       hard     inodes     soft     hard
		// /dev/sdb1                     10188          0      12288          4        0        0
		// /dev/sdc1                         0          0      45056          0        0        0
		$result = array();
		foreach ($output as $outputk => $outputv) {
			if (preg_match("/^\s+(\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+".
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
 * System users class.
 */
class OMVSystemUsers extends OMVObject {
	/**
	 * Get a list of system users.
	 * @return An array of system users, otherwise FALSE.
	 */
	public static function get() {
		$cmd = "getent passwd";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		$list = array();
		foreach ($output as $outputv) {
			$passwd = explode(":", $outputv);
			if (empty($passwd))
				continue;
			$list[] = $passwd[0];
		}
		return $list;
	}
}

/**
 * System user class.
 */
class OMVSystemUser extends OMVObject {
	protected $name = "";
	protected $uid = -1;
	protected $gid = -1;
	protected $password = "";
	protected $class = "";
	protected $homedir = "";
	protected $shell = "";
	protected $lastchanged = "";
	protected $minimum = "";
	protected $maximum = "";
	protected $warn = "";
	protected $inactive = "";
	protected $expire = "";
	protected $reserved = "";

	/**
	 * Constructor
	 * @param name The name of the system group.
	 */
	public function __construct($name) {
		$this->name = $name;
	}

	/**
	 * Get the system group data.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if ($this->uid !== -1)
			return TRUE;
		$cmd = sprintf("getent shadow | grep -e '^%s:'", $this->getName());
		@OMVUtil::exec($cmd, $output, $result);
		if (($result !== 0) || empty($output[0]))
			return FALSE;
		$shadow = explode(":", $output[0]);
		unset($output);
		$cmd = sprintf("getent passwd | grep -e '^%s:'", $this->getName());
		@OMVUtil::exec($cmd, $output, $result);
		if (($result !== 0) || empty($output[0]))
			return FALSE;
		$passwd = explode(":", $output[0]);
		$this->uid = intval($passwd[2]);
		$this->gid = intval($passwd[3]);
		$this->class = $passwd[4];
		$this->homedir = $passwd[5];
		$this->shell = $passwd[6];
		$this->password = $shadow[1];
		$this->lastchanged = $shadow[2];
		$this->minimum = $shadow[3];
		$this->maximum = $shadow[4];
		$this->warn = $shadow[5];
		$this->inactive = $shadow[6];
		$this->expire = $shadow[7];
		$this->reserved = $shadow[8];
		return TRUE;
	}

	/**
	 * Check whether the system user exists.
	 * @return TRUE if the system user exists, otherwise FALSE.
	 */
	public function exists() {
		if ($this->getData() === FALSE)
			return FALSE;
		return ($this->gid !== -1);
	}

	/**
	 * Get the group name.
	 * @return The group name.
	 */
	public function getName() {
		return $this->name;
	}

	/**
	 * Get the user id.
	 * @return The user id, otherwise FALSE.
	 */
	public function getUid() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->uid;
	}

	/**
	 * Get the users group id.
	 * @return The user group id, otherwise FALSE.
	 */
	public function getGid() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->gid;
	}

	public function getClass() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->class;
	}

	/**
	 * Get the home directory.
	 * @return The home directory, otherwise FALSE.
	 */
	public function getHomeDir() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->homedir;
	}

	/**
	 * Get the shell.
	 * @return The shell, otherwise FALSE.
	 */
	public function getShell() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->shell;
	}

	/**
	 * Get the encrypted password.
	 * @return The encrypted password, otherwise FALSE.
	 */
	public function getPassword() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->password;
	}

	public function getLastChanged() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->lastchanged;
	}

	public function getMinimum() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->minimum;
	}

	public function getMaximum() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->maximum;
	}

	public function getWarn() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->warn;
	}

	public function getInactive() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->inactive;
	}

	public function getExpire() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->expire;
	}

	public function getReserved() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->reserved;
	}

	/**
	 * Get the user quotas.
	 * @return An array containing the quotas.
	 */
	public function getQuotas() {
		$cmd = sprintf("export LANG=C; sudo EDITOR=cat edquota -u %s",
		  $this->getName());
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;

		// Parse command output:
		// Filesystem                   blocks       soft       hard     inodes     soft     hard
		// /dev/sdb1                     10188          0      12288          4        0        0
		// /dev/sdc1                         0          0      45056          0        0        0
		$result = array();
		foreach ($output as $outputk => $outputv) {
			if (preg_match("/^\s+(\S+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+".
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
 * Class to handle physical volumes.
 */
class OMVPhysicalVolumes extends OMVObject {
	/**
	 * Get a list of physical volumes.
	 * @return A list of physical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/sdb
	 *   1 => /dev/sdd
	 * )
	 */
	public static function get() {
		$cmd = "export LANG=C; sudo pvdisplay --noheadings -C -o pv_name";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		$list = array();
		foreach ($output as $outputk => $outputv) {
			$list[] = trim($outputv);
		}
		return $list;
	}
}

/**
 * Class to handle logical volumes.
 */
class OMVLogicalVolumes extends OMVObject {
	/**
	 * Get a list of logical volumes.
	 * @return A list of logical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/mapper/vgName-lvName
	 *   1 => /dev/mapper/...
	 * )
	 */
	public static function get() {
		$cmd = "export LANG=C; sudo lvdisplay --noheadings --separator '|' ".
		  "-C -o vg_name,lv_name";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			return FALSE;
		}
		$list = array();
		foreach ($output as $outputk => $outputv) {
			$outputv = explode("|", trim($outputv));
			$list[] = sprintf("/dev/mapper/%s-%s", $outputv[0], $outputv[1]);
		}
		return $list;
	}
}

/**
 * Class to handle a logical volume.
 */
class OMVLogicalVolume extends OMVObject {
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
		if ($this->dataCached !== FALSE)
			return TRUE;

		// Parse command output:
		// VOSP6y-Lw75-pbTs-3v8A-6Qek-XBKJ-Md2G1q|vltest|test|12582912B
		$cmd = sprintf("export LANG=C; sudo lvdisplay --noheadings ".
		  "--separator '|' -C -o lv_uuid,lv_name,vg_name,lv_size,lv_attr,".
		  "lv_kernel_major,lv_kernel_minor --unit b %s",
		  $this->getDeviceFile());
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
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
		if ($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Checks if the device exists.
	 * @return TRUE if the device exists, otherwise FALSE.
	 */
	public function exists() {
		if ($this->getData() === FALSE)
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
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->vgName;
	}

	/**
	 * Get the name of the logical volume.
	 * @return The logical volume name, e.g. lvol0, or FALSE on failure.
	 */
	public function getName() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->name;
	}

	/**
	 * Get the path of the logical volume.
	 * @return The logical volume path, e.g. /dev/vg0/lvol0, or FALSE on
	 * failure.
	 */
	public function getPath() {
		if ($this->getData() === FALSE)
			return FALSE;
		return sprintf("/dev/%s/%s", $this->getVGName(), $this->getName());
	}

	/**
	 * Get the size of the logical volume in bytes.
	 * @return The size of the logical volume in bytes as string,
	 * FALSE on failure.
	 */
	public function getSize() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->size;
	}

	/**
	 * Get the capacity in the highest possible unit.
	 * @return The capacity of the logical volume, otherwise FALSE.
	 */
	public function getCapacity() {
		if (FALSE === ($size = $this->getSize()))
			return FALSE;
		return binary_format($size);
	}

	/**
	 * Get the UUID of the array.
	 * @return The UUID of the array, FALSE on failure.
	 */
	public function getUuid() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->uuid;
	}

	/**
	 * Get the list of underlying devices used by this logical volume.
	 * @return A list of underlying devices used by this logical volume,
	 * otherwise FALSE.
	 */
	public function getDevices() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->devices;
	}

	/**
	 * Get the description of the logical volume.
	 * @return The logical volume description, FALSE on failure.
	 */
	public function getDescription() {
		if ($this->getData() === FALSE)
			return FALSE;
		return sprintf(gettext("Logical volume %s [%s, %s]"),
		  $this->getName(), $this->getDeviceFile(), $this->getCapacity());
	}
}
?>
