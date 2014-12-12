<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
	 * Check if the given device file contains the operating system.
	 * @param deviceFile The devicefile to check, e.g. /dev/sda1.
	 * @return Returns TRUE if the device file contains the operating system,
	 *   otherwise FALSE.
	 */
	public static function isRootDeviceFile($deviceFile) {
		if ("/dev/root" == $deviceFile)
			return TRUE;
		$rootDeviceFile = self::getRootDeviceFile();
		return ($rootDeviceFile == $deviceFile) ? TRUE : FALSE;
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
			"iface" => "netstat -i | awk '{print $1}'"
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
		if ($result !== 0)
			return FALSE;
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
 * Helper class that implements functions regarding the power management.
 * References:
 * https://www.kernel.org/doc/Documentation/power/swsusp.txt
 * https://wiki.archlinux.org/index.php/Suspend_and_Hibernate
 * @ingroup api
 */
class OMVPowerManagement extends OMVObject {
	const PM_STATE_NONE = 0x0;
	const PM_STATE_SUSPEND = 0x1;
	const PM_STATE_HIBERNATE = 0x2;
	const PM_STATE_SUSPENDHYBRID = 0x4;

	/**
	 * Checks if the given power management state is supported.
	 * @param state The state to check for.
	 * @return TRUE if the state is supported, if not then FALSE. In case
	 *   of an error NULL is returned.
	 */
	public function isStateSupported($state) {
		$cmdArgs = [];
		switch ($state) {
		case self::PM_STATE_SUSPEND:
			$cmdArgs[] = "--suspend";
			break;
		case self::PM_STATE_HIBERNATE:
			$cmdArgs[] = "--hibernate";
			break;
		case self::PM_STATE_SUSPENDHYBRID:
			$cmdArgs[] = "--suspend-hybrid";
			break;
		}
		$cmd = sprintf("export LANG=C; pm-is-supported %s 2>&1",
		  implode(" ", $cmdArgs));
		@OMVUtil::exec($cmd, $output, $result);
		if (!(($result == 0) || ($result == 1))) {
			$this->setLastError($output);
			return NULL;
		}
		$supported = ($result == 0) ? TRUE : FALSE;
/*
		// In case of PM_STATE_HIBERNATE and PM_STATE_SUSPENDHYBRID the
		// swap space must be greater than the physical memory size to
		// work correctly.
		if ((TRUE === $supported) && (self::PM_STATE_SUSPEND !== $state)) {
			if (FALSE === ($stats = OMVSystem::getMemoryStats()))
				return NULL;
			if ($stats['mem']['total'] > $stats['swap']['total']) {
				$supported = FALSE;
				syslog(LOG_NOTICE, gettext("Hybrid-suspend and hibernate mode are supported but ignored because the RAM size is greater than the available swap space."));
			}
		}
*/
		return $supported;
	}

	/**
	 * Get all supported power management states.
	 * @return The supported power management states.
	 */
	public function getSupportedStates() {
		$states = self::PM_STATE_NONE;
		if (TRUE === $this->isStateSupported(self::PM_STATE_SUSPEND))
			$states |= self::PM_STATE_SUSPEND;
		if (TRUE === $this->isStateSupported(self::PM_STATE_HIBERNATE))
			$states |= self::PM_STATE_HIBERNATE;
		if (TRUE === $this->isStateSupported(self::PM_STATE_SUSPENDHYBRID))
			$states |= self::PM_STATE_SUSPENDHYBRID;
		return $states;
	}

	/**
	 * Put the machine in a sleep state. If suspend to disk (STD) or RAM (STR)
	 * is not supported the system will be shut down. The system will be put
	 * into one of the following state depending on which state is supported:
	 * <ul>
	 * \li Hybrid suspend (STB)
	 * \li Suspend to disk (STD)
	 * \li Suspend to RAM (STR)
	 * \li Shut down and turn of system
	 * </ul>
	 * @return TRUE if command has been executed sucessfully, otherwise
	 *   FALSE.
	 */
	public function standby() {
		$result = FALSE;
		$cmd = "export LANG=C; shutdown -h -P now 2>&1";
		if (TRUE === $this->isStateSupported(self::PM_STATE_SUSPENDHYBRID))
			$cmd = "export LANG=C; pm-suspend-hybrid 2>&1";
		else if (TRUE === $this->isStateSupported(self::PM_STATE_HIBERNATE))
			$cmd = "export LANG=C; pm-hibernate 2>&1";
		else if (TRUE === $this->isStateSupported(self::PM_STATE_SUSPEND))
			$cmd = "export LANG=C; pm-suspend 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Put the machine into suspend to RAM (STR) mode. If this state is not
	 * supported the system will be shut down.
	 * @return TRUE if command has been executed sucessfully, otherwise
	 *   FALSE.
	 */
	public function suspend() {
		$result = FALSE;
		$cmd = "export LANG=C; shutdown -h -P now 2>&1";
		if (TRUE === $this->isStateSupported(self::PM_STATE_SUSPEND))
			$cmd = "export LANG=C; pm-suspend 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Put the machine into suspend to disk (STD) mode. If this state is not
	 * supported the system will be shut down.
	 * @return TRUE if command has been executed sucessfully, otherwise
	 *   FALSE.
	 */
	public function hibernate() {
		$cmd = "export LANG=C; shutdown -h -P now 2>&1";
		if (TRUE === $this->isStateSupported(self::PM_STATE_HIBERNATE))
			$cmd = "export LANG=C; pm-hibernate 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Put the machine into suspend-hybrid (STB) mode. If this state is not
	 * supported the system will be shut down.
	 * @return TRUE if command has been executed sucessfully, otherwise
	 *   FALSE.
	 */
	public function suspendHybrid() {
		$cmd = "export LANG=C; shutdown -h -P now 2>&1";
		if (TRUE === $this->isStateSupported(self::PM_STATE_SUSPENDHYBRID))
			$cmd = "export LANG=C; pm-suspend-hybrid 2>&1";
		OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}
}

/**
 * The generic class that represents a filesystem backend.
 * @ingroup api
 */
abstract class OMVFilesystemBackendAbstract extends OMVObject {
	const PROP_NONE = 0x0;
	const PROP_MNTENT = 0x1;
	const PROP_POSIX_ACL = 0x2;
	const PROP_DISCARD = 0x4;

	protected $type;
	protected $properties = self::PROP_NONE;
	protected $mkfsOptions = "";
	protected $mntOptions = array();

	/**
	 * Get the type of the filesystem, e.g. 'ext3', 'vfat' or 'btrfs'.
	 */
	final public function getType() {
		return strtolower($this->type);
	}

	/**
	 * Get a list of filesystem devices of the given filesystem backend.
	 * Override this method if the filesystem implemented by this backend
	 * is not identified by the block device identification library.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	public function enumerate() {
		return FALSE;
	}

	/**
	 * Check whether the filesystem implemented by this backend is
	 * identified by the block device identification library. If this is
	 * not the case, then the backend must override the \ref enumerate
	 * method.
	 * @return TRUE if the filesystem is identified by the block device
	 *   identification library, otherwise FALSE.
	 */
	public function isBlkidEnumerated() {
		return TRUE;
	}

	/**
	 * Check whether the given filesystem identifier is represented by this
	 * filesystem backend.
	 * @param id The filesystem identifier (e.g. UUID or device path), e.g.
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
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function isTypeOf($id) {
		if (!is_devicefile($id)) {
			// Get the device file containing the file system. This is
			// required for the blkid low-level probing mode.
			$cmd = sprintf("export LANG=C; findfs UUID=%s", $id);
			@OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0)
				return FALSE;
			$id = $output[0];
			unset($output);
		}
		// Get the filesystem type.
		// !!! Note, do not use the '-p' option here. !!!
		$cmd = sprintf("export LANG=C; blkid -o value -s TYPE %s",
		  escapeshellarg($id));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		if (empty($output))
			return FALSE;
		// Compare the filesystem type.
		return ($output[0] == $this->getType()) ? TRUE : FALSE;
	}

	/**
	 * Get the filesystem properties.
	 * @return The properties defined for this filesystem.
	 */
	final public function getProperties() {
		return $this->properties;
	}

	/**
	 * Check whether the filesystem has the given property.
	 * @param property The property to check for, e.g. PROP_POSIX_ACL.
	 * @return TRUE if the given property is set, otherwise FALSE.
	 */
	final public function hasProperty($property) {
		return ($this->properties & $property) === $property;
	}

	/**
	 * Check whether the filesystem supports the discard/TRIM commands to
	 * the underlying block device when blocks are freed.
	 * See https://wiki.archlinux.org/index.php/Solid_State_Drives#TRIM.
	 * @return TRUE if the filesystem supports the discard/TRIM commands,
	 *   otherwise FALSE.
	 */
	public function hasDiscardSupport() {
		return $this->hasProperty(self::PROP_DISCARD);
	}

	/**
	 * Does the filesystem support POSIX ACL.
	 * @return TRUE if the filesystem supports POSIX ACL, otherwise FALSE.
	 */
	public function hasPosixAclSupport() {
		return $this->hasProperty(self::PROP_POSIX_ACL);
	}

	/**
	 * Is the filesystem mounted via /etc/fstab?
	 * @return TRUE if the filesystem is mounted via '/etc/fstab',
	 *   otherwise FALSE.
	 */
	public function hasFstabSupport() {
		return $this->hasProperty(self::PROP_MNTENT);
	}

	/**
	 * Get filesystem mount options used in '/etc/fstab'.
	 * See http://linux.die.net/man/8/mount.
	 * If a storage device is given the result mount option list can contain
	 * specific options for the given storage device.
	 * @param sd A optional storage device object. Defaults to NULL.
	 * @return An array of 'mount' options.
	 */
	public function getFstabMntOptions(OMVStorageDeviceAbstract $sd = null) {
		$options = $this->mntOptions;
		// Append the mount option 'acl' by default if the filesystem
		// supports POSIX ACL.
		if ($this->hasPosixAclSupport())
			$options[] = "acl";
		return $options;
	}

	/**
	 * Get 'mkfs' command options used to create the filesystem.
	 * See http://linux.die.net/man/8/mkfs.
	 * If a storage device is given the result command option list can contain
	 * specific options for the given storage device.
	 * @param sd A optional storage device object. Defaults to NULL.
	 * @return An array of 'mkfs' options.
	 */
	public function getMkfsOptions(OMVStorageDeviceAbstract $sd = null) {
		return $this->mkfsOptions;
	}

	/**
	 * Get the object of the class that represents and implements a filesystem
	 * of this filesystem backend.
	 * @param args The arguments to the class constructor.
	 * @return The object of the class implementing the given filesystem type,
	 *   otherwise NULL.
	 */
	function getImpl($args) {
		return new OMVFilesystem($args);
	}
}

class OMVFilesystemBackendNone extends OMVFilesystemBackendAbstract {
	public function __construct() {
		parent::__construct();
		$this->type = "none";
		$this->properties = self::PROP_MNTENT;
	}
}

class OMVFilesystemBackendExt extends OMVFilesystemBackendAbstract {
	public function __construct() {
		parent::__construct();
		$this->type = "ext";
		$this->properties = self::PROP_MNTENT;
	}
}

class OMVFilesystemBackendExt2 extends OMVFilesystemBackendExt {
	public function __construct() {
		parent::__construct();
		$this->type = "ext2";
		$this->properties = self::PROP_MNTENT | self::PROP_POSIX_ACL;
		$this->mntOptions = explode(",", $GLOBALS['OMV_FSTAB_MNTOPS_EXT2']);
	}
}

class OMVFilesystemBackendExt3 extends OMVFilesystemBackendExt2 {
	public function __construct() {
		parent::__construct();
		$this->type = "ext3";
		$this->properties = self::PROP_MNTENT | self::PROP_POSIX_ACL |
		  self::PROP_DISCARD;
		$this->mkfsOptions = $GLOBALS['OMV_MKFS_OPTIONS_EXT3'];
		$this->mntOptions = explode(",", $GLOBALS['OMV_FSTAB_MNTOPS_EXT3']);
	}
}

class OMVFilesystemBackendExt4 extends OMVFilesystemBackendExt3 {
	public function __construct() {
		parent::__construct();
		$this->type = "ext4";
		$this->mkfsOptions = $GLOBALS['OMV_MKFS_OPTIONS_EXT4'];
		$this->mntOptions = explode(",", $GLOBALS['OMV_FSTAB_MNTOPS_EXT4']);
	}

	public function getMkfsOptions(OMVStorageDeviceAbstract $sd = null) {
		$options = parent::getMkfsOptions($sd);
		if (is_a($sd, "OMVStorageDeviceMdadm")) {
			// Append extra options for mdadm software RAID devices.
			// http://article.gmane.org/gmane.comp.file-systems.ext4/33938
			// http://serverfault.com/a/419628
			$options .= sprintf(" %s",
			  $GLOBALS['OMV_MKFS_EXTRAOPTIONS_EXT4_MDADM']);
		}
		return $options;
	}
}

class OMVFilesystemBackendXfs extends OMVFilesystemBackendAbstract {
	public function __construct() {
		parent::__construct();
		$this->type = "xfs";
		$this->properties = self::PROP_MNTENT | self::PROP_POSIX_ACL |
		  self::PROP_DISCARD;
		$this->mkfsOptions = $GLOBALS['OMV_MKFS_OPTIONS_XFS'];
		$this->mntOptions = explode(",", $GLOBALS['OMV_FSTAB_MNTOPS_XFS']);
	}
}

class OMVFilesystemBackendJfs extends OMVFilesystemBackendAbstract {
	public function __construct() {
		parent::__construct();
		$this->type = "jfs";
		$this->properties = self::PROP_MNTENT | self::PROP_POSIX_ACL |
		  self::PROP_DISCARD;
		$this->mkfsOptions = $GLOBALS['OMV_MKFS_OPTIONS_JFS'];
		$this->mntOptions = explode(",", $GLOBALS['OMV_FSTAB_MNTOPS_JFS']);
	}
}

class OMVFilesystemBackendIso9660 extends OMVFilesystemBackendAbstract {
	public function __construct() {
		parent::__construct();
		$this->type = "iso9660";
		$this->properties = self::PROP_MNTENT;
	}
}

class OMVFilesystemBackendMsdos extends OMVFilesystemBackendAbstract {
	public function __construct() {
		parent::__construct();
		$this->type = "msdos";
		$this->properties = self::PROP_MNTENT;
	}
}

class OMVFilesystemBackendVfat extends OMVFilesystemBackendAbstract {
	public function __construct() {
		parent::__construct();
		$this->type = "vfat";
		$this->properties = self::PROP_MNTENT | self::PROP_DISCARD;
		$this->mntOptions = explode(",", $GLOBALS['OMV_FSTAB_MNTOPS_VFAT']);
	}
}

class OMVFilesystemBackendNtfs extends OMVFilesystemBackendAbstract {
	public function __construct() {
		parent::__construct();
		$this->type = "ntfs";
		$this->properties = self::PROP_MNTENT;
		$this->mntOptions = explode(",", $GLOBALS['OMV_FSTAB_MNTOPS_NTFS']);
	}
}

class OMVFilesystemBackendUmsdos extends OMVFilesystemBackendAbstract {
	public function __construct() {
		parent::__construct();
		$this->type = "umsdos";
		$this->properties = self::PROP_MNTENT;
	}
}

class OMVFilesystemBackendUdf extends OMVFilesystemBackendAbstract {
	public function __construct() {
		parent::__construct();
		$this->type = "udf";
		$this->properties = self::PROP_MNTENT;
	}
}

class OMVFilesystemBackendUfs extends OMVFilesystemBackendAbstract {
	public function __construct() {
		parent::__construct();
		$this->type = "ufs";
		$this->properties = self::PROP_MNTENT;
	}
}

class OMVFilesystemBackendReiserfs extends OMVFilesystemBackendAbstract {
	public function __construct() {
		parent::__construct();
		$this->type = "reiserfs";
		$this->properties = self::PROP_MNTENT | self::PROP_POSIX_ACL;
	}
}

class OMVFilesystemBackendBtrfs extends OMVFilesystemBackendAbstract {
	public function __construct() {
		parent::__construct();
		$this->type = "btrfs";
		$this->properties = self::PROP_MNTENT | self::PROP_POSIX_ACL |
		  self::PROP_DISCARD;
		$this->mntOptions = explode(",", $GLOBALS['OMV_FSTAB_MNTOPS_BTRFS']);
	}

	public function getFstabMntOptions(OMVStorageDeviceAbstract $sd = null) {
		$options = parent::getFstabMntOptions($sd);
		if (!is_null($sd)) {
			if (!$sd->isRotational())
				$options[] = "ssd";
		}
		return $options;
	}
}

class OMVFilesystemBackendHfsplus extends OMVFilesystemBackendAbstract {
	public function __construct() {
		parent::__construct();
		$this->type = "hfsplus";
		$this->properties = self::PROP_MNTENT;
		$this->mntOptions = explode(",", $GLOBALS['OMV_FSTAB_MNTOPS_HFSPLUS']);
	}
}

/**
 * Helper class that implements functions regarding filesystems.
 * @ingroup api
 */
class OMVFilesystems extends OMVObject {
	static private $backends = array();

	/**
	 * Register a filesystem backend.
	 * @param backend The filesystem backend object to register.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	final static function registerBackend(OMVFilesystemBackendAbstract $backend) {
		if (!isset($backend))
			return FALSE;
		self::$backends[] = $backend;
		return TRUE;
	}

	/**
	 * Get the backend of the given filesystem.
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
	 *   \li tank/multimedia/movies
	 *   </ul>
	 * @return The backend that implements the given filesystem, otherwise
	 *   NULL.
	 */
	final static function getBackend($id) {
		$result = NULL;
		$enums = self::enumerate();
		foreach ($enums as $enumk => $enumv) {
			$found = FALSE;
			if (is_devicefile($id)) {
				// Compare the device file name. Use the canonical device
				// file for an additional check.
				$found = (($id == $enumv['devicefile']) ||
				  (realpath($id) == $enumv['devicefile']));
			} else if (is_fs_uuid($id)) {
				// Compare the filesystem UUID.
				$found = ($id == $enumv['uuid']);
			} else {
				$found = ($id == $enumv['devicefile']);
			}
			if (TRUE === $found) {
				$result = self::getBackendByType($enumv['type']);
				break;
			}
		}
		/* Slow, but the more correct implementation.
		foreach (self::$backends as $backendk => $backendv) {
			if (FALSE === $backendv->isTypeOf($id))
				continue;
			$result = $backendv;
			break;
		}
		*/
		return $result;
	}

	/**
	 * Get the backend of the given filesystem type.
	 * @param type Specifies the filesystem type, e.g. 'ext3', 'vfat', ...
	 * @return The backend that implements the given filesystem, otherwise
	 *   NULL.
	 */
	final static function getBackendByType($type) {
		$result = NULL;
		foreach (self::$backends as $backendk => $backendv) {
			if (strtolower($type) !== $backendv->getType())
				continue;
			$result = $backendv;
			break;
		}
		return $result;
	}

	/**
	 * Check whether the given file system type is supported.
	 * @param type The file system type, e.g. 'ext4', 'iso9660', ...
	 * @return TRUE if the file system is supported, otherwise FALSE.
	 */
	final static function isSupported($type) {
		if (NULL == ($backend = self::getBackendByType($type)))
			return FALSE;
		return TRUE;
	}

	/**
	 * Get a list of all detected file systems.
	 * @deprecated
	 * @return A list of file systems, otherwise FALSE. The array elements
	 *   contain the fields: \em devicefile, \em uuid, \em label and \em type.
	 *   The devicefile is used as key to access the array elements.
	 */
	public static function get() {
		return $this->enumerate();
	}

	/**
	 * Enumerate all available/detected file systems.
	 * @return A list of filesystems, otherwise FALSE. The array elements
	 *   contain the fields: \em devicefile, \em uuid, \em label and \em type.
	 *   The devicefile is used as key to access the array elements.
	 */
	public static function enumerate() {
		// Get all filesystems that are identified by the block device
		// identification library.
		// !!! Note !!!
		// Filesystems that are identified by the block device identification
		// library are processed in one step to increase performance. Otherwise
		// each backend must implement the \ref enumerate method and call the
		// command 'blkid -t TYPE=<xxx> -o full'.
		$cmd = "export LANG=C; blkid -o full";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		$result = array();
		foreach ($output as $outputk => $outputv) {
			// Parse command output:
			// /dev/sdb: UUID="e6a0f61e-d969-ffcb-d4c5-06e7711285a3" LABEL="dhcppc4:0" TYPE="linux_raid_member"
			// /dev/sdg1: LABEL="test" UUID="d19bcea0-a323-4aea-9791-c6578180c129" TYPE="jfs"
			// /dev/sda5: UUID="902775c7-8250-4630-81eb-2602ac65e136" TYPE="swap"
			// /dev/sda1: LABEL="Ubuntu" UUID="4aa77ba3-beb6-45dc-b2c7-e77cffd6c6fd" TYPE="ext4"
			// /dev/sdb1: UUID="A27E16CD7E1699DD" TYPE="ntfs"
			// /dev/sdc1: UUID="1a03b79f-d347-4a24-8ae2-62f1aa9a4554" TYPE="ext4"
			// /dev/sdd1: UUID="cb8f7844-362f-4151-8e9e-3888aab723c2" TYPE="ext4"
			// /dev/sda3: UUID="fdca09a8-cbc8-432f-9cba-ce033cb59139" TYPE="ext4" LABEL="Debian"
			// /dev/sr0: LABEL="Debian 7.5.0 amd64 1" TYPE="iso9660"
			$regex = '/^(\S+): (.+)$/i';
			if (1 !== preg_match($regex, $outputv, $matches))
				continue;
			// Set default values and extract key/value pairs.
			$data = array(
				"devicefile" => $matches[1],
				"uuid" => "",
				"label" => "",
				"type" => ""
			);
			$parts = preg_split("/(\S+=\\\"[^\\\"]+\\\")|[\s]+/", $matches[2],
			  -1, PREG_SPLIT_NO_EMPTY | PREG_SPLIT_DELIM_CAPTURE);
			foreach ($parts as $partk => $partv) {
				$keyValue = explode("=", $partv);
				if (count($keyValue) != 2)
					continue;
				$data[strtolower($keyValue[0])] = substr($keyValue[1], 1, -1);
			}
			// Is this filesystem processed via blkid?
			if (NULL == ($backend = self::getBackendByType($data['type'])))
				continue;
			if (FALSE == $backend->isBlkidEnumerated())
				continue;
			// Append filesystem.
			$result[$data['devicefile']] = $data;
		}
		// Get the filesystems that are not identified by the block device
		// identification library.
		foreach (self::$backends as $backendk => $backendv) {
			if (TRUE === $backendv->isBlkidEnumerated())
				continue;
			if (FALSE === ($data = $backendv->enumerate()))
				return FALSE;
			if (empty($data))
				continue;
			$result = array_merge($result, $data);
		}
		return $result;
	}

	/**
	 * Get the object of the class implementing the given filesystem.
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
	 * @return The object of the class implementing the given filesystem,
	 *   otherwise NULL.
	 */
	public static function getImpl($id) {
		if (NULL == ($backend = self::getBackend($id)))
			return NULL;
		return $backend->getImpl($id);
	}
}

/**
 * Factory to create objects of filesystem classes.
 * @ingroup api
 */
class OMVFilesystemFactory extends OMVObject {
	/**
	 * Get the object of the class implementing the given filesystem.
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
	 * @return The object of the class implementing the given filesystem,
	 *   otherwise NULL.
	 */
	public static function get($id) {
		return OMVFilesystems::getImpl($id);
	}
}

/**
 * Abstract class implementing generic filesystems.
 * @ingroup api
 */
class OMVFilesystemAbstract extends OMVObject {
	protected $uuid = "";
	protected $type = "";
	protected $deviceFile = "";
	protected $label = "";
	protected $partEntry = NULL;
	protected $usage = "";
	private $dataCached = FALSE;

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

	/**
	 * Get the filesystem detailed information.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if ($this->dataCached !== FALSE)
			return TRUE;

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
			$cmd = sprintf("export LANG=C; findfs UUID=%s", $this->uuid);
			@OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				$this->setLastError($output);
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
		if (1 == preg_match("/^\/dev\/disk\/by-.+$/", $this->deviceFile))
			$this->deviceFile = realpath($this->deviceFile);

		// Get the filesystem details.
		$cmd = sprintf("export LANG=C; blkid -p -o full %s", escapeshellarg(
		  $this->deviceFile));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}

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
		$data = array(
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
		);
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
		$this->partEntry = array(
			"scheme" => $data['part_entry_scheme'],
			"uuid" => $data['part_entry_uuid'],
			"type" => $data['part_entry_type'],
			"flags" => $data['part_entry_flags'],
			"number" => $data['part_entry_number'],
			"offset" => $data['part_entry_offset'],
			"size" => $data['part_entry_size'],
			"disk" => $data['part_entry_disk']
		);

		// Set flag to mark information has been successfully read.
		$this->dataCached = TRUE;

		return TRUE;
	}

	/**
	 * Refresh the cached information.
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
	 * Check if the filesystem has an UUID, e.g. <ul>
	 * \li 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
	 * \li 7A48-BA97 (FAT)
	 * \li 2ED43920D438EC29 (NTFS)
	 * </ul>
	 * @see http://wiki.ubuntuusers.de/UUID
	 * @return Returns TRUE if the filesystem has an UUID, otherwise FALSE.
	 */
	public function hasUuid() {
		if (FALSE === ($uuid = $this->getUuid()))
			return FALSE;
		return is_fs_uuid($uuid);
	}

	/**
	 * Get the UUID of the filesystem.
	 * @see http://wiki.ubuntuusers.de/UUID
	 * @return The UUID of the filesystem, otherwise FALSE.
	 */
	public function getUuid() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->uuid;
	}

	/**
	 * Check if the filesystem has a label.
	 * @return Returns TRUE if the filesystem has a label, otherwise FALSE.
	 */
	public function hasLabel() {
		if (FALSE === ($label = $this->getLabel()))
			return FALSE;
		return !empty($label);
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
	 * Get the filesystem type, e.g. 'ext3' or 'vfat'.
	 * @return The filesystem type, otherwise FALSE.
	 */
	public function getType() {
		if ($this->getData() === FALSE)
			return FALSE;
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
	 * @return The filesystem type, otherwise FALSE.
	 */
	public function getPartitionScheme() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->partEntry['scheme'];
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
	 * Get the partition entry information.
	 * @return An array with the fields \em scheme, \em uuid, \em type,
	 *   \em flags, \em number, \em offset, \em size and \em disk,
	 *   otherwise FALSE.
	 */
	public function getPartitionEntry() {
		if ($this->getData() === FALSE)
			return FALSE;
		return $this->partEntry;
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
	 * Get the canonical device file, e.g. <ul>
	 * \li /dev/root -> /dev/sde1
	 * \li /dev/disk/by-uuid/4B04EA317E4AA567 -> /dev/sdd1
	 * </ul>
	 */
	public function getCanonicalDeviceFile() {
		return realpath($this->deviceFile);
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
		if ($this->getData() === FALSE)
			return FALSE;
		$deviceFile = $this->getDeviceFile();
		if ($this->hasUuid()) {
			$deviceFileByUuid = sprintf("/dev/disk/by-uuid/%s",
			  $this->getUuid());
			if (is_block($deviceFileByUuid))
				$deviceFile = $deviceFileByUuid;
		}
		return $deviceFile;
	}

	/**
	 * Get the device file of the storage device containing this
	 * file system. Example:
	 * <ul>
	 * \li /dev/sdb1 => /dev/sdb
	 * \li /dev/cciss/c0d0p2 => /dev/cciss/c0d0
	 * </ul>
	 * @return The device file of the underlying storage device,
	 *   otherwise FALSE.
	 */
	public function getStorageDeviceFile() {
		if ($this->getData() === FALSE)
			return FALSE;
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
		if (NULL === ($backend = OMVStorageDevices::getBackend($deviceFile)))
			return FALSE;
		return $backend->baseDeviceFile($deviceFile);
	}

	/**
	 * Get the filesystem block size.
	 * @return The block size, otherwise FALSE.
	 */
	public function getBlockSize() {
		if ($this->getData() === FALSE)
			return FALSE;
		$cmd = sprintf("export LANG=C; blockdev --getbsz %s",
		  escapeshellarg($this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return intval($output[0]);
	}

	/**
	 * Get the mount point of the given filesystem.
	 * @return The mountpoint of the filesystem or FALSE.
	 */
	public function getMountPoint() {
		if ($this->getData() === FALSE)
			return FALSE;
		$cmd = sprintf("export LANG=C; findmnt -cnrv -o SOURCE,UUID,".
		  "TARGET 2>&1");
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
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
	 */
	public function getStatistics() {
		if ($this->getData() === FALSE)
			return FALSE;
		// Get the mount point of the filesystem.
		if (FALSE === ($mountPoint = $this->getMountPoint()))
			return FALSE;
		@OMVUtil::exec("export LANG=C; df -PT", $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
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
			$result = array(
				"devicefile" => $this->deviceFile,
				"type" => $matches[1],
				"blocks" => $matches[2],
				"size" => bcmul($matches[2], "1024", 0),
				"used" => binary_convert($matches[3], "KiB", "B"),
				"available" => binary_convert($matches[4], "KiB", "B"),
				"percentage" => intval(trim($matches[5], "%")),
				"mountpoint" => $matches[6]
			);
		}
		return $result;
	}

	/**
	 * Does the filesystem support POSIX ACL.
	 * @return TRUE if the filesystem supports POSIX ACL, otherwise FALSE.
	 */
	public function hasPosixAclSupport() {
		if ($this->getData() === FALSE)
			return FALSE;
		if (NULL == ($backend = OMVFilesystems::getBackendByType(
		  $this->getType())))
			return FALSE;
		// The list of POSIX ACL supporting filesystems. See:
		// - http://de.wikipedia.org/wiki/Access_Control_List
		// - http://www.suse.de/~agruen/acl/linux-acls/online
		return $backend->hasPosixAclSupport();
	}

	/**
	 * Check if a filesystem is mounted.
	 * @return TRUE if the filesystem is mounted, otherwise FALSE.
	 */
	public function isMounted() {
		if ($this->getData() === FALSE)
			return FALSE;
		$cmd = sprintf("export LANG=C; findmnt -cnrv -o SOURCE,UUID 2>&1");
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
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
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function mount($options = "") {
		$cmdArgs = array("-v");
		if (!empty($options)) {
			if (!is_array($options))
				$options = array($options);
			$cmdArgs[] = sprintf("-o %s", implode(",", $options));
		}
		if (TRUE === $this->hasUuid())
			$cmdArgs[] = sprintf("-U %s", $this->getUuid());
		else
			$cmdArgs[] = $this->getDeviceFile();
		$cmd = sprintf("export LANG=C; mount %s 2>&1", implode(" ", $cmdArgs));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
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
		if (TRUE === $force)
			$cmdArgs[] = "-f";
		if (TRUE === $lazy)
			$cmdArgs[] = "-l";
		$cmdArgs[] = $this->getDeviceFile();
		$cmd = sprintf("export LANG=C; umount %s 2>&1", implode(" ", $cmdArgs));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
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
		return sprintf("%s/%s", $GLOBALS['OMV_MOUNT_DIR'], $uuid);
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
		$cmd = sprintf("export LANG=C; blkid | grep -E '^%s[0-9]*:.+$'",
		  $deviceFile);
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		return TRUE;
	}
}

/**
 * Class to get detailed information about a filesystem.
 * @ingroup api
 */
class OMVFilesystem extends OMVFilesystemAbstract {
	// Nothing to do here.
}

/**
 * A class that implements a mount point.
 * @ingroup api
 */
class OMVMntEnt extends OMVObject {
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
	 * Get the device.
	 * @return The device, otherwise FALSE.
	 */
	public function getDevice() {
		return empty($this->device) ? FALSE : $this->device;
	}

	/**
	 * Get the mount point.
	 * @return The mount point of the filesystem or FALSE.
	 */
	public function getMountPoint() {
		return empty($this->dir) ? FALSE : $this->dir;
	}

	/**
	 * Mount the given file system.
	 * @param options Additional mount options. Defaults to "".
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function mount($options = "") {
		$cmdArgs = array("-v");
		if (!empty($options)) {
			if (!is_array($options))
				$options = array($options);
			$cmdArgs[] = sprintf("-o %s", implode(",", $options));
		}
		$cmdArgs[] = escapeshellarg($this->dir);
		$cmd = sprintf("export LANG=C; mount %s 2>&1", implode(" ", $cmdArgs));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Unmount the given file system.
	 * @param force Set to TRUE to force unmount. Defaults to FALSE.
	 * @param lazy Set to TRUE to lazy unmount. Defaults to FALSE.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function umount($force = FALSE, $lazy = FALSE) {
		$cmdArgs = array("-v");
		if (TRUE === $force)
			$cmdArgs[] = "-f";
		if (TRUE === $lazy)
			$cmdArgs[] = "-l";
		$cmdArgs[] = escapeshellarg($this->dir);
		$cmd = sprintf("export LANG=C; umount %s 2>&1", implode(" ", $cmdArgs));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Create the mount point of the file system.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function mkDir() {
		if (is_dir($this->dir))
			return TRUE;
		return mkdir($this->dir, 0700, TRUE);
	}

	/**
	 * Unlink the mount point of the file system.
	 * @param force Set to TRUE to ignore nonexistent files. Defaults to TRUE.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function unlinkDir($force = TRUE) {
		$cmd = sprintf("export LANG=C; rm -rf %s 2>&1", escapeshellarg(
		  $this->dir));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
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
		$cmd = sprintf("export LANG=C; mountpoint %s 2>&1", escapeshellarg(
		  $this->dir));
		@OMVUtil::exec($cmd, $output, $result);
		return ($result == 0) ? TRUE : FALSE;
	}

	/**
	 * Get the OMVFilesystem object for the given mount point.
	 * @param dir The file system path prefix.
	 * @return The class object or NULL on failure.
	 */
	public static function getFilesystemByDir($dir) {
		$result = NULL;
		foreach (file("/proc/mounts") as $rowk => $rowv) {
			// Parse command output:
			// rootfs / rootfs rw 0 0
			// none /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0
			// none /proc proc rw,nosuid,nodev,noexec,relatime 0 0
			// tmpfs /dev/shm tmpfs rw,nosuid,nodev,relatime 0 0
			// /dev/sde1 /home/ftp/data ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			// /dev/sde1 /media/46607e80-dc8e-427e-9939-7909985dfe5b ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			$matches = explode(" ", $rowv);
			if ($matches[1] == $dir) {
				$result = OMVFilesystemFactory::get($matches[0]);
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
		foreach (file("/proc/mounts") as $rowk => $rowv) {
			// Parse command output:
			// rootfs / rootfs rw 0 0
			// none /sys sysfs rw,nosuid,nodev,noexec,relatime 0 0
			// none /proc proc rw,nosuid,nodev,noexec,relatime 0 0
			// tmpfs /dev/shm tmpfs rw,nosuid,nodev,relatime 0 0
			// /dev/sde1 /home/ftp/data ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			// /dev/sde1 /media/46607e80-dc8e-427e-9939-7909985dfe5b ext4 rw,noexec,relatime,user_xattr,acl,barrier=1,data=ordered,jqfmt=vfsv0,usrjquota=aquota.user,grpjquota=aquota.group 0 0
			$matches = explode(" ", $rowv);
			if ($matches[0] == $name) {
				$result = OMVFilesystemFactory::get($matches[0]);
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
	 * @param canonical If set to TRUE the canonical device file will
	 *   be used. Defaults to FALSE.
	 * @return The device name.
	 */
	function getDeviceName($canonical = FALSE);

	/**
	 * Get the device number, e.g. 8:17.
	 * @link See https://www.kernel.org/doc/Documentation/devices.txt for
	 *   more information. @endlink
	 * @return The device number as string or FALSE on failure.
	 */
	function getDeviceNumber();

	/**
	 * Get the major device number.
	 * @link See https://www.kernel.org/doc/Documentation/devices.txt for
	 *   more information. @endlink
	 * @return The major device number or FALSE on failure.
	 */
	function getMajor();

	/**
	 * Get the minor device number.
	 * @link See https://www.kernel.org/doc/Documentation/devices.txt for
	 *   more information. @endlink
	 * @return The minor device number or FALSE on failure.
	 */
	function getMinor();

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

	/**
	 * Constructor
	 * @param deviceFile The device file.
	 */
	public function __construct($deviceFile) {
		$this->deviceFile = $deviceFile;
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
	 * Get the canonical device file, e.g. /dev/mapper/vg0-lv0 -> /dev/dm-0
	 */
	public function getCanonicalDeviceFile() {
		return realpath($this->deviceFile);
	}

	/**
	 * See interface definition.
	 */
	public function getDeviceName($canonical = FALSE) {
		// Get the device file and extract the name, e.g. /dev/sda => sda.
		$deviceName = str_replace("/dev/", "", !$canonical ?
		  $this->getDeviceFile() : $this->getCanonicalDeviceFile());
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

	/**
	 * See interface definition.
	 */
	public function getDeviceNumber() {
		// Get the canonical device name, e.g.
		// /dev/root => /dev/sda1
		$deviceName = $this->getDeviceName(TRUE);
		// Get the device number via sysfs.
		$filename = sprintf("/sys/class/block/%s/dev", $deviceName);
		if (!file_exists($filename))
			return FALSE;
		return trim(file_get_contents($filename));
	}

	/**
	 * See interface definition.
	 */
	public function getMajor() {
		if (FALSE === ($devNum = $this->getDeviceNumber()))
			return FALSE;
		$devNumParts = explode(":", $devNum);
		return intval($devNumParts[0]);
	}

	/**
	 * See interface definition.
	 */
	public function getMinor() {
		if (FALSE === ($devNum = $this->getDeviceNumber()))
			return FALSE;
		$devNumParts = explode(":", $devNum);
		return intval($devNumParts[1]);
	}
}

/**
 * This class implements an interface to a generic Linux block device.
 * @ingroup api
 */
class OMVBlockDevice extends OMVBlockDeviceAbstract {
	/**
	 * See interface definition.
	 */
	public function getDescription() {
		return sprintf("Block device %s [%s]", $this->getDeviceName(),
		  $this->getDeviceNumber());
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
	 * \li /dev/disk/by-id/scsi-SATA_IBM-DHEA-36481_SG0SGF08038
	 * \li /dev/disk/by-id/ata-Hitachi_HDT725032VLA360_VFD200R2CWB7ML
	 * @return The device path (/dev/disk/by-id/xxx) if available,
	 *   otherwise /dev/xxx will be returned. In case of an error FALSE
	 *   will be returned.
	 */
	public function getDeviceFileById() {
		if (!$this->deviceFileById) {
			// Set the device file by default.
			$this->deviceFileById = $this->deviceFile;
			// Now try to find the device file by-id for the given device.
			$path = "/dev/disk/by-id";
			if (is_dir($path)) {
				$deviceFilesById = array();
				$deviceName = $this->getDeviceName();
				// Process directory:
				// ata-WDC_WD2002FAEX-007BA0_WD-WCAY01065572 -> ../../sda
				// scsi-SATA_WDC_WD2002FAEX-_WD-WCAY01065572 -> ../../sda
				// wwn-0x50014ee25d4cdecd -> ../../sda
				// usb-Generic-_SM_xD_Picture_058F63626420-0:2 -> ../../sde
				// usb-Generic-_Compact_Flash_058F63626420-0:1 -> ../../sdd
				// ata-ST31000528AS_7VS3K6EY-part5 -> ../../sdb5
				// ata-ST31000528AS_7VS3K6EY-part2 -> ../../sdb2
				// ata-ST31000528AS_7VS3K6EY-part1 -> ../../sdb1
				// ata-ST31000528AS_7VS3K6EY -> ../../sdb
				$dir = new DirectoryIterator($path);
				foreach ($dir as $item) {
					// Skip everything that is not a symlink.
					if ($item->isDot() || !$item->isLink())
						continue;
					// Skip files that does not match the device name,
					// e.g. sdb or sdg.
					if (basename($item->getLinkTarget()) !== $deviceName)
						continue;
					$deviceFilesById[] = $item->getPathname();
				}
				if (!empty($deviceFilesById)) {
					// Sort the device files in the following order:
					// s=[s]csi-xxx
					// a=[a]ta-xxx
					// x=any other device file name
					// fn=call strnatcmp function
					// |      | $a=s | $a=a | $a=x |
					// |------|------|------|------|
					// | $b=s |  fn  |  -1  |  -1  |
					// | $b=a |   1  |  fn  |  -1  |
					// | $b=x |   1  |   1  |  fn  |
					//
					// Example:
					// xxx...
					// yyy...
					// wwn-0x50014ee25d4cdecd
					// ata-WDC_WD2002FAEX-007BA0_WD-WCAY01065572
					// scsi-SATA_WDC_WD2002FAEX-_WD-WCAY01065572
					usort($deviceFilesById, create_function('$a, $b',
						'if (($a[0] == "s") && ($b[0] == "a")) return 1;'.
						'if (($a[0] == "s") && (($b[0] !== "a") && ($b[0] !== "s"))) return 1;'.
						'if (($a[0] == "a") && ($b[0] == "s")) return -1;'.
						'if (($a[0] == "a") && (($b[0] !== "a") && ($b[0] !== "s"))) return 1;'.
						'if ((($a[0] !== "a") && ($a[0] !== "s")) && ($b[0] == "s")) return -1;'.
						'if ((($a[0] !== "a") && ($a[0] !== "s")) && ($b[0] == "a")) return -1;'.
						'return strnatcmp($a, $b);'));
					$this->deviceFileById = array_pop($deviceFilesById);
				}
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
			if($result !== 0) {
				$this->setLastError($output);
				return FALSE;
			}
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
				$this->setLastError($output);
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
				$this->setLastError($output);
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
	 * Check if the device is of rotational type or non-rotational type.
	 * See https://www.kernel.org/doc/Documentation/block/queue-sysfs.txt
	 * @return TRUE if device is of rotational type, otherwise FALSE.
	 */
	public function isRotational() {
		$filename = sprintf("/sys/block/%s/queue/rotational",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return TRUE; // Assume device is rotational if file doesn't exist.
		return (trim(file_get_contents($filename)) == "1") ? TRUE : FALSE;
	}

	/**
	 * Check if the given device is a hardware/software RAID device.
	 * @return TRUE if the device is a hardware/software RAID, otherwise FALSE.
	 */
	public function isRaid() {
		return FALSE;
	}

	/**
	 * Check if the given device is an USB device.
	 * @return TRUE if the device is a connected via USB, otherwise FALSE.
	 */
	public function isUsb() {
		// Identify USB devices via 'ID_BUS=usb'.
		if (FALSE !== ($property = $this->getUdevProperty("ID_BUS"))) {
			if (0 == strcasecmp("usb", $property))
				return TRUE;
		}
		// Identify USB devices via 'ID_PATH=xxx-usb-xxx'.
		// Example:
		// ID_PATH=pci-0000:02:02.0-usb-0:1:1.0-scsi-0:0:0:0
		// ID_PATH=pci-0000:00:12.2-usb-0:3:1.0-scsi-0:0:0:0
		if (FALSE !== ($property = $this->getUdevProperty("ID_PATH"))) {
			if (1 == preg_match('/^.+-usb-.+$/i', $property))
				return TRUE;
		}
		return FALSE;
	}

	/**
	 * Check if the given device is connected via ATA.
	 * @return TRUE if the device is connected via ATA, otherwise FALSE.
	 */
	public function isAta() {
		if(FALSE === ($property = $this->getUdevProperty("ID_BUS")))
			return FALSE;
		return (0 == strcasecmp("ata", $property)) ? TRUE : FALSE;
	}

	/**
	 * Queries the udev database for device information stored in the udev
	 * database.
	 * @param id The name of the property, e.g. ID_VENDOR, ID_MODEL or
	 *   ID_SERIAL_SHORT.
	 * @return The requested property, otherwise FALSE.
	 */
	protected function getUdevProperty($id) {
		if ($this->udevInfo === FALSE) {
			$cmd = sprintf("export LANG=C; udevadm info --query=property ".
			  "--name=%s", escapeshellarg($this->getDeviceFile()));
			@OMVUtil::exec($cmd, $output, $result);
			if ($result !== 0) {
				$this->setLastError($output);
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
			//
			// DEVLINKS=/dev/disk/by-id/ata-IBM-DTTA-341050_WF0WABH1579 /dev/disk/by-id/scsi-SATA_IBM-DTTA-341050_WF0WABH1579 /dev/disk/by-path/pci-0000:00:02.5-scsi-0:0:0:0
			// DEVNAME=/dev/sda
			// DEVPATH=/devices/pci0000:00/0000:00:02.5/host0/target0:0:0/0:0:0:0/block/sda
			// DEVTYPE=disk
			// ID_ATA=1
			// ID_ATA_FEATURE_SET_HPA=1
			// ID_ATA_FEATURE_SET_HPA_ENABLED=1
			// ID_ATA_FEATURE_SET_PM=1
			// ID_ATA_FEATURE_SET_PM_ENABLED=1
			// ID_ATA_FEATURE_SET_SECURITY=1
			// ID_ATA_FEATURE_SET_SECURITY_ENABLED=0
			// ID_ATA_FEATURE_SET_SECURITY_ERASE_UNIT_MIN=22
			// ID_ATA_FEATURE_SET_SMART=1
			// ID_ATA_FEATURE_SET_SMART_ENABLED=0
			// ID_ATA_WRITE_CACHE=1
			// ID_ATA_WRITE_CACHE_ENABLED=1
			// ID_BUS=ata
			// ID_MODEL=IBM-DTTA-351010
			// ID_MODEL_ENC=IBM-DTTA-351010\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20
			// ID_PART_TABLE_TYPE=dos
			// ID_PATH=pci-0000:00:02.5-scsi-0:0:0:0
			// ID_PATH_TAG=pci-0000_00_02_5-scsi-0_0_0_0
			// ID_REVISION=T56OA7A3
			// ID_SCSI_COMPAT=SATA_IBM-DTTA-341050_WF0WABH1579
			// ID_SERIAL=IBM-DTTA-341050_WF0WABH1579
			// ID_SERIAL_SHORT=WF0WFJH1486
			// ID_TYPE=disk
			// MAJOR=8
			// MINOR=0
			// SUBSYSTEM=block
			// UDEV_LOG=3
			// USEC_INITIALIZED=16872806
			foreach ($output as $row) {
				$data = explode("=", $row);
				if (empty($data))
					continue;
				$this->udevInfo[$data[0]] = $data[1];
			}
		}
		if (!array_key_exists($id, $this->udevInfo))
			return FALSE;
		return $this->udevInfo[$id];
	}

	/**
	 * Check if the given device has S.M.A.R.T. support.
	 * @return TRUE if the device supports S.M.A.R.T., otherwise FALSE.
	 */
	public function hasSMART() {
		return ($this instanceof OMVStorageDeviceSMARTAbstract) ? TRUE : FALSE;
	}
}

/**
 * Abstract class implementing generic storage devices that support the
 * S.M.A.R.T. AT Attachment (ATA) interface.
 * @ingroup api
 */
abstract class OMVStorageDeviceSMARTAbstract extends OMVStorageDeviceAbstract {
	protected $smartDeviceType = "";

	/**
	 * Set the device type required by the smartctl command.
	 * @return Returns the device type.
	 */
	public function setSMARTDeviceType($type) {
		$this->smartDeviceType = $type;
		return $this->smartDeviceType;
	}

	/**
	 * Identify the device type required by the smartctl command.
	 * @return Returns the identified device type or an empty string.
	 */
	public function getSMARTDeviceType() {
		return $this->smartDeviceType;
	}

	public function getSMARTInformation() {
		return new OMVATASMARTInformation($this->getDeviceFile(),
		  $this->getSMARTDeviceType());
	}
}

/**
 * The generic class that represents a storage device backend.
 * @ingroup api
 */
abstract class OMVStorageDeviceBackendAbstract extends OMVObject {
	/**
	 * Get the type of the storage device backend, e.g.
	 * OMV_STORAGE_DEVICE_TYPE_HDD, OMV_STORAGE_DEVICE_TYPE_HBA, ...
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
	public function enumerateSlaves() {
		return array();
	}

	/**
	 * Check whether the given device is represented by this storage
	 * device backend.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
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
	public function isTypeOf($deviceFile) {
		return FALSE;
	}

	/**
	 * Get the object of the class that represents and implements a device
	 * of this storage device backend.
	 * @param args The arguments to the class constructor.
	 * @return The object of the class implementing the given device type,
	 *   otherwise NULL.
	 */
	public function getImpl($args) {
		return NULL;
	}

	/**
	 * Returns base device file by stripping the partition appendix.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb1 => /dev/sdb
	 *   \li /dev/cciss/c0d0p2 => /dev/cciss/c0d0
	 *   \li /dev/mapper/vg0-lv0 => /dev/mapper/vg0-lv0
	 *   \li /dev/dm-0 => /dev/dm-0
	 *   \li /dev/md0 => /dev/md0
	 *   \li /dev/loop0 => /dev/loop0
	 *   </ul>
	 * @return The base device file.
	 */
	public function baseDeviceFile($deviceFile) {
		return $deviceFile;
	}

	/**
	 * Return the device file to be used to create a filesystem. Note,
	 * only one partition per storage device is supported.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb => /dev/sdb1
	 *   \li /dev/cciss/c0d0 => /dev/cciss/c0d0p1
	 *   \li /dev/mapper/vg0-lv0 => /dev/mapper/vg0-lv0
	 *   \li /dev/dm-0 => /dev/dm-0
	 *   \li /dev/md0 => /dev/md0
	 *   </ul>
	 * @return The device file. Defaults to /dev/xxx1.
	 */
	public function fsDeviceFile($deviceFile) {
		return sprintf("%s1", $deviceFile);
	}

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
	 * @param deviceFile Specifies the device file.
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
		// Check if the device name matches the given regular expression.
		return (1 == preg_match("/^{$regex}$/i", $deviceName));
	}
}

/**
 * Implements the storage device backend for Linux loop devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendLoop extends OMVStorageDeviceBackendAbstract {
	protected $regex = "loop[0-9]+";

	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_LOOPDEVICE;
	}

	function enumerate() {
		return $this->enumerateProcFs($this->regex);
	}

	function isTypeOf($deviceFile) {
		return $this->isTypeOfByName($deviceFile, $this->regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceLoop($args);
	}
}

/**
 * Implements the storage device backend for Linux Bcache devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendBcache extends OMVStorageDeviceBackendAbstract {
	protected $regex = "bcache[0-9]+";

	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HDD;
	}

	function enumerate() {
		return $this->enumerateProcFs($this->regex);
	}

	function enumerateSlaves() {
		$devs = $this->enumerate();
		if(FALSE === $devs)
			return FALSE;
		$result = array();
		foreach($devs as $devk => $devv) {
			$object = new OMVStorageDeviceBcache($devv);
			if(!$object->exists())
				continue;
			$result = array_merge($result, $object->getSlaves());
		}
		return $result;
	}

	function isTypeOf($deviceFile) {
		return $this->isTypeOfByName($deviceFile, $this->regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceBcache($args);
	}
}

/**
 * Implements the storage device backend for Linux device mapper devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendDM extends OMVStorageDeviceBackendAbstract {
  	protected $regex = "(mapper\/.+)|(dm-\d+)";

	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_DEVICEMAPPER;
	}

	function enumerate() {
		return $this->enumerateProcFs($this->regex);
	}

	function enumerateSlaves() {
		$devs = $this->enumerate();
		if(FALSE === $devs)
			return FALSE;
		$result = array();
		foreach($devs as $devk => $devv) {
			$object = new OMVStorageDeviceDM($devv);
			if(!$object->exists())
				continue;
			$result = array_merge($result, $object->getSlaves());
		}
		return $result;
	}

	function isTypeOf($deviceFile) {
		return $this->isTypeOfByName($deviceFile, $this->regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceDM($args);
	}

	function fsDeviceFile($deviceFile) {
		return $deviceFile;
	}
}

/**
 * Implements the storage device backend for LVM2.
 * @ingroup api
 */
class OMVStorageDeviceBackendLVM extends OMVStorageDeviceBackendDM {
	function enumerate() {
		$devs = parent::enumerate();
		if(FALSE === $devs)
			return FALSE;
		$result = array();
		foreach($devs as $devk => $devv) {
			// Check if the given device is a LVM logical volume.
			$object = new OMVLvmLogicalVolume($devv);
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
			if(!$object->exists())
				continue;
			// Get the physical volumes.
			$result = array_merge($result, $object->getSlaves());
			// Get the volume group.
			$result[] = sprintf("/dev/%s", $object->getVGName());
		}
		return $result;
	}

	function isTypeOf($deviceFile) {
		// Check if the given device is of type device mapper.
		if(FALSE === parent::isTypeOf($deviceFile))
			return FALSE;
		// Check if it is a LVM logical volume.
		$object = new OMVLvmLogicalVolume($deviceFile);
		return $object->exists();
	}

	function getImpl($args) {
		return new OMVLvmLogicalVolume($args);
	}
}

/**
 * Implements the storage device backend for Linux Mdadm software RAID
 * devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendMdadm extends OMVStorageDeviceBackendAbstract {
	protected $regex = "md\d+(p\d+)*";

	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_SOFTWARERAID;
	}

	function enumerate() {
		return $this->enumerateProcFs($this->regex);
	}

	function enumerateSlaves() {
		$devs = $this->enumerate();
		if(FALSE === $devs)
			return FALSE;
		$result = array();
		foreach($devs as $devk => $devv) {
			$object = new OMVStorageDeviceMdadm($devv);
			if(!$object->exists())
				continue;
			$result = array_merge($result, $object->getSlaves());
		}
		return $result;
	}

	function isTypeOf($deviceFile) {
		return $this->isTypeOfByName($deviceFile, $this->regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceMdadm($args);
	}

	function fsDeviceFile($deviceFile) {
		return $deviceFile;
	}
}

/**
 * Implements the storage device backend for CCISS HBA devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendCCISS extends OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HBA;
	}

	function enumerate() {
		return $this->enumerateProcFs("cciss\/c[0-9]+d[0-9]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/cciss/c0d0
		// - /dev/cciss/c0d0p2
		$regex = "cciss\/c[0-9]+d[0-9]+(p[0-9]+)?";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceCCISS($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(p\d+)$/", "", $deviceFile);
	}

	function fsDeviceFile($deviceFile) {
		// E.g. /dev/cciss/c0d0p1
		return sprintf("%sp1", $deviceFile);
	}
}

/**
 * Implements the storage device backend for harddisk devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendHDD extends OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HDD;
	}

	function enumerate() {
		return $this->enumerateProcFs("(h|s|x?v)d[a-z]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/sdb
		// - /dev/sda1
		$regex = "(h|s|x?v)d[a-z]+[0-9]*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceHDD($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(\d+)$/", "", $deviceFile);
	}
}

/**
 * Implements the storage device backend for FusionIO devices.
 * @ingroup api
 */
class OMVStorageDeviceBackendFIO extends OMVStorageDeviceBackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HDD;
	}

	function enumerate() {
		// major minor  #blocks  name
		//    8        0  312571224 sda
		//    8        1    1052226 sda1
		//    8        2    8393962 sda2
		//  252        0  157143584 fioa
		//  252        1  157139766 fioa1
		return $this->enumerateProcFs("fio[a-z]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/fioa
		// - /dev/fioa1
		$regex = "fio[a-z]+[0-9]*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new OMVStorageDeviceHDD($args);
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
		if (!isset($backend))
			return FALSE;
		self::$backends[] = $backend;
		return TRUE;
	}

	/**
	 * Get the backend of the given device.
	 * @param deviceFile Specifies the device file.
	 * @return The backend that implements the given device file, otherwise
	 *   NULL.
	 */
	final static function getBackend($deviceFile) {
		$result = NULL;
		foreach (self::$backends as $backendk => $backendv) {
			if (FALSE === $backendv->isTypeOf($deviceFile))
				continue;
			$result = $backendv;
			break;
		}
		return $result;
	}

	/**
	 * Enumerate devices matching the given storage device type.
	 * @param type Defines the storage device type, e.g. hard disk, hard or
	 *   Software RAID. Defaults to OMV_STORAGE_DEVICE_TYPE_ALL.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	public static function enumerate($type = OMV_STORAGE_DEVICE_TYPE_ALL) {
		$result = array();
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($devs = $backendv->enumerate()))
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
		if (FALSE === ($result = self::enumerate($type)))
			return FALSE;
		// Remove used devices.
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($devs = $backendv->enumerateSlaves()))
				return FALSE;
			if (empty($devs))
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
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($devs = $backendv->enumerateSlaves()))
				return FALSE;
			$result = array_unique(array_merge($result, $devs));
		}
		return $result;
	}

	/**
	 * Check if the given device is used/consumed by another storage device.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
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
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($devs = $backendv->enumerateSlaves()))
				return FALSE;
			if (in_array($deviceFile, $devs)) {
				$result = TRUE;
				break;
			}
		}
		return $result;
	}

	/**
	 * Get the object of the class which implements the given storage device.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @return The object of the class implementing the given storage device,
	 *   otherwise NULL.
	 */
	public static function getImpl($deviceFile) {
		if (NULL == ($backend = self::getBackend($deviceFile)))
			return NULL;
		return $backend->getImpl($deviceFile);
	}
}

/**
 * Factory to create objects of storage device classes.
 * @ingroup api
 */
class OMVStorageDeviceFactory extends OMVObject {
	/**
	 * Get the object of the class implementing the given storage device.
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb
	 *   \li /dev/md1
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-label/DATA
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
	 *   </ul>
	 * @return The object of the class implementing the given storage device,
	 *   otherwise NULL.
	 */
	public static function get($deviceFile) {
		return OMVStorageDevices::getImpl($deviceFile);
	}
}

/**
 * This class provides a simple interface to handle ATA/SATA and SCSI
 * hard disk devices. Try to use sysfs only to get the required information,
 * otherwise the disk will spin-up if it is in sleep mode.
 * @ingroup api
 */
class OMVStorageDeviceHDD extends OMVStorageDeviceSMARTAbstract {
	/**
	 * Get the current power mode status.
	 * @return The current power mode status which can be
	 *   <ul>
	 *   \li unknown
	 *   \li active/idle
	 *   \li standby
	 *   \li sleeping
	 *   </ul>
	 *   or FALSE on failure.
	 */
	public function getPowerModeState() {
		$result = FALSE;
		$cmd = sprintf("export LANG=C; hdparm -C %s", escapeshellarg(
		  $this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		// Parse command output:
		// /dev/sda:
		//  drive state is:  active/idle
		//
		// /dev/sdb:
		//  drive state is:  standby
		foreach ($output as $outputk => $outputv) {
			$regex = "/^\s+drive state is:\s+(\S+)$/i";
			if (1 == preg_match($regex, $outputv, $matches)) {
				$result = $matches[1];
				break;
			}
		}
		return $result;
	}

	/**
	 * Identify the device type required by the smartctl command.
	 * @return Returns the identified device type or an empty string.
	 */
	public function getSMARTDeviceType() {
		$type = parent::getSMARTDeviceType();
		if (empty($type)) {
			// TODO: The device type identification must be improved.
			if (TRUE === $this->isUsb())
				$type = $this->setSMARTDeviceType("sat");
		}
		return $type;
	}
}

/**
 * This class provides a simple interface to handle the CCISS hardware RAID
 * devices, e.g. HP's Smart Array hardware RAID controller.
 * @ingroup api
 */
class OMVStorageDeviceCCISS extends OMVStorageDeviceSMARTAbstract {
	public function getDeviceName($canonical = FALSE) {
		// Get the device name and convert '/' character to '!', e.g.
		// cciss/c0d0 => cciss!c0d0.
		return strtr(parent::getDeviceName($canonical), "/", "!");
	}

	public function isRaid() {
		return TRUE;
	}

	/**
	 * Identify the device type required by the smartctl command.
	 * @return Returns the identified device type or an empty string.
	 */
	public function getSMARTDeviceType() {
		$type = parent::getSMARTDeviceType();
		// Note, 'auto' attempts to guess the device type from the device
		// name or from controller type info provided by the operating system
		// or from a matching USB ID entry in the drive database. Thus we
		// have to do the job ourself for various hardware RAID devices.
		if(empty($type) || ("auto" === $type)) {
			if(1 == preg_match("/^cciss!c(\d)d(\d)$/", $this->getDeviceName(),
			  $matches)) {
				$type = $this->setSMARTDeviceType(sprintf("cciss,%s",
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
class OMVStorageDeviceMdadm extends OMVStorageDeviceAbstract {
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
	 * Get the software RAID device detailed information.
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
		if($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}

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

		// Get more information.
		$cmd = sprintf("export LANG=C; mdadm --detail %s",
		  escapeshellarg($this->getDeviceFile()));
		unset($output);
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}

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

		// Set flag to mark information has been successfully read.
		$this->dataCached = TRUE;

		return TRUE;
	}

	/**
	 * Refresh the cached information.
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
		if ($this->getData() === FALSE)
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
		// md0 : active raid1 sdc[2] sdd[0]
		//       2930135360 blocks super 1.2 [2/1] [U_]
		//       [>....................]  recovery = 1.1% (33131904/2930135360) finish=266.5min speed=181134K/sec
		//
		// md0 : inactive sdb[0](S) sdc[2](S) sdd[1](S)
		//       311808 blocks super 1.2
		//
		// md0 : active raid0 sdd[0] sde[1]
		//       311296 blocks super 1.2 512k chunks
		//
		// unused devices: <none>
		$regex = sprintf('/^(%s\s*:.*?)^\s*$/ims', $this->getDeviceName());
		if(1 !== preg_match($regex, file_get_contents("/proc/mdstat"),
		  $matches))
			return FALSE;
		$mdstat = $matches[1];
		// Extract the resync|reshape|recovery progress state.
		$progress = "unknown";
		if (1 == preg_match("/^.+(resync|reshape|recovery)\s*=\s*(.+)$/im",
		  $mdstat, $matches))
			$progress = trim($matches[2]);
		$state = $this->state;
		if ((FALSE !== stristr($state, "resyncing")) ||
		  (FALSE !== stristr($state, "recovering"))) {
			$state = sprintf("%s (%s)", $state, $progress);
		} else if (FALSE !== stristr($mdstat, "resync")) {
			$state = sprintf("%s, resyncing (%s)", $state, $progress);
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
class OMVStorageDeviceDM extends OMVStorageDeviceAbstract {
	/**
	 * Constructor
	 * @param deviceFile Specifies the device file, e.g. /dev/dm-1,
	 *   /dev/vg0/lv0 or /dev/mapper/vg0-lv0.
	 */
	public function __construct($deviceFile) {
		// Call parent constructor.
		parent::__construct($deviceFile);
		// Any devices of the form /dev/dm-n are for internal use only and
		// should never be used. Because of that the device file returned
		// should look like /dev/mapper/<xyz>. See for more information:
		// https://access.redhat.com/site/documentation/en-US/Red_Hat_Enterprise_Linux/6/html-single/DM_Multipath
		if(1 == preg_match("/^\/dev\/dm-\d+$/", $this->deviceFile)) {
			if(FALSE !== ($name = $this->getDeviceMapperName()))
				$this->deviceFile = sprintf("/dev/mapper/%s", $name);
		}
	}

	/**
	 * Get the name of the device mapper device.
	 * @return The name of the device mapper device.
	 */
	public function getDeviceMapperName() {
		// Make sure the canonical device file is used to extract the name
		// of the device.
		$path = sprintf("/sys/block/%s/dm/name", $this->getDeviceName(TRUE));
		if(!file_exists($path))
			return FALSE;
		return trim(file_get_contents($path));
	}

	/**
	 * Get the UUID of the device mapper device.
	 * @return The UUID of the device mapper device, otherwise FALSE.
	 */
	public function getDeviceMapperUuid() {
		// Make sure the canonical device file is used to extract the name
		// of the device.
		$path = sprintf("/sys/block/%s/dm/uuid", $this->getDeviceName(TRUE));
		if(!file_exists($path))
			return FALSE;
		return trim(file_get_contents($path));
	}

	/**
	 * Get the slave devices of the device mapper device.
	 * @return An array of device files, otherwise FALSE.
	 */
	public function getSlaves() {
		// Make sure the canonical device file is used to extract the name
		// of the device.
		$path = sprintf("/sys/block/%s/slaves", $this->getDeviceName(TRUE));
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

	/**
	 * Get the description of the device mapper device.
	 * @return The logical volume description, FALSE on failure.
	 */
	public function getDescription() {
		return sprintf(gettext("Device Mapper %s [%s, %s]"),
		  $this->getDeviceMapperName(), $this->getDeviceFile(),
		  binary_format($this->getSize()));
	}
}

/**
 * This class provides a simple interface to handle loop devices.
 * @ingroup api
 */
class OMVStorageDeviceLoop extends OMVStorageDeviceAbstract {
	/**
	 * Get the description of the device.
	 * @return The device description, FALSE on failure.
	 */
	public function getDescription() {
		return sprintf(gettext("Loop device [%s, %s]"),
		  $this->getDeviceFile(), binary_format($this->getSize()));
	}
}

/**
 * This class provides a simple interface to handle Linux Bcache devices.
 * @ingroup api
 */
class OMVStorageDeviceBcache extends OMVStorageDeviceAbstract {
	/**
	 * Get the slave devices of the Bcache device.
	 * @return An array of device files, otherwise FALSE.
	 */
	public function getSlaves() {
		// Make sure the canonical device file is used to extract the name
		// of the device.
		$path = sprintf("/sys/block/%s/slaves", $this->getDeviceName(TRUE));
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

	/**
	 * Get the description of the device.
	 * @return The device description, FALSE on failure.
	 */
	public function getDescription() {
		return sprintf(gettext("Block layer cache [%s, %s]"),
		  $this->getDeviceFile(), binary_format($this->getSize()));
	}
}

/**
 * This class implements methods to get and process S.M.A.R.T.
 * AT Attachment (ATA) information and properties.
 * @ingroup api
 */
class OMVATASMARTInformation extends OMVObject {
	protected $deviceFile = "";
	protected $type = "";
	private $cmdOutput = NULL;
	private $dataCached = FALSE;

	/**
	 * Constructor
	 * @param deviceFile Specifies the device file, e.g.
	 *   <ul>
	 *   \li /dev/sdb
	 *   \li /dev/cciss/c0d0
	 *   \li /dev/disk/by-id/scsi-SATA_ST3200XXXX2AS_5XWXXXR6
	 *   \li /dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0
	 *   </ul>
	 * @param type Specifies the type of the device, e.g. 'ata', 'cciss'
	 *   or '3ware'. For more details please check the option '-d' in
	 *   http://smartmontools.sourceforge.net/man5/smartd.conf.5.html.
	 */
	public function __construct($deviceFile, $type) {
		$this->deviceFile = $deviceFile;
		$this->type = $type;
	}

	/**
	 * Get S.M.A.R.T. information.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if ($this->dataCached !== FALSE)
			return TRUE;

		// Get all available S.M.A.R.T. information.
		$cmdArgs = array();
		$cmdArgs[] = sprintf("-x %s", escapeshellarg($this->deviceFile));
		if (!empty($this->type))
			$cmdArgs[] = sprintf("-d %s", $this->type);
		$cmd = sprintf("export LANG=C; smartctl %s", implode(" ", $cmdArgs));
		@OMVUtil::exec($cmd, $output, $result);
		// Bit 0: Command line did not parse
		// Bit 1: Device open failed, or device did not return an
		//        IDENTIFY DEVICE structure
		if (($result !== 0) && (($result & 0) || ($result & 1))) {
			$this->setLastError($output);
			return FALSE;
		}
		$this->cmdOutput = $output;

		// Set flag to mark information has been successfully read.
		$this->dataCached = TRUE;

		return TRUE;
	}

	/**
	 * Refresh the cached information.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		if ($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Get the plain text 'smartctl' command output.
	 * @return The command output, otherwise FALSE.
	 */
	public function getExtendedInformation() {
		if ($this->getData() === FALSE)
			return FALSE;
		return implode("\n", $this->cmdOutput);
	}

	/**
	 * Get the S.M.A.R.T. attributes data structure.
	 * @return An array of S.M.A.R.T. attributes data, otherwise FALSE.
	 */
	public function getAttributes() {
		if ($this->getData() === FALSE)
			return FALSE;
		$result = array();
		foreach ($this->cmdOutput as $cmdOutputk => $cmdOutputv) {
			// smartctl 5.41 2011-06-09 r3365
			// SMART Attributes Data Structure revision number: 16
			// Vendor Specific SMART Attributes with Thresholds:
			// ID# ATTRIBUTE_NAME          FLAGS    VALUE WORST THRESH FAIL RAW_VALUE
			//   1 Raw_Read_Error_Rate     POSR-K   100   100   051    -    48
			//   2 Throughput_Performance  -OS--K   055   055   000    -    18907
			//   3 Spin_Up_Time            PO---K   068   068   025    -    9773
			//   4 Start_Stop_Count        -O--CK   100   100   000    -    795
			//   5 Reallocated_Sector_Ct   PO--CK   252   252   010    -    0
			//   7 Seek_Error_Rate         -OSR-K   252   252   051    -    0
			$regex = '/^\s*(\d+)\s+(\S+)\s+([POSRCK-]+)\s+(\d+)\s+(\d+)\s+'.
			  '(\d+)\s+(\S+)\s+(.+)$/i';
			if (1 == preg_match($regex, $cmdOutputv, $matches)) {
				$result[] = array(
					"id" => $matches[1],
					"attrname" => $matches[2],
					"flags" => $matches[3],
					"value" => $matches[4],
					"worst" => $matches[5],
					"treshold" => $matches[6],
					"whenfailed" => $matches[7],
					"rawvalue" => $matches[8]
				);
			}
		}
		return $result;
	}

	/**
	 * Get the S.M.A.R.T. Self-test log structure.
	 * @return An array of S.M.A.R.T. self-test logs, otherwise FALSE.
	 */
	public function getSelfTestLogs() {
		if ($this->getData() === FALSE)
			return FALSE;
		$result = array();
		foreach ($this->cmdOutput as $cmdOutputk => $cmdOutputv) {
			// SMART Self-test log structure
			// Parse command output:
			// Num  Test_Description    Status                  Remaining  LifeTime(hours)  LBA_of_first_error
			// # 1  Extended offline    Completed: read failure       90%       670         57217755
			// # 2  Short captive       Interrupted (host reset)      80%      1392         -
			$regex = '/^#\s*(\d+)\s+(Short offline|Extended offline|'.
			  'Short captive|Extended captive)\s+(.+)\s+(\d+)%\s+(\d+)'.
			  '\s+(.+)$/';
			if (1 == preg_match($regex, $cmdOutputv, $matches)) {
				$result[] = array(
					"num" => $matches[1],
					"description" => $matches[2],
					"status" => $matches[3],
					"remaining" => $matches[4],
					"lifetime" => $matches[5],
					"lbaoffirsterror" => $matches[6]
				);
			}
		}
		return $result;
	}

	/**
	 * Get various device information.
	 * @return An array of strings, otherwise FALSE.
	 */
	public function getInformation() {
		if ($this->getData() === FALSE)
			return FALSE;
		// Initialize with default values. Note, the result list may
		// contain additional key/value pairs.
		$result = array(
			"devicemodel" => "",
			"serialnumber" => "",
			"firmwareversion" => "",
			"usercapacity" => ""
		);
		// INFORMATION SECTION
		// Parse command output:
		// === START OF INFORMATION SECTION ===
		// Model Family:     Western Digital RE3 Serial ATA family
		// Device Model:     WDC WD2502ABYS-02B7A0
		// Serial Number:    WD-WCAV1B245569
		// Firmware Version: 02.03B03
		// User Capacity:    251,059,544,064 bytes
		// Device is:        In smartctl database [for details use: -P show]
		// ATA Version is:   8
		// ATA Standard is:  Exact ATA specification draft version not indicated
		// Local Time is:    Tue Mar 11 10:18:42 2014 CET
		// SMART support is: Available - device has SMART capability.
		// SMART support is: Enabled
		//
		// === START OF READ SMART DATA SECTION ===
		// ...
		$sectionFound = FALSE;
		foreach ($this->cmdOutput as $cmdOutputk => $cmdOutputv) {
			$cmdOutputv = trim($cmdOutputv);
			// Abort parsing, we are not interested in the information
			// shown below this line.
			if ($cmdOutputv == "=== START OF READ SMART DATA SECTION ===")
				break;
			// Ignore everything that is not below this line.
			if ($cmdOutputv == "=== START OF INFORMATION SECTION ===") {
				$sectionFound = TRUE;
				continue;
			}
			// Have we found the information section?
			if (FALSE === $sectionFound)
				continue;
			// Parse the information section line:
			// Device Model:     WDC WD2502ABYS-02B7A0
			$regex = '/^([^:]+):\s+(.+)$/i';
			if (1 !== preg_match($regex, $cmdOutputv, $matches))
				continue;
			// Convert the attribute name, e.g. 'Device Model' to
			// 'devicemodel'.
			$attrKey = strtolower(str_replace(" ", "", $matches[1]));
			// Append key/value to result array.
			$result[$attrKey] = $matches[2];
		}
		return $result;
	}

	/**
	 * Get the device temperature in °C (value only, no unit).
	 * @return The temperature value, otherwise FALSE.
	 */
	public function getTemperature() {
		if (FALSE === ($attributes = $this->getAttributes()))
			return FALSE;
		$temp = FALSE;
		$found = FALSE;
		// Process the attributes to get the temperature value.
		foreach ($attributes as $attrk => $attrv) {
			switch ($attrv['id']) {
			case 194:
			case 231:
				// The value may look like: '43 (Lifetime Min/Max 16/61)'
				$regex = '/^(\d+)(\s+(.+))*$/';
				if (1 !== preg_match($regex, $attrv['rawvalue'], $matches))
					continue;
				$temp = $matches[1];
				$found = TRUE;
				break;
			}
			if ($found)
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
		if (FALSE === $found) {
			foreach ($this->cmdOutput as $cmdOutputk => $cmdOutputv) {
				$regex = '/^Current Drive Temperature:\s+(\d+)\s+C$/';
				if (1 == preg_match($regex, $cmdOutputv, $matches)) {
					$temp = $matches[1];
					break;
				}
			}
		}
		return $temp;
	}
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
	 * @deprecated
	 * @return A list of network interfaces, otherwise FALSE.
	 */
	public static function get() {
		return self::enumerate();
	}

	/**
	 * Enumerate all network interfaces.
	 * @return A list of network interfaces, e.g. eth0 or lo, otherwise FALSE.
	 */
	public static function enumerate() {
		$result = array();
		$dir = new DirectoryIterator("/sys/class/net");
		foreach ($dir as $item) {
			// Skip everything that is not a symlink.
			if ($item->isDot() || !$item->isLink())
				continue;
			// Validate device names.
			$regex = '/^[a-z]+[0-9]+|lo$/i';
			if (1 !== preg_match($regex, $item->getFilename()))
				continue;
			$result[] = $item->getFilename();
		}
		// Sort the devices using a "natural order" algorithm.
		if (!natsort($result))
			return FALSE;
		return $result;
	}

	/**
	 * Get a list of network interfaces by type.
	 * @deprecated
	 * @param type The type of interface, e.g. eth, lo or bond.
	 * @return A list of network interfaces, otherwise FALSE.
	 */
	public static function getByType($type) {
		return self::enumerateByType($type);
	}

	/**
	 * Enumerate all network interfaces by type.
	 * @param type The type of interface, e.g. eth, lo or bond.
	 * @return A list of network interfaces, otherwise FALSE.
	 */
	public static function enumerateByType($type) {
		$result = array();
		$deviceNames = self::enumerate();
		foreach ($deviceNames as $deviceNamek => $deviceNamev) {
			// Validate device names.
			$regex = sprintf('/%s/i', $type);
			if (1 !== preg_match($regex, $deviceNamev))
				continue;
			$result[] = $deviceNamev;
		}
		// Sort the devices using a "natural order" algorithm.
		if (!natsort($result))
			return FALSE;
		return $result;
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
	 * @deprecated
	 * @return An array of system group names, otherwise FALSE.
	 */
	public static function get() {
		return self::enumerate();
	}

	/**
	 * Enumerate system group names.
	 * @return An array of system group names, otherwise FALSE.
	 */
	public static function enumerate() {
		$cmd = "export LANG=C; getent group";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		$list = array();
		// Parse command output:
		// shadow:x:42:openmediavault
		// snmp:x:112:
		// sambashare:x:113:
		// openmediavault:x:999:
		// nut:x:114:
		foreach ($output as $outputv) {
			$data = explode(":", $outputv);
			if (empty($data))
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
	private $dataCached = FALSE;

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
		if($this->dataCached !== FALSE)
			return TRUE;
		// Query user information.
		$groupInfo = FALSE;
		if(!is_null($this->gid))
			$groupInfo = posix_getgrgid($this->gid);
		else
			$groupInfo = posix_getgrnam($this->name);
		if(FALSE === $groupInfo) {
			$this->setLastError(posix_strerror(posix_errno()));
			return FALSE;
		}
		$this->name = $groupInfo['name'];
		$this->gid = $groupInfo['gid'];
		$this->members = $groupInfo['members'];
		// Set flag to mark information has been successfully read.
		$this->dataCached = TRUE;
		return TRUE;
	}

	/**
	 * Refresh the cached information.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		if($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Check whether the system group exists.
	 * @return TRUE if the system group exists, otherwise FALSE.
	 */
	public function exists() {
		if($this->getData() === FALSE)
			return FALSE;
		return !is_null($this->name) && !is_null($this->gid);
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
		  escapeshellarg($this->getName()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
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
 * Helper class to get the users on the system.
 * @ingroup api
 */
class OMVUsers extends OMVObject {
	/**
	 * Get a list of system user names.
	 * @deprecated
	 * @return An array of system user names, otherwise FALSE.
	 */
	public static function get() {
		return self::enumerate();
	}

	/**
	 * Enumerate system user names.
	 * @return An array of system user names, otherwise FALSE.
	 */
	public static function enumerate() {
		$cmd = "export LANG=C; getent passwd";
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		$list = array();
		// Parse command output:
		// proftpd:x:109:65534::/var/run/proftpd:/bin/false
		// ftp:x:110:65534::/home/ftp:/bin/false
		// openmediavault:x:999:999::/home/openmediavault:/bin/sh
		// admin:x:998:100:WebGUI administrator:/home/admin:/usr/sbin/nologin
		// nut:x:111:114::/var/lib/nut:/bin/false
		// test:x:1001:100:sdfds:/home/test:/bin/dash
		foreach ($output as $outputv) {
			$data = explode(":", $outputv);
			if (empty($data))
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
			if(($result !== 0) || empty($output[0])) {
				$this->setLastError($output);
				return FALSE;
			}
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
			if(!is_null($this->name) && !is_null($this->uid))
				return TRUE;
			// Query user information.
			$userInfo = FALSE;
			if(!is_null($this->uid))
				$userInfo = posix_getpwuid($this->uid);
			else
				$userInfo = posix_getpwnam($this->name);
			if(FALSE === $userInfo) {
				$this->setLastError(posix_strerror(posix_errno()));
				return FALSE;
			}
			$this->name = $userInfo['name'];
			$this->uid = $userInfo['uid'];
			$this->gid = $userInfo['gid'];
			$this->gecos = $userInfo['gecos'];
			$this->dir = $userInfo['dir'];
			$this->shell = $userInfo['shell'];
			$result = TRUE;
			break;
		case "groups":
			if(!is_null($this->groups))
				return TRUE;
			// Get the group IDs the user is in as white-space separated
			// numbers. Thus it is possible to correctly process group
			// names that contain blanks (e.g. imported via LDAP or AD).
			$cmd = sprintf("export LANG=C; id -G %s", escapeshellarg(
			  $this->getName()));
			@OMVUtil::exec($cmd, $output, $result);
			if(($result !== 0) || empty($output[0])) {
				$this->setLastError($output);
				return FALSE;
			}
			$gids = explode(" ", $output[0]);
			// Resolve the group names.
			$this->groups = array();
			foreach($gids as $gidk => $gidv) {
				$groupInfo = posix_getgrgid($gidv);
				$this->groups[] = $groupInfo['name'];
			}
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
		return !is_null($this->name) && !is_null($this->uid);
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
		  escapeshellarg($this->getName()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
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

	/**
	 * Authenticate user via PAM. Account expiration and access hours are
	 * checked, too. Note, the calling process must have the privilege to
	 * read the user shadow file.
	 * @param password The password.
	 * @return TRUE if the authentication was successful, otherwise FALSE.
	 */
	public function authenticate($password) {
		$error = "";
		if (FALSE === pam_auth($this->getName(), $password, $error, TRUE)) {
			$this->setLastError($error);
			return FALSE;
		}
		return TRUE;
	}
}

/**
 * This class provides a simple interface to handle a LVM physical volume.
 * @ingroup api
 */
class OMVLvmPhysicalVolumes extends OMVObject {
	/**
	 * Get a list of physical volumes.
	 * @deprecated
	 * @return A list of physical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/sdb
	 *   1 => /dev/sdd
	 * )
	 */
	public static function get() {
		return self::enumerate();
	}

	/**
	 * Enumerate LVM physical volumes.
	 * @return A list of LVM physical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/sdb
	 *   1 => /dev/sdd
	 * )
	 */
	public static function enumerate() {
		$cmd = "export LANG=C; pvdisplay --noheadings -C -o pv_name ".
		  "2>/dev/null";
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
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
	 * @deprecated
	 * @return A list of logical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/mapper/vgName-lvName
	 *   1 => /dev/mapper/...
	 * )
	 */
	public static function get() {
		return self::enumerate();
	}

	/**
	 * Enumerate LVM logical volumes.
	 * @return A list of LVM logical volumes, otherwise FALSE.
	 * Example: array(
	 *   0 => /dev/mapper/vgName-lvName
	 *   1 => /dev/mapper/...
	 * )
	 */
	public static function enumerate() {
		$cmd = "export LANG=C; lvdisplay --noheadings --separator '|' ".
		  "-C -o vg_name,lv_name 2>/dev/null";
		@OMVUtil::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
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
class OMVLvmLogicalVolume extends OMVStorageDeviceDM {
	protected $uuid = "";
	protected $attr = "";
	protected $kernelMajor = "";
	protected $kernelMinor = "";
	protected $lvName = "";
	protected $vgName = "";
	private $dataCached = FALSE;

	/**
	 * Get the logical volume detailed information.
	 * @private
	 * @return TRUE if successful, otherwise FALSE.
	 */
	private function getData() {
		if ($this->dataCached !== FALSE)
			return TRUE;

		// Parse command output:
		// VOSP6y-Lw75-pbTs-3v8A-6Qek-XBKJ-Md2G1q|vltest|test|12582912B
		$cmd = sprintf("export LANG=C; lvdisplay --noheadings ".
		  "--separator '|' -C -o lv_uuid,lv_name,vg_name,lv_size,lv_attr,".
		  "lv_kernel_major,lv_kernel_minor --unit b %s",
		  escapeshellarg($this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}

		$output = explode("|", trim($output[0]));

		$this->uuid = $output[0];
		$this->lvName = $output[1];
		$this->vgName = $output[2];
		$this->size = substr($output[3], 0, -1);
		$this->attr = $output[4];
		$this->kernelMajor = intval($output[5]);
		$this->kernelMinor = intval($output[6]);

		// Set flag to mark information has been successfully read.
		$this->dataCached = TRUE;

		return TRUE;
	}

	/**
	 * Refresh the cached information.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		if ($this->getData() === FALSE)
			return FALSE;
		return TRUE;
	}

	/**
	 * Checks if the logical volume exists.
	 * @return TRUE if the logical volume exists, otherwise FALSE.
	 */
	public function exists() {
		if ($this->getData() === FALSE)
			return FALSE;
		return !empty($this->uuid);
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
		return $this->lvName;
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
		return parent::getSize();
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
	 * Get the description of the logical volume.
	 * @return The logical volume description, FALSE on failure.
	 */
	public function getDescription() {
		if ($this->getData() === FALSE)
			return FALSE;
		return sprintf(gettext("LVM logical volume %s [%s, %s]"),
		  $this->getName(), $this->getDeviceFile(),
		  binary_format($this->getSize()));
	}

	/**
	 * Create the logical volume.
	 * @param name The name for the new logical volume.
	 * @param size Gives the size of bytes to allocate for the new
	 *   logical volume.
	 * @param vgName The name of the volume group where to create the
	 *   logical volume.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function create($name, $size, $vgName) {
		$cmd = sprintf("export LANG=C; lvcreate --name %s --size %sK %s 2>&1",
		  escapeshellarg($name), binary_convert($size, "B", "KiB"),
		  escapeshellarg($vgName));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Remove the logical volume.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function remove() {
		$cmd = sprintf("export LANG=C; lvremove --force %s 2>&1",
		  escapeshellarg($this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Rename the logical volume.
	 * @param name The new logical volume path/name.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function rename($name) {
		if ($this->getData() === FALSE)
			return FALSE;
		$cmd = sprintf("export LANG=C; lvrename %s %s 2>&1", escapeshellarg(
		  $this->getPath()), escapeshellarg($name));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Extend the logical volume.
	 * @param size Gives the size of bytes to extend the logical volume.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function extend($size) {
		// Convert size to KiB to ensure it is a multiple of 512.
		$cmd = sprintf("export LANG=C; lvextend --size %sK %s 2>&1",
		  binary_convert($size, "B", "KiB"), escapeshellarg(
		  $this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Reduce the logical volume.
	 * @param size Gives the size of bytes to extend the logical volume.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function reduce($size) {
		// Convert size to KiB to ensure it is a multiple of 512.
		$cmd = sprintf("export LANG=C; lvreduce --size %sK %s 2>&1",
		  binary_convert($size, "B", "KiB"), escapeshellarg(
		  $this->getDeviceFile()));
		@OMVUtil::exec($cmd, $output, $result);
		if ($result !== 0) {
			$this->setLastError($output);
			return FALSE;
		}
		return TRUE;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Register default storage device backends.
///////////////////////////////////////////////////////////////////////////////
OMVStorageDevices::registerBackend(new OMVStorageDeviceBackendHDD());
OMVStorageDevices::registerBackend(new OMVStorageDeviceBackendCCISS());
OMVStorageDevices::registerBackend(new OMVStorageDeviceBackendMdadm());
OMVStorageDevices::registerBackend(new OMVStorageDeviceBackendLVM());
OMVStorageDevices::registerBackend(new OMVStorageDeviceBackendDM());
OMVStorageDevices::registerBackend(new OMVStorageDeviceBackendLoop());
OMVStorageDevices::registerBackend(new OMVStorageDeviceBackendBcache());
OMVStorageDevices::registerBackend(new OMVStorageDeviceBackendFIO());

///////////////////////////////////////////////////////////////////////////////
// Register default filesystem backends.
///////////////////////////////////////////////////////////////////////////////
OMVFilesystems::registerBackend(new OMVFilesystemBackendExt4);
OMVFilesystems::registerBackend(new OMVFilesystemBackendExt3);
OMVFilesystems::registerBackend(new OMVFilesystemBackendExt2);
OMVFilesystems::registerBackend(new OMVFilesystemBackendExt);
OMVFilesystems::registerBackend(new OMVFilesystemBackendXfs);
OMVFilesystems::registerBackend(new OMVFilesystemBackendJfs);
OMVFilesystems::registerBackend(new OMVFilesystemBackendVfat);
OMVFilesystems::registerBackend(new OMVFilesystemBackendNtfs);
OMVFilesystems::registerBackend(new OMVFilesystemBackendMsdos);
OMVFilesystems::registerBackend(new OMVFilesystemBackendHfsplus);
OMVFilesystems::registerBackend(new OMVFilesystemBackendBtrfs);
OMVFilesystems::registerBackend(new OMVFilesystemBackendIso9660);
OMVFilesystems::registerBackend(new OMVFilesystemBackendUdf);
OMVFilesystems::registerBackend(new OMVFilesystemBackendReiserfs);
OMVFilesystems::registerBackend(new OMVFilesystemBackendUmsdos);
OMVFilesystems::registerBackend(new OMVFilesystemBackendUfs);
OMVFilesystems::registerBackend(new OMVFilesystemBackendNone);
?>
