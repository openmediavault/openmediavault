<?php

/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Michael Myrcik <michael.myrcik@web.de>
 * @copyright Copyright (c) 2009-2026 Volker Theile
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
 * along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.
 */

namespace OMV\System\Storage\Bcache;

require_once("openmediavault/functions.inc");

/**
 * Class to handle a bcache cache device.
 */
class CacheDevice extends BcacheDevice
{
    public function getDescription()
    {
        return sprintf(
            gettext("Bcache cache device [%s, %s]"),
            $this->getDeviceFile(),
            binary_format($this->getSize())
        );
    }

    /**
     * Creates a backing device.
     * @return void
     * @throws RuntimeException
     * @throws \OMV\ExecException
     */
    public function create()
    {
        $cmdArgs = [];
        $cmdArgs[] = "-C";
        $cmdArgs[] = escapeshellarg($this->getDeviceFile());
        $cmd = new \OMV\System\Process("make-bcache", $cmdArgs);
        $cmd->setRedirect2to1();
        $cmd->execute();
        // Wait some time to get the device registered.
        waitUntil(10, [$this, "isRegistered"]);
    }

    /**
     * Get the state of cache device.
     * @return The state of cache device.
     * @throws \OMV\ExecException
     */
    public function getState()
    {
        return $this->isRegistered() ? 1 : 2;
    }

    /**
     * Shut down the cache set.
     * Waits until all attached backing devices have been shut down.
     * @return void
     * @throws RuntimeException
     */
    public function stop()
    {
        $this->setValue("stop", 1);
        // Wait some time to get the device unregistered.
        waitUntil(10, function () {
            return !$this->isRegistered();
        });
    }

    /**
     * Get a list of backing devices cached by this device.
     * @return A list of devicefiles.
     * Example: array(
     *   0 => /dev/sda
     *   1 => /dev/sdv
     * )
     * @throws \OMV\ExecException
     */
    public function getBackingDevices()
    {
        $cmdArgs = [];
        $cmdArgs[] = sprintf(
            "/sys/fs/bcache/%s/bdev*/backing_dev_name",
            $this->getCsetUuid()
        );
        $cmd = new \OMV\System\Process("cat", $cmdArgs);
        $cmd->setQuiet(true);
        $cmd->setRedirect2to1();
        $cmd->execute($output, $exitStatus);
        $result = [];
        if ($exitStatus !== 0) {
            return $result;
        }
        foreach ($output as $outputk => $outputv) {
            $result[] = sprintf("/dev/%s", trim($outputv));
        }
        return $result;
    }

    /**
     * Get index of the symlink to the backing device.
     * @param string device name.
     * @return A int value.
     */
    public function getBackingDeviceIndex($deviceName)
    {
        // E.g.: /sys/fs/bcache/<cset-uuid>/bdev*
        $path = sprintf("/sys/fs/bcache/%s", $this->getCsetUuid());
        foreach (new \DirectoryIterator($path) as $file) {
            if ($file->isDot() || !$file->isLink()) {
                continue;
            }
            if (1 !== preg_match("/^bdev[0-9]+$/", $file->getFilename())) {
                continue;
            }
            // E.g.: .... /target8:0:0/8:0:0:0/block/sdg/bcache
            if (basename(dirname($file->getLinkTarget())) !== $deviceName) {
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
     */
    public function getValue($name, $firstValue = true)
    {
        $filename = build_path(
            DIRECTORY_SEPARATOR,
            "/sys/fs/bcache",
            $this->getCsetUuid(),
            $name
        );
        $output = file($filename, FILE_IGNORE_NEW_LINES);
        return $firstValue ? $output[0] : $output;
    }

    /**
     * General function to set sys values.
     * @param string name of value.
     * @param string|int the value.
     * @return int|false The number of bytes written, or false on failure.
     */
    public function setValue($name, $value)
    {
        $filename = build_path(
            DIRECTORY_SEPARATOR,
            "/sys/fs/bcache",
            $this->getCsetUuid(),
            $name
        );
        return file_put_contents($filename, $value);
    }

    /**
     * Get statistic of the bcache device.
     * @return The statistic of the bcache device.
     * @throws \OMV\ExecException
     */
    public function getStatistic()
    {
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
     * @throws \OMV\ExecException
     */
    private function getStatisticValue($name)
    {
        $values = $this->getValue("stats_*/{$name}", false);
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
     */
    public function getPriorityState()
    {
        $output = $this->getValue("cache0/priority_stats", false);
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
     * @throws \OMV\ExecException
     */
    public static function enumerateBcache()
    {
        $objects = BcacheDevice::enumerateBcache();
        $result = [];
        foreach ($objects as $objectk => $objectv) {
            if ($objectv['bcachetype'] !== 'cache') {
                continue;
            }
            $sd = new CacheDevice($objectv['devicefile']);
            $result[] = array_merge($objectv, [
                "description" => $sd->getDescription()
            ]);
        }
        return $result;
    }
}
