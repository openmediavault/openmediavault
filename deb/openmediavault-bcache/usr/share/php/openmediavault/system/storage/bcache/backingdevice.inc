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

require_once("openmediavault/functions.inc");

/**
 * Class to handle a bcache backing device.
 */
class BackingDevice extends BcacheDevice {
	/**
	 * Creates a backing device.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function create() {
		$cmdArgs = [];
		$cmdArgs[] = "-B";
		$cmdArgs[] = $this->getDeviceFile();
		$cmd = new \OMV\System\Process("make-bcache", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Wait some time to get the device registered.
		waitUntil(10, [$this, "isRegistered"]);
	}

	/**
	 * Enable caching with the given cache device.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function attach($csetUuid) {
		$this->setValue("attach", $csetUuid);
	}

	/**
	 * Get the device file of the bcache device.
	 * @return string Returns the device file.
	 * @throw \OMV\ExecException
	 */
	public function getBcacheDeviceFile() {
		$path = "/sys/block/{$this->getDeviceName(TRUE)}/bcache/dev";
		if (!file_exists($path)) {
			return "";
		}
		$file = new \SplFileInfo($path);
		// link target: ../../../../../../../../../../../virtual/block/bcache0
		$linkTarget = $file->getLinkTarget();
		$arr = explode(DIRECTORY_SEPARATOR, $linkTarget);
		return sprintf("/dev/%s", $arr[count($arr) - 1]);
	}

	/**
	 * Get the device file of the cache device.
	 * @return string Returns the device file.
	 * @throw \OMV\ExecException
	 */
	public function getCacheDeviceFile() {
		$path = sprintf("/sys/block/%s/bcache/cache/cache0",
			$this->getDeviceName(TRUE));
		if (!file_exists($path)) {
			return "";
		}
		$file = new \SplFileInfo($path);
		// link target: ../../../devices/virtual/block/<device name>/bcache
		$linkTarget = $file->getLinkTarget();
		$arr = explode(DIRECTORY_SEPARATOR, $linkTarget);
		return sprintf("/dev/%s", $arr[count($arr) - 2]);
	}

	/**
	 * The backing device can be in one of four different states:
	 * no cache: Has never been attached to a cache set.
	 * clean: Part of a cache set, and there is no cached dirty data.
	 * dirty: Part of a cache set, and there is cached dirty data.
	 * inconsistent: The backing device was forcibly run by the user
	 *   when there was dirty data cached but the cache set was
	 *   unavailable; whatever data was on the backing device has
	 *   likely been corrupted.
	 * @return string The state.
	 * @throw \OMV\ExecException
	 */
	public function getState() {
		if (!$this->isRegistered()) {
			return 'unregistered';
		}
		if (!$this->isRunning()) {
			return 'missing cache';
		}
		return $this->getValue("state");
	}

	/**
	 * If bcache is running (i.e. whether the /dev/bcache device exists,
	 * whether it’s in passthrough mode or caching.
	 * @return boolean True if bcache is running, otherwise false.
	 * @throw \OMV\ExecException
	 */
	public function isRunning() {
		return $this->getValue("running") == 1;
	}

	/**
	 * Shut down the bcache device and close the backing device.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function stop() {
		$this->setValue("stop", 1);
		// Wait some time to get the device unregistered.
		waitUntil(10, function() { return !$this->isRegistered(); });
	}

	/**
	 * General function to get sys values.
	 * @param string name of value.
	 * @param boolean when true return only first line.
	 * @return A output array.
	 * @throw \OMV\ExecException
	 */
	public function getValue($name, $firstValue = TRUE) {
		$filename = "/sys/block/{$this->getDeviceName(TRUE)}/bcache/{$name}";
		$cmd = new \OMV\System\Process("cat", $filename);
		$cmd->setQuiet(TRUE);
		$cmd->execute($output);
		return $firstValue ? $output[0] : $output;
	}

	/**
	 * General function to set values.
	 * @param string name of value.
	 * @param string|int the value.
	 * @return void
	 * @throw \OMV\ExecException
	 */
	public function setValue($name, $value) {
		$filename = "/sys/block/{$this->getDeviceName(TRUE)}/bcache/{$name}";
		$cmdLine = sprintf("echo %s > %s", $value, $filename);
		$cmd = new \OMV\System\Process($cmdLine);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Can be one of either writethrough, writeback, writearound or none.
	 * @return string On of the cache modes.
	 * @throw \OMV\ExecException
	 */
	public function getCacheMode() {
		$output = $this->getValue("cache_mode");
		$pattern = '{\[(\w+)\]}';
		if (preg_match($pattern, $output, $matches)) {
			return $matches[1];
		}
		return $output;
	}

	/**
	 * A sequential IO will bypass the cache once it passes this threshold.
	 * @return integer threshold in bytes.
	 * @throw \OMV\ExecException
	 */
	public function getSequentialCutoff() {
		$output = $this->getValue("sequential_cutoff");
		$pattern = '{(\d*(?:\.\d+)?)([kMG])}';
		$value = 0;
		$unit = "";
		if (preg_match($pattern, $output, $matches)) {
			$value = (float) $matches[1];
			$unit = $matches[2];
		}
		switch ($unit) {
            case 'G':
                $value *= 1024;
            case 'M':
                $value *= 1024;
			case 'k':
                $value *= 1024;
            default:
                break;
        }
		$value = ceil($value);
		return $value;
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
	 * Enumerate bcache backing devices.
	 * @return A list of bcache backing devices.
	 * @throw \OMV\ExecException
	 */
	public static function enumerateBcache() {
		$objects = BcacheDevice::enumerateBcache();
		$result = [];
		foreach ($objects as $objectk => $objectv) {
			if ($objectv['bcachetype'] !== 'backing') {
				continue;
			}
			$result[] = $objectv;
		}
		return $result;
	}
}
