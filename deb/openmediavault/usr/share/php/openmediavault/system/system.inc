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
namespace OMV\System;

require_once("openmediavault/functions.inc");

/**
 * Helper class that provides various functions regarding the Linux system.
 * @ingroup api
 */
class System {
	/**
	 * Get the device file where the operating system is installed on (e.g.
	 * /dev/sda1). To avoid further useless process calls to determine the
	 * root device file the value is cached in a static variable when this
	 * method is called the first time.
	 * @return Returns the device file where the operating system is installed
	 *   on.
	 * @throw \OMV\ExecException
	 */
	public static function getRootDeviceFile() {
		// Cache the root device file. If this should change during the
		// meantime something strange has been happened to the system.
		static $rootDeviceFile = NULL;
		if (!is_null($rootDeviceFile))
			return $rootDeviceFile;
		// Do not use /dev/root anymore because Debian has removed this
		// symlink in Jessie. The OMV workaround to create this symlink
		// during the boot process via an udev rule does not work reliable.
		//if (is_block_device("/dev/root"))
		//	return realpath("/dev/root");
		$mp = new \OMV\System\MountPoint("/");
		return $mp->getDeviceFile();
	}

	/**
	 * Check if the given device file contains the operating system.
	 * @param deviceFile The devicefile to check, e.g. /dev/sdb or /dev/sda1.
	 * @param exact Set to FALSE to detect partitions, too. Defaults to TRUE.
	 * @return Returns TRUE if the device file contains the operating system,
	 *   otherwise FALSE.
	 */
	public static function isRootDeviceFile($deviceFile, $exact = TRUE) {
		// '/dev/root' is obsolete, but we still keep it (maybe we will
		// need it sometime).
		if ("/dev/root" == $deviceFile)
			return TRUE;
		$rootDeviceFile = self::getRootDeviceFile();
		if (TRUE === $exact) {
			return (($rootDeviceFile == $deviceFile) ||
				($rootDeviceFile == realpath($deviceFile))) ?
				TRUE : FALSE;
		}
		return ((0 === strpos($rootDeviceFile, $deviceFile)) ||
			(0 === strpos($rootDeviceFile, realpath($deviceFile)))) ?
			TRUE : FALSE;
	}

	/**
	 * Get the /etc/login.defs configuration
	 * @return Array containing the configuration as key/value pairs or
	 *   FALSE on failure.
	 */
	public static function getLoginDefs() {
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
		$file = new \OMV\Util\KeyValueFile("/etc/login.defs");
		$file->setKeyValueDelimiter("\\s");
		$file->setKeyCaseSensitiv(TRUE); // Do not lowercase keys.
		return $file->getAssoc();
	}

	/**
	 * Get the next free device name, e.g. md3 or bond2.
	 * Note, do not use this method combined with predictable network
	 * device names.
	 * @param type The type of the device, e.g. disk or iface
	 * @param name The device name, e.g. sda, hda, md, eth or bond
	 * @return The next free device name, or FALSE on failure.
	 */
	public static function getNextDevice($type, $name) {
		$cmdList = [
			"disk" => "awk '{print $4}' /proc/partitions",
			"iface" => "awk -F: '/[:]/ {print $1}' /proc/net/dev"
		];
		$cmd = new \OMV\System\Process($cmdList[$type]);
		$cmd->execute($output);
		$nums = [];
		$regex = sprintf("/^%s(\d+)$/", $name);
		foreach ($output as $itemk => $itemv) {
			if (1 !== preg_match($regex, trim($itemv), $matches))
				continue;
			$nums[] = $matches[1];
		}
		// Try to find the next free number.
		$num = -1;
		for ($i = 0; $i <= 255; $i++) {
			if (TRUE === in_array($i, $nums))
				continue;
			$num = $i;
			break;
		}
		if (0 > $num)
			return FALSE;
		return sprintf("%s%d", $name, $num);
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
		if (FALSE === $indexed)
			return sprintf("%d %s %d %s %d %s %d %s",
			  $days, ($days == 1) ? gettext("day") : gettext("days"),
			  $hours, ($hours == 1) ? gettext("hour") : gettext("hours"),
			  $mins, ($mins == 1) ? gettext("minute") : gettext("minutes"),
			  $secs, gettext("seconds"));
		else
			return [
				"seconds" => $secs,
				"minutes" => $mins,
				"hours" => $hours,
				"days" => $days
			];
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
		$cmd = new \OMV\System\Process("free", "-b", "-t", "-w");
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$result = [
			"mem" => [
				"total" => 0,
				"used" => 0,
				"free" => 0,
				"shared" => 0,
				"buffers" => 0,
				"cache" => 0,
				"available" => 0
			],
			"swap" => [
				"total" => 0,
				"used" => 0,
				"free" => 0
			],
			"total" => [
				"total" => 0,
				"used" => 0,
				"free" => 0
			]
		];
		// Parse command output:
		//               total        used        free      shared     buffers       cache   available
		// Mem:     2101825536    98390016  1823117312    12967936    16809984   163508224  1844318208
		// Swap:             0           0           0
		// Total:   2101825536    98390016  1823117312
		foreach ($output as $outputk => $outputv) {
			if (preg_match('/^Mem:\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)$/',
			  $outputv, $matches)) {
				$result['mem'] = [
					"total" => $matches[1],
					"used" => $matches[2],
					"free" => $matches[3],
					"shared" => $matches[4],
					"buffers" => $matches[5],
					"cache" => $matches[6],
					"available" => $matches[7]
				];
			} else if (preg_match('/^Swap:\s+(\d+)\s+(\d+)\s+(\d+)$/',
			  $outputv, $matches)) {
				$result['swap'] = [
					"total" => $matches[1],
					"used" => $matches[2],
					"free" => $matches[3]
				];
			} else if (preg_match('/^Total:\s+(\d+)\s+(\d+)\s+(\d+)$/',
			  $outputv, $matches)) {
				$result['total'] = [
					"total" => $matches[1],
					"used" => $matches[2],
					"free" => $matches[3]
				];
			}
		}
		return $result;
	}

