<?php

/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
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

namespace OMV\System;

/**
 * Helper class to access the udev device database. The information
 * is read from the database that is maintained by systemd-udevd, the
 * devices themselves are not accessed.
 * @ingroup api
 */
class Udev
{
    /**
     * Get the properties of all devices from the udev database.
     * @param string|null $subsystem Get only the devices of the given
     *   subsystem, e.g. 'block'. Defaults to NULL.
     * @return array A list of associative arrays containing the
     *   properties of the devices, e.g. DEVNAME, DEVTYPE or ID_FS_TYPE.
     * @throw \OMV\ExecException
     */
    public static function getDevices(?string $subsystem = null): array
    {
        $cmdArgs = [];
        $cmdArgs[] = "info";
        $cmdArgs[] = "--export-db";
        if (!is_null($subsystem)) {
            $cmdArgs[] = sprintf("--subsystem-match=%s", escapeshellarg(
                $subsystem
            ));
        }
        $cmd = new Process("udevadm", $cmdArgs);
        $cmd->setRedirect2to1();
        $cmd->execute($output);
        $result = self::parseDatabase($output);
        // Filter the devices again, the '--subsystem-match' option is
        // only an optimization.
        if (!is_null($subsystem)) {
            $result = array_values(array_filter(
                $result,
                fn ($properties) => ($properties['SUBSYSTEM'] ?? null) === $subsystem
            ));
        }
        return $result;
    }

    /**
     * Get the properties of the given device from the udev database.
     * @param string $deviceFile The device file, e.g. '/dev/sda1'.
     * @return array An associative array containing the properties
     *   of the device, e.g. DEVNAME, DEVTYPE or ID_FS_TYPE.
     * @throw \OMV\ExecException
     */
    public static function getProperties(string $deviceFile): array
    {
        $cmdArgs = [];
        $cmdArgs[] = "info";
        $cmdArgs[] = "--query=property";
        $cmdArgs[] = sprintf("--name=%s", escapeshellarg($deviceFile));
        $cmd = new Process("udevadm", $cmdArgs);
        $cmd->setRedirect2to1();
        $cmd->execute($output);
        return self::parseProperties($output);
    }

    /**
     * Parse the output of 'udevadm info --export-db'.
     * @param array $lines The lines to parse, e.g.
     *   <pre>
     *   P: /devices/virtual/block/loop0
     *   M: loop0
     *   U: block
     *   E: DEVNAME=/dev/loop0
     *   E: SUBSYSTEM=block
     *
     *   P: /devices/virtual/block/loop1
     *   ...
     *   </pre>
     * @return array A list of associative arrays containing the
     *   properties of the devices.
     */
    public static function parseDatabase(array $lines): array
    {
        $result = [];
        $deviceLines = [];
        // Append an empty line to process the last device, too.
        foreach (array_merge($lines, [""]) as $line) {
            if ("" === trim($line)) {
                // An empty line separates the devices.
                $properties = self::parseProperties($deviceLines);
                if (!empty($properties)) {
                    $result[] = $properties;
                }
                $deviceLines = [];
                continue;
            }
            // Collect the property lines of the current device, all
            // other lines are not of interest.
            if (str_starts_with($line, "E: ")) {
                $deviceLines[] = substr($line, 3);
            }
        }
        return $result;
    }

    /**
     * Parse the output of 'udevadm info --query=property'.
     * @param array $lines The lines to parse, e.g.
     *   <pre>
     *   DEVNAME=/dev/sda1
     *   DEVTYPE=partition
     *   ID_FS_TYPE=ext4
     *   </pre>
     * @return array An associative array containing the properties.
     */
    public static function parseProperties(array $lines): array
    {
        $result = [];
        foreach ($lines as $line) {
            $parts = explode("=", $line, 2);
            if (count($parts) !== 2) {
                continue;
            }
            // Trim only the key, do not touch the value.
            $result[trim($parts[0])] = $parts[1];
        }
        return $result;
    }

    /**
     * Decode a string that has been encoded by udev, e.g. the value of
     * the property ID_FS_LABEL_ENC. Characters that are not safe are
     * encoded as '\xNN' by udev, e.g. 'My\x20Data'.
     * @param string $value The encoded string.
     * @return string The decoded string.
     */
    public static function decode(string $value): string
    {
        return preg_replace_callback(
            "/\\\\x([0-9a-fA-F]{2})/",
            fn ($matches) => chr(hexdec($matches[1])),
            $value
        );
    }

    /**
     * Make sure the udev database is up to date for the given devices.
     * Use this after a block device has been modified, e.g. a file system
     * has been created or wiped.
     *
     * A change event is triggered for the given devices and only these
     * events are waited for. Note, 'udevadm settle' is not used because
     * it waits for the whole udev event queue, so a stuck event of an
     * unrelated (e.g. failing) device would block this, too. Additionally
     * the triggered event makes sure the devices are probed after they
     * have been modified.
     * @param array $deviceFiles The device files, e.g. '/dev/sda1'.
     * @param int $timeout The maximum number of seconds to wait.
     *   Defaults to 30.
     * @return void
     * @throw \OMV\ExecException
     */
    public static function settle(array $deviceFiles, int $timeout = 30): void
    {
        // Note, 'udevadm trigger' without devices triggers ALL devices.
        $deviceFiles = array_unique(array_filter($deviceFiles, "strlen"));
        if (empty($deviceFiles)) {
            return;
        }
        $cmdArgs = [];
        $cmdArgs[] = sprintf("%d", $timeout);
        $cmdArgs[] = "udevadm";
        $cmdArgs[] = "trigger";
        $cmdArgs[] = "--action=change";
        $cmdArgs[] = "--settle";
        foreach ($deviceFiles as $deviceFile) {
            $cmdArgs[] = escapeshellarg($deviceFile);
        }
        $cmd = new Process("timeout", $cmdArgs);
        $cmd->setRedirect2to1();
        $cmd->execute();
    }
}
