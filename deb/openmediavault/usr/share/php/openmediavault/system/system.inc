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
namespace OMV\System;

require_once("openmediavault/functions.inc");

/**
 * Helper class that provides various functions regarding the Linux system.
 * @ingroup api
 */
class System {
	/**
	 * Get the device file where the operating system is installed on (e.g.
	 * /dev/sda1).
	 * @return Returns the device file where the operating system is installed
	 *   on or FALSE on failure.
	 */
	public static function getRootDeviceFile() {
		if (is_block("/dev/root"))
			return realpath("/dev/root");
		$cmd = new \OMV\System\Process("findmnt", "-f", "-n",
		  "-o SOURCE", "/");
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute($output);
		return $output[0];
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
		return $file->getAssoc();
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
			"iface" => "export LANG=C; netstat -i | awk '{print $1}'"
		);
		$cmd = sprintf("%s | grep '^%s[0-9]\{1,\}$' | sort -r | " .
		  "head -n 1", $cmdList[$type], $name);
		$cmd = new \OMV\System\Process($cmd);
		$deviceName = $cmd->execute();
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
		if (FALSE === $indexed)
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
		$cmd = new \OMV\System\Process("free", "-b");
		$cmd->setRedirect2to1(TRUE);
		$cmd->execute($output);
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
	public static function getCpuStats() {
		// Get the CPU information. Take care about CPUs with multiple cores.
		// processor	: 0
		// vendor_id	: GenuineIntel
		// cpu family	: 6
		// model		: 30
		// model name	: Intel(R) Core(TM) i7 CPU         860  @ 2.80GHz
		// stepping	: 5
		// microcode	: 0x3
		// cpu MHz		: 2792.999
		$cpuInfo = new \OMV\Util\KeyValueFile("/proc/cpuinfo", ":");
		$cpuInfo->setKeyNameDelimiter("");
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
		$diffTotal = $tnowTotal - $tprevTotal;
		return array(
			"modelname" => $cpuInfo->get("modelname"),
			"cpumhz" => $cpuInfo->get("cpumhz"),
			"usage" => (0 == $diffTotal) ? 0 : (($diffTotal - $diffIdle) /
			  $diffTotal) * 100
		);
	}
}
