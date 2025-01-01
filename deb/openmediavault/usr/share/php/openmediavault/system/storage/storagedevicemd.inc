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
namespace OMV\System\Storage;

/**
 * This class provides a simple interface to handle MD (Multiple Device)
 * software RAID devices.
 * @ingroup api
 */
class StorageDeviceMd extends StorageDevice {
	protected $name = "";
	protected $level = FALSE;
	protected $devices = [];
	protected $numDevices = FALSE;
	protected $uuid = FALSE;
	protected $state = "unknown";
	protected $hasWriteIntentBitmap = FALSE;
	protected $details = [];
	private $dataCached = FALSE;

	/**
	 * Get the software RAID device detailed information.
	 * @private
	 * @return void
	 * @throw \OMV\ExecException
	 */
	private function getData() {
		if ($this->dataCached !== FALSE)
			return;

		// ARRAY /dev/md0 level=raid5 num-devices=3 metadata=1.2 name=xxxx:0 UUID=a4266bf7:c671b343:c3d6e535:ca455e37
		//    devices=/dev/sdb,/dev/sdc,/dev/sdd
		$cmdArgs = [];
		$cmdArgs[] = "--detail";
		$cmdArgs[] = "--brief";
		$cmdArgs[] = "--verbose";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("mdadm", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$attributes = [
			"name" => "",
			"level" => "",
			"num-devices" => -1,
			"uuid" => "",
			"devices" => ""
		];
		$output = explode(" ", implode(" ", $output));
		foreach ($output as $outputk => &$outputv) {
			$keyValue = explode("=", $outputv);
			if (count($keyValue) != 2)
				continue;
			$attributes[mb_strtolower($keyValue[0])] = $keyValue[1];
		}

		$this->name = $attributes['name'];
		$this->level = $attributes['level'];
		$this->numDevices = intval($attributes['num-devices']);
		$this->uuid = $attributes['uuid'];
		$this->devices = explode(",", $attributes['devices']);

		// Sort the devices using a "natural order" algorithm.
		sort($this->devices, SORT_NATURAL);

		// Get more information.
		$cmdArgs = [];
		$cmdArgs[] = "--detail";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("mdadm", $cmdArgs);
		$cmd->setRedirect2to1();
		unset($output);
		$cmd->execute($output);

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
		foreach ($output as $outputk => $outputv) {
			if (empty($outputv))
				continue;
			$parts = explode(":", $outputv);
			if (empty($parts))
				continue;
			$parts = array_map("trim", $parts);
			switch (mb_strtolower($parts[0])) {
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
	 * @return void
	 */
	public function refresh() {
		$this->dataCached = FALSE;
		$this->getData();
	}

	/**
	 * See interface definition.
	 */
	public function exists() {
		try {
			$this->getData();
		} catch(\Exception $e) {
			return FALSE;
		}
		return ($this->level !== FALSE);
	}

	/**
	 * Get the array name.
	 * @return The array name.
	 */
	public function getName() {
		$this->getData();
		return $this->name;
	}

	/**
	 * Get the level of the array.
	 * @return The level of the array.
	 */
	public function getLevel() {
		$this->getData();
		return $this->level;
	}

	/**
	 * Get the number of active devices in the array.
	 * @return The number of active devices in the array.
	 */
	public function getNumDevices() {
		$this->getData();
		return $this->numDevices;
	}

	/**
	 * Get the UUID of the array.
	 * @return The UUID of the array.
	 */
	public function getUuid() {
		$this->getData();
		return $this->uuid;
	}

	/**
	 * See abstract class definition.
	 */
	public function getSerialNumber() {
		return $this->getUuid();
	}

	/**
	 * Get the slave devices of this device.
	 * @return A list of device files used in the array.
	 */
	public function getSlaves() {
		$this->getData();
		return $this->devices;
	}

	public function getState() {
		$this->getData();
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
		//
		// Testing:
		// --------
		// - Get a degraded array
		//   # mdadm /dev/md0 -f /dev/sde
		//   # echo 1 > /sys/block/sde/device/delete
		// - Get an inactive array
		//   # echo 1 > /sys/block/sdX/device/delete
		//   # echo 1 > /sys/block/sdY/device/delete
		//   Use UI to scan for new devices. The previously removed devices
		//   should reappear and the array becomes inactive.
		$regex = sprintf('/^(%s\s*:.*?)^\s*$/ims', $this->getDeviceName(TRUE));
		if (1 !== preg_match($regex, file_get_contents("/proc/mdstat"),
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
			if ("PENDING" == $progress) {
				# The state is already in proper form.
	  			# # cat /proc/mdstat
	  			# Personalities : [raid6] [raid5] [raid4]
	  			# md0 : active (auto-read-only) raid5 sda[0] sdd[3](S) sdc[2] sdb[1]
	  			#       2095104 blocks super 1.2 level 5, 512k chunk, algorithm 2 [3/3] [UUU]
	  			#       	resync=PENDING
	  			#
	  			# # mdadm --state
	  			# ...
	  			# Update Time : Fri Dec  2 15:32:42 2016
	  			# State : clean, resyncing (PENDING)
	  			# Active Devices : 3
	  			# Working Devices : 4
	  			# Failed Devices : 0
	  			# Spare Devices : 1
	  			# ...
			} else {
				$state = sprintf("%s (%s)", $state, $progress);
			}
		} else if (FALSE !== stristr($mdstat, "resync")) {
			$state = sprintf("%s, resyncing (%s)", $state, $progress);
		}
		return $state;
	}

	/**
	 * Get detail of the md device.
	 * @return The detail of the md device.
	 */
	public function getDetail() {
		$this->getData();
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
		$this->getData();
		return $this->hasWriteIntentBitmap;
	}
}