	/**
	 * Get CPU statistics.
	 * @return The memory statistics as array or FALSE on failure.
	 * array(
	 *   cpumhz => "2792.999"
	 *   modelname => "Intel(R) Core(TM) i7 CPU         860  @ 2.80GHz",
	 *   usage => 12
	 * )
	 */
	public static function getCpuStats() {
		// Get the CPU information. Take care about CPUs with multiple cores.
		// processor	: 0
		// vendor_id	: GenuineIntel
		// cpu family	: 6
		// model		: 30
		// model name	: Intel(R) Core(TM) i7 CPU         860  @ 2.80GHz
		// stepping	    : 5
		// microcode	: 0x3
		// cpu MHz		: 2792.999
		// ...
		// system type             : MT7621
		// machine                 : GnuBee
		// processor               : 0
		// cpu model               : MIPS 1004Kc V2.15
		// BogoMIPS                : 663.55
		// ...
		$cpuInfo = new \OMV\Util\KeyValueFile("/proc/cpuinfo", ":");
		$cpuInfo->setKeyNameDelimiter("");
		$cpuInfo->setKeyCaseSensitiv(TRUE);
		// Get the current CPU statistics.
		// Use the very first "cpu" line which aggregates the numbers in
		// all of the other "cpuN" lines.
		// cpu  2255 34 2290 22625563 6290 127 456
		// * user: normal processes executing in user mode
		// * nice: niced processes executing in user mode
		// * system: processes executing in kernel mode
		// * idle: twiddling thumbs
		// * iowait: waiting for I/O to complete
		// * irq: servicing interrupts
		// * softirq: servicing softirqs
		//
		// See also
		// - https://serverfault.com/a/667250.
		// - https://en.wikipedia.org/wiki/Load_(computing)
		$tprev = file("/proc/stat");
		sleep(1);
		$tnow = file("/proc/stat");
		$tprev = explode(" ", $tprev[0]);
		array_shift($tprev); array_shift($tprev);
		$tnow = explode(" ", $tnow[0]);
		array_shift($tnow); array_shift($tnow);
		// Calculate the total CPU time.
		$tprevTotal = array_sum($tprev);
		$tnowTotal = array_sum($tnow);
		// Calculate the CPU usage since we last checked.
		$diffIdle = $tnow[3] - $tprev[3];
		$diffIowait = $tnow[4] - $tprev[4];
		$diffTotal = $tnowTotal - $tprevTotal;
		// Get the processor name.
		// Parse output:
		// ...
		// CPU part	: 0xd03
		// CPU revision	: 4
		//
		// Hardware		: Allwinnersun50iw2Family
		// Revision		: 0000
		// Serial		: 0000000000000000
		$modelName = gettext("n/a");
		foreach ([ "modelname", "cpumodel", "Processor", "Hardware" ] as $key) {
			if (TRUE === $cpuInfo->exists($key)) {
				$modelName = $cpuInfo->get($key);
				break;
			}
		}
		return [
			"modelname" => $modelName,
			"cpumhz" => $cpuInfo->exists("cpumhz") ?
				$cpuInfo->get("cpumhz") : gettext("n/a"),
			"usage" => (0 == $diffTotal) ? 0 :
				(($diffTotal - $diffIowait - $diffIdle) / $diffTotal) * 100
		];
	}

	/**
	 * Get a list of Debian packages that are available to be upgraded.
	 * This list is automatically generated via an APT trigger which is
	 * executed when 'apt-get update' is called. Thus it is not necessary
	 * to execute the very cost intensive code every RPC call.
	 * @return array An array of objects containing the fields \em name,
	 *   \em version, \em oldversion, \em repository, \em architecture,
	 *   \em package, \em priority, \em section, \em installedsize,
	 *   \em maintainer, \em filename, \em size, \em md5sum, \em sha1,
	 *   \em sha256, \em abstract and \em homepage.
	 *   The following fields are optional: \em description, \em depends,
	 *   \em replaces and \em conflicts.
	 * @throws \OMV\Exception
	 */
	public static function getAptUpgradeList() {
		// Read the information from the package index. This will be
		// updated automatically via APT trigger.
		$file = new \OMV\Json\File(\OMV\Environment::get(
			"OMV_APT_UPGRADE_INDEX_FILE"));
		if (!$file->exists()) {
			throw new \OMV\Exception(
				"The index of upgradeable packages does not exist. Please ".
				"re-synchronize the package index files from their sources.");
		}
		$file->open("r");
		$result = $file->read();
		$file->close();
		return $result;
	}
}
