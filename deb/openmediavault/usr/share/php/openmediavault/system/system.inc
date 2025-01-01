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
	 * @return Returns the canonical device file where the operating system
	 *   is installed on.
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
		// Return the canonical device file to prevent future hiccups,
		// e.g. `/dev/mapper/omv--lvm--vg-root` => `/dev/dm-0`.
		return realpath($mp->getDeviceFile());
	}

	/**
	 * Check if the given device file contains the operating system.
	 * @param deviceFile The device file to check, e.g. /dev/sdb or /dev/sda1.
	 * @param exact Set to FALSE to detect partitions, too. Defaults to TRUE.
	 * @return Returns TRUE if the device file contains the operating system,
	 *   otherwise FALSE.
	 */
	public static function isRootDeviceFile($deviceFile, $exact = TRUE) {
		// '/dev/root' is obsolete, but we still keep it (maybe we will
		// need it sometime).
		if ("/dev/root" == $deviceFile)
			return TRUE;
		$rootDeviceFile = static::getRootDeviceFile();
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
	 * @return The uptime in seconds.
	 */
	public static function uptime(): float {
		$uptime = explode(" ", trim(file_get_contents("/proc/uptime")));
		return floatval($uptime[0]);
	}

	/**
	 * Get load average in regard to both the CPU and IO over time.
	 * @return The CPU and IO utilization of the last one, five, and 10 minute
	 *   periods or FALSE on failure.
	 */
	public static function getLoadAverage() {
		// https://scoutapm.com/blog/understanding-load-averages
		$loadavg = explode(" ", trim(file_get_contents("/proc/loadavg")));
		return [
			"1min" => floatval($loadavg[0]),
			"5min" => floatval($loadavg[1]),
			"15min" => floatval($loadavg[2])
		];
	}

	/**
	 * Get memory statistics.
	 * @return The memory statistics as array.
	 *   Note, all values that will break the 32bit compatibility are
	 *   returned as strings.
	 * array(
	 *   mem (
	 *     total => xxx, (bytes as string)
	 *     used => xxx, (bytes as string)
	 *     free => xxx, (bytes as string)
	 *     shared => xxx, (bytes as string)
	 *     buffers => xxx, (bytes as string)
	 *     cached => xxx, (bytes as string)
	 *     utilization => xxx, (float)
	 *   )
	 *   swap (
	 *     total => xxx, (bytes as string)
	 *     used => xxx, (bytes as string)
	 *     free => xxx, (bytes as string)
	 *     utilization => xxx, (float)
	 *   )
	 *   total (
	 *     total => xxx, (bytes as string)
	 *     used => xxx, (bytes as string)
	 *     free => xxx, (bytes as string)
	 *     utilization => xxx, (float)
	 *   )
	 * )
	 */
	public static function getMemoryStats() {
		$result = [
			"mem" => [
				"total" => "0",
				"used" => "0",
				"free" => "0",
				"shared" => "0",
				"buffers" => "0",
				"cache" => "0",
				"available" => "0",
				"utilization" => 0
			],
			"swap" => [
				"total" => "0",
				"used" => "0",
				"free" => "0",
				"utilization" => 0
			],
			"total" => [
				"total" => "0",
				"used" => "0",
				"free" => "0",
				"utilization" => 0
			]
		];
/*
		// The utilization is calculated based on
		// https://github.com/giampaolo/psutil/blob/release-5.9.0/psutil/_pslinux.py#L523
		$calcUtilization = function($total, $avail, $scale = 5) {
			if (0 == bccomp($total, "0")) {
				return "0";
			}
			return bcdiv(bcsub($total, $avail), $total, $scale);
		};

		// Get well explained information about extracting the values
		// from the /procfs in the Python psutils code.
		// https://github.com/giampaolo/psutil/blob/release-5.9.0/psutil/_pslinux.py#L415
		$output = file("/proc/meminfo", FILE_IGNORE_NEW_LINES |
			FILE_SKIP_EMPTY_LINES);
		if (FALSE === $output) {
			return $result;
		}

		// Parse command output:
		// MemTotal:        3787296 kB
		// MemFree:         2118144 kB
		// MemAvailable:    3275732 kB
		// Buffers:          172764 kB
		// Cached:           959772 kB
		// SwapCached:            0 kB
		// Active:           241084 kB
		// Inactive:        1037316 kB
		// Active(anon):       4264 kB
		// ...
		$memInfo = [];
		foreach ($output as $outputk => $outputv) {
			$regex = "/^(\w+):\s+(\d+)( kb)?$/i";
			if (1 !== preg_match($regex, $outputv, $matches)) {
				continue;
			}
			if ("kb" == strtolower(trim($matches[3]))) {
				$memInfo[$matches[1]] = bcmul($matches[2], "1024");
			} else {
				$memInfo[$matches[1]] = $matches[2];
			}
		}

		$result['mem']['total'] = $memInfo['MemTotal'];
		$result['mem']['free'] = $memInfo['MemFree'];
		$result['mem']['shared'] = $memInfo['Shmem'];
		$result['mem']['buffers'] = $memInfo['Buffers'];
		$result['mem']['cache'] = $memInfo['Cached'];
		$result['mem']['available'] = $memInfo['MemAvailable'];
		$result['mem']['used'] = bcsub($result['mem']['total'],
			bcadd($result['mem']['free'],
			bcadd($result['mem']['cache'], $result['mem']['buffers'])));
		if (-1 == bccomp($result['mem']['used'], "0")) {
			// See https://github.com/giampaolo/psutil/blob/release-5.9.0/psutil/_pslinux.py#L498
			$result['mem']['used'] = bcsub($result['mem']['total'],
				$result['mem']['free']);
		}
		if (-1 == bccomp($result['mem']['available'], "0")) {
			// See https://github.com/giampaolo/psutil/blob/release-5.9.0/psutil/_pslinux.py#L512
			$result['mem']['available'] = "0";
		}
		if (1 == bccomp($result['mem']['available'],
				$result['mem']['total'])) {
			// See https://github.com/giampaolo/psutil/blob/release-5.9.0/psutil/_pslinux.py#L521
			$result['mem']['available'] = $result['mem']['free'];
		}
		$result['mem']['utilization'] = $calcUtilization(
			$result['mem']['total'], $result['mem']['available']);

		$result['swap']['total'] = $memInfo['SwapTotal'];
		$result['swap']['free'] = $memInfo['SwapFree'];
		$result['swap']['used'] = bcsub($result['swap']['total'],
			$result['swap']['free']);
		$result['swap']['utilization'] = $calcUtilization(
			$result['swap']['total'], $result['swap']['used']);

		$result['total']['total'] = bcadd($result['mem']['total'],
			$result['swap']['total']);
		$result['total']['used'] = bcadd($result['mem']['used'],
			$result['swap']['used']);
		$result['total']['free'] = bcadd($result['mem']['free'],
			$result['swap']['free']);
		$result['total']['utilization'] = $calcUtilization(
			$result['total']['total'], $result['total']['used']);

		return $result;
*/
		$cmd = new \OMV\System\Process("free", "-b", "-t", "-w");
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		// The utilization is calculated based on
		// https://github.com/giampaolo/psutil/blob/release-5.9.0/psutil/_pslinux.py#L523
		$calcUtilization = function($total, $avail, $scale = 5) {
			if (0 == bccomp($total, "0")) {
				return "0";
			}
			return bcdiv(bcsub($total, $avail), $total, $scale);
		};
		// Parse command output:
		//               total        used        free      shared     buffers       cache   available
		// Mem:     2101825536    98390016  1823117312    12967936    16809984   163508224  1844318208
		// Swap:             0           0           0
		// Total:   2101825536    98390016  1823117312
		//
		// Some information about the 'Mem' metrics.
		// See https://psutil.readthedocs.io/en/latest/#psutil.virtual_memory
		//
		// total     - Total physical memory (exclusive swap).
		// available - The memory that can be given instantly to processes
		//             without the system going into swap.
		// used      - Memory used. 'total - free' does not necessarily
		//             match used.
		// free      - Memory not being used at all (zeroed) that is
		//             readily available; note that this doesn’t reflect
		//             the actual memory available (use available
		//             instead). 'total - used' does not necessarily match
		//             free.
		// shared    - Memory that may be simultaneously accessed by
		//             multiple processes.
		// cache     - Cache for various things.
		foreach ($output as $outputk => $outputv) {
			$outputv = trim($outputv);
			if (preg_match('/^Mem:\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)$/i',
					$outputv, $matches)) {
				$result['mem'] = [
					"total" => $matches[1],
					"used" => $matches[2],
					"free" => $matches[3],
					"shared" => $matches[4],
					"buffers" => $matches[5],
					"cache" => $matches[6],
					"available" => $matches[7],
					"utilization" => $calcUtilization($matches[1],
						$matches[7])
				];
			} else if (preg_match('/^Swap:\s+(\d+)\s+(\d+)\s+(\d+)$/i',
					$outputv, $matches)) {
				$result['swap'] = [
					"total" => $matches[1],
					"used" => $matches[2],
					"free" => $matches[3],
					"utilization" => $calcUtilization($matches[1],
						$matches[2])
				];
			} else if (preg_match('/^Total:\s+(\d+)\s+(\d+)\s+(\d+)$/i',
					$outputv, $matches)) {
				$result['total'] = [
					"total" => $matches[1],
					"used" => $matches[2],
					"free" => $matches[3],
					"utilization" => $calcUtilization($matches[1],
						$matches[2])
				];
			}
		}
		return $result;
	}

	/**
	 * Get CPU statistics.
	 * @return The CPU statistics as an array.
	 *   array(
	 *     mhz => "2792.999"
	 *     cores => 4
	 *     modelname => "Intel(R) Core(TM) i7 CPU         860  @ 2.80GHz",
	 *     utilization => 12
	 *   )
	 */
	public static function getCpuStats() {
		$cpuInfo = new \OMV\Dictionary(\OMV\System\Cpu::info());
		// Get the processor name.
		// Example 1:
		// ...
		// CPU part	: 0xd03
		// CPU revision	: 4
		//
		// Hardware		: Allwinnersun50iw2Family
		// Revision		: 0000
		// Serial		: 0000000000000000
		// ...
		//
		// Example 2:
		// processor	: 0
		// vendor_id	: AuthenticAMD
		// cpu family	: 23
		// model		: 96
		// model name	: AMD Ryzen 7 PRO 4750U with Radeon Graphics
		//
		// Example 3:
		// processor    : 0
		// BogoMIPS     : 108.00
		//
		// Revision     : d03141
		// Serial       : xxxxxxxxxxxxxxxx
		// Model        : Raspberry Pi Compute Module 4 Rev 1.1
		$modelName = NULL;
		foreach ([ "model_name", "cpu_model", "hardware", "model" ] as $key) {
			if (TRUE === $cpuInfo->exists($key)) {
				$modelName = $cpuInfo->get($key);
				break;
			}
		}
		return [
			"modelname" => $modelName,
			"mhz" => $cpuInfo->get("cpu_mhz"),
			"cores" => $cpuInfo->get("cpu_cores"),
			"utilization" => \OMV\System\Cpu::utilization()
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
