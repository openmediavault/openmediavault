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

/**
 * @ingroup api
 */
class Cpu {
	/**
	 * Get the aggregated CPU statistics.
	 */
	public static function stats(): array {
		// Use the very first "cpu" line which aggregates the numbers in
		// all of the other "cpuN" lines.
		//
		// Example:
		// cpu  3163 4 2087 120087 470 0 33 15 0 0
		// cpu0 3163 4 2087 120087 470 0 33 15 0 0
		// ...
		//
		// * user: Time spent in user mode
		// * nice: Time spent processing nice processes in user mode
		// * system: Time spent executing kernel code
		// * idle: Time spent idle
		// * iowait: Time spent waiting for I/O
		// * irq: Time spent servicing interrupts
		// * softirq: Time spent servicing software interrupts
		// * steal: Time stolen from a virtual machine
		// * guest: Time spent running a virtual CPU for a guest operating system
		// * guest_nice: Time spent running a virtual CPU for a “niced” guest operating system
		$stat = file("/proc/stat");
		$stat = explode(" ", $stat[0]);
		array_shift($stat); array_shift($stat);
		$stat = array_map("intval", $stat);
		return [
			"user" => $stat[0],
			"nice" => $stat[1],
			"system" => $stat[2],
			"idle" => $stat[3],
			"iowait" => $stat[4],
			"irq" => $stat[5],
			"softirq" => $stat[6],
			"steal" => $stat[7],
			"guest" => $stat[8],
			"guest_nice" => $stat[9],
			// Calculate the total value. Note that `guest` and `guest_nice`
			// are not included here.
			"total" => array_sum(array_slice($stat, 0, 8))
		];
	}

	/**
	 * Get the content of `/proc/cpuinfo` as an array.
	 */
	public static function info(): array {
		// Example:
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
		$kvFile = new \OMV\Util\KeyValueFile("/proc/cpuinfo", ":");
		$kvFile->setKeyNameDelimiter("_");
		return $kvFile->getAssoc();
	}

	public static function utilization(int $interval = 1): float {
		// - https://serverfault.com/a/667250
		// - https://en.wikipedia.org/wiki/Load_(computing)
		// - https://rosettacode.org/wiki/Linux_CPU_utilization#Python
		// - https://soltveit.org/cpu-idle-time-since-boot/
		// - https://gist.github.com/pcolby/6558833
		// - https://github.com/shirou/gopsutil/blob/master/cpu/cpu.go
		$lastStat = self::stats();
		sleep($interval);
		$stat = self::stats();

		$lastBusy = $lastStat['total'] - $lastStat['idle'] - $lastStat['iowait'];
		$busy = $stat['total'] - $stat['idle'] - $stat['iowait'];
		$busyDiff = $busy - $lastBusy;

		if ($busyDiff <= 0.0) {
			return 0.0;
		}

		$totalDiff = $stat['total'] - $lastStat['total'];

		if ($totalDiff <= 0.0) {
			return 100.0;
		}

		return min(100.0, max(0.0, ($busyDiff / $totalDiff) * 100.0));
	}
}
