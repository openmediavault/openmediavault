<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Michael Myrcik <michael.myrcik@web.de>
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
namespace OMV\System\Storage\Bcache;

/**
 * Class to handle a bcache cache device.
 */
class CacheDevice extends BcacheDevice {
	/**
	 * Creates a backing device.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function create() {
		$cmdArgs = [];
		$cmdArgs[] = "-C";
		$cmdArgs[] = $this->getDeviceFile();
		$cmd = new \OMV\System\Process("make-bcache", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Wait some time to get the device registered.
		waitUntil(10, [$this, "isRegistered"]);
	}

	/**
	 * Get the state of cache device.
	 * @return The state of cache device.
	 * @throw \OMV\ExecException
	 */
	public function getState() {
		return $this->isRegistered() ? 1 : 2;
	}

	/**
	 * Shut down the cache set.
	 * Waits until all attached backing devices have been shut down.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function stop() {
		$this->setValue("stop", 1);
		// Wait some time to get the device unregistered.
		waitUntil(10, function() { return !$this->isRegistered(); });
	}

	/**
	 * Get a list of backing devices cached by this device.
	 * @return A list of devicefiles.
	 * Example: array(
	 *   0 => /dev/sda
	 *   1 => /dev/sdv
	 * )
	 * @throw \OMV\ExecException
	 */
	public function getBackingDevices() {
		$filename = "/sys/fs/bcache/{$this->getCsetUuid()}/bdev*/".
			"backing_dev_name";
		$cmdLine = sprintf("cat %s", $filename);
		$cmd = new \OMV\System\Process($cmdLine);
		$cmd->setQuiet(TRUE);
		$cmd->setRedirect2to1();
		$cmd->execute($output, $exitStatus);
		$result = [];
		if ($exitStatus !== 0) {
			return $result;
		}
		foreach($output as $outputk => $outputv) {
			$result[] = sprintf("/dev/%s", trim($outputv));
		}
		return $result;
	}

	/**
	 * Get index of the symlink to the backing device.
	 * @param string device name.
	 * @return A int value.
	 */
	public function getBackingDeviceIndex($deviceName) {
		// /sys/fs/bcache/<cset-uuid>/bdev*
		$path = sprintf("/sys/fs/bcache/%s", $this->getCsetUuid());
		foreach (new \DirectoryIterator($path) as $file) {
			if ($file->isDot() || !$file->isLink()) {
				continue;
			}
			if (1 !== preg_match("/^bdev[0-9]+$/", $file->getFilename())) {
				continue;
			}
			// .... /target8:0:0/8:0:0:0/block/sdg/bcache
			$linkTarget = $file->getLinkTarget();
			$arr = explode(DIRECTORY_SEPARATOR, $linkTarget);
			if ($arr[count($arr) - 2] !== $deviceName) {
				continue;
			}
			return substr($file->getFilename(), 4);
		}
		return "";
	}

	/**
	 * General function to get sys values.
	 * @param string name of value.
	 * @param boolean when true return only first line.
	 * @return A value.
	 * @throw \OMV\ExecException
	 */
	public function getValue($name, $firstValue = TRUE) {
		$filename = "/sys/fs/bcache/{$this->getCsetUuid()}/{$name}";
		$cmd = new \OMV\System\Process("cat", $filename);
		$cmd->setQuiet(TRUE);
		$cmd->execute($output);
		return $firstValue ? $output[0] : $output;
	}

	/**
	 * General function to set sys values.
	 * @param string name of value.
	 * @param string|int the value.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function setValue($name, $value) {
		$filename = "/sys/fs/bcache/{$this->getCsetUuid()}/{$name}";
		$cmdLine = sprintf("echo %s > %s", $value, $filename);
		$cmd = new \OMV\System\Process($cmdLine);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Get statistic of the bcache device.
	 * @return The statistic of the bcache device.
	 * @throw \OMV\ExecException
	 */
	public function getStatistic() {
		$result = [];
		$result[] = $this->getStatisticValue("bypassed");
		$result[] = $this->getStatisticValue("cache_bypass_hits");
		$result[] = $this->getStatisticValue("cache_bypass_misses");
		$result[] = $this->getStatisticValue("cache_hit_ratio");
		$result[] = $this->getStatisticValue("cache_hits");
		$result[] = $this->getStatisticValue("cache_miss_collisions");
		$result[] = $this->getStatisticValue("cache_misses");
		return $result;
	}

	/**
	 * Get statistics for a name.
	 * @param string name of the statistic.
	 * @return An array.
	 * @throw \OMV\ExecException
	 */
	private function getStatisticValue($name) {
		$values = $this->getValue("stats_*/{$name}", FALSE);
		return [
			"name" => $name,
			"fiveminutes" => $values[1],
			"hour" => $values[2],
			"day" => $values[0],
			"total" => $values[3]
		];
	}

	/**
	 * Get Statistics about how recently data in the cache has been accessed.
	 * @return An array.
	 * @throw \OMV\ExecException
	 */
	public function getPriorityState() {
		$output = $this->getValue("cache0/priority_stats", FALSE);
		$attributes = [];
		foreach ($output as $outputk => $outputv) {
			$keyValue = preg_split('/:/', $outputv);
			if (count($keyValue) != 2) {
				continue;
			}
			$key = mb_strtolower(trim($keyValue[0]));
			$value = trim($keyValue[1]);
			$attributes[$key] = $value;
		}
		return $attributes;
	}

	/**
	 * Enumerate bcache cache devices.
	 * @return A list of bcache cache devices.
	 * @throw \OMV\ExecException
	 */
	public static function enumerateBcache() {
		$objects = BcacheDevice::enumerateBcache();
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			if ($objectv['bcachetype'] !== 'cache') {
				continue;
			}
			$result[] = $objectv;
		}
		return $result;
	}
}
