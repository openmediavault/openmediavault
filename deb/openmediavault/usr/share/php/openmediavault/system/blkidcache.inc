<?php

/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Karol Olszacki <karol@olszacki.pl>
 * @copyright Copyright (c) 2026 Karol Olszacki
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

namespace OMV\System;

/**
 * Cache for the output of the 'blkid' command.
 *
 * A 'blkid' probe on a device that does not answer (a disk with unreadable
 * sectors, a USB bridge that is slow to wake up) blocks the caller in
 * uninterruptible disk wait. The process can not be killed there, so the
 * number of probes running at the same time has to be limited instead of
 * just bounding their runtime.
 *
 * The result of a probe is therefore cached per key and only one probe per
 * key runs at a time. The cache is shared by all processes, because the
 * engine forks a child per RPC request.
 */
class BlkidCache
{
    /**
     * Get the directory that holds the cache and lock files.
     * @return string The absolute path of the cache directory.
     */
    public static function getDirectory(): string
    {
        // '/run' is a tmpfs: the cache does not wear the SSD and it is
        // gone after a reboot, so no device name from a previous boot can
        // be mistaken for a current one.
        return \OMV\Environment::getString(
            "OMV_BLKID_CACHE_DIR",
            "/run/openmediavault/blkid-cache"
        );
    }

    /**
     * Get the name of the file that holds the cached result of a key.
     * @param string $key The cache key.
     * @return string The absolute path of the cache file.
     */
    public static function getCacheFile(string $key): string
    {
        return sprintf("%s/%s.json", self::getDirectory(), sha1($key));
    }

    /**
     * Get the name of the lock file that serializes the probes of a key.
     *
     * The lock lives in '/run/lock' and not next to the cache: if the
     * cache directory was removed while a probe was running, the next
     * caller would create a new lock file and both would hold a different
     * inode, which silently breaks the mutual exclusion. A lock file is
     * never removed, because unlinking a locked file breaks it as well.
     * @param string $key The cache key.
     * @return string The absolute path of the lock file.
     */
    public static function getLockFile(string $key): string
    {
        return sprintf("/run/lock/omv-blkid-%s.lock", sha1($key));
    }

    /**
     * Get the time in seconds a cached result is considered fresh.
     * @return int The cache lifetime in seconds.
     */
    public static function getTtl(): int
    {
        return \OMV\Environment::getInteger(
            "OMV_BLKID_SCAN_CACHE_TTL",
            30
        );
    }

    /**
     * Get a stable identifier for a device.
     *
     * The kernel device name is not stable: it changes with the
     * enumeration order and with every hot-plug, so it must not be used
     * as a cache key. The '/dev/disk/by-id' path is stable as long as the
     * device keeps its serial number, and the '/dev/disk/by-uuid' path is
     * stable for the same reason.
     *
     * @param string $deviceFile The device file to identify.
     * @return string A stable identifier for the device.
     */
    public static function getStableDeviceId(string $deviceFile): string
    {
        $real = realpath($deviceFile);
        if (false === $real) {
            // The device is gone, nothing to identify.
            return $deviceFile;
        }
        // The by-id names are preferred in a fixed order, so that the
        // identifier of a device does not depend on the order in which the
        // directory happens to be read. 'wwn-' is the most canonical one
        // (it is derived from the world wide name of the device).
        $prefixes = ["wwn-", "ata-", "usb-", "nvme-", "scsi-", "dm-name-", "md-uuid-"];
        $links = [];
        foreach (glob("/dev/disk/by-id/*") as $link) {
            if (!is_link($link)) {
                continue;
            }
            $target = realpath($link);
            if (false === $target || $target !== $real) {
                continue;
            }
            $name = basename($link);
            foreach ($prefixes as $pos => $prefix) {
                if (str_starts_with($name, $prefix)) {
                    // A lower position wins, a shorter name breaks the tie.
                    $rank = [$pos, strlen($name)];
                    if (!isset($links[$link]) || $rank < $links[$link]) {
                        $links[$link] = $rank;
                    }
                }
            }
        }
        if (!empty($links)) {
            // Sort by rank (the value), not by the file name.
            asort($links);
            return array_key_first($links);
        }
        // No by-id name: the file system UUID is the next best identity.
        foreach (glob("/dev/disk/by-uuid/*") as $link) {
            if (is_link($link) && realpath($link) === $real) {
                return $link;
            }
        }
        return $deviceFile;
    }

    /**
     * Read the cached result of a key.
     * @param string $key The cache key.
     * @return array|null An array with the keys 'data' and 'mtime', or NULL
     *   if there is no usable cache entry.
     */
    public static function get(string $key): ?array
    {
        $cacheFile = self::getCacheFile($key);
        $mtime = @filemtime($cacheFile);
        if (false === $mtime) {
            return null;
        }
        $content = @file_get_contents($cacheFile);
        if (false === $content) {
            return null;
        }
        $decoded = json_decode($content, true);
        if (!is_array($decoded)) {
            return null;
        }
        return [
            "data" => $decoded,
            "mtime" => $mtime
        ];
    }

    /**
     * Refresh the age of a cache entry, e.g. when a stale result is served
     * because a fresh scan did not finish in time.
     * @param string $key The cache key.
     * @return void
     */
    public static function touch(string $key): void
    {
        $cacheFile = self::getCacheFile($key);
        if (file_exists($cacheFile)) {
            @touch($cacheFile);
        }
    }

    /**
     * Store the result of a probe. The file is written atomically, so a
     * concurrent reader never sees a partially written entry.
     * @param string $key The cache key.
     * @param array $data The result to cache.
     * @return void
     */
    public static function put(string $key, array $data): void
    {
        $cacheFile = self::getCacheFile($key);
        $dir = dirname($cacheFile);
        if (!is_dir($dir)) {
            @mkdir($dir, 0700, true);
        }
        $tmpFile = $cacheFile . "." . getmypid() . ".tmp";
        if (false === @file_put_contents($tmpFile, json_encode($data))) {
            return;
        }
        @rename($tmpFile, $cacheFile);
    }

    /**
     * Check whether a cached result is still fresh enough to be served.
     * @param array $entry The cache entry as returned by \ref get.
     * @return bool TRUE if the entry is fresh.
     */
    public static function isFresh(array $entry): bool
    {
        return (time() - $entry['mtime']) < self::getTtl();
    }

    /**
     * Try to acquire the lock of a key without blocking.
     * @param string $key The cache key.
     * @return resource|false|null The lock handle, FALSE if the lock is
     *   held by somebody else, or NULL if the lock file cannot be opened
     *   at all.
     */
    public static function tryLock(string $key)
    {
        // The lock lives in '/run/lock', which the system provides.
        $lock = @fopen(self::getLockFile($key), "c");
        if (false === $lock) {
            // The lock file cannot be opened, the cache is unusable.
            return null;
        }
        if (false === flock($lock, LOCK_EX | LOCK_NB)) {
            // Somebody else holds the lock.
            fclose($lock);
            return false;
        }
        return $lock;
    }

    /**
     * Release a lock that was acquired with \ref tryLock.
     * @param resource $lock The lock handle.
     * @return void
     */
    public static function releaseLock($lock): void
    {
        if (false === $lock) {
            return;
        }
        flock($lock, LOCK_UN);
        fclose($lock);
    }
}
