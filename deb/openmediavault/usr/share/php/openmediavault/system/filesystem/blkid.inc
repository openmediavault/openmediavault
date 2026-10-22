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

namespace OMV\System\Filesystem;

/**
 * Get the information of the block device identification library
 * (libblkid) about the block devices.
 *
 * The information is NOT probed by running 'blkid', it is read from the
 * udev database instead. udev probes a block device via libblkid (the
 * 'blkid' builtin) whenever the device is added or changed, e.g. after a
 * file system has been created or wiped. Thus, the block devices are not
 * accessed on every request, which may hang for a long time on failing
 * disks.
 *
 * The returned information uses the same (lowercase) keys as 'blkid',
 * e.g. 'uuid', 'label', 'type', 'usage' or 'part_entry_scheme'. Only
 * the keys that are detected are returned.
 *
 * @ingroup api
 */
class Blkid
{
    /**
     * Enumerate all block devices that contain a superblock, e.g. a file
     * system, RAID or LVM signature. This is the equivalent to the
     * command 'blkid -o full'.
     * @return array A list of associative arrays with the fields
     *   \em devicefile, \em type and the detected ones, e.g. \em uuid
     *   and \em label (unescaped).
     * @throw \OMV\ExecException
     */
    public static function enumerate(): array
    {
        $result = [];
        foreach (\OMV\System\Udev::getDevices("block") as $properties) {
            // Skip devices without a file system and incomplete entries.
            if (empty($properties['ID_FS_TYPE']) || empty($properties['DEVNAME'])) {
                continue;
            }
            $result[] = array_merge(
                ["devicefile" => self::getDeviceFile($properties)],
                self::fromUdevProperties($properties)
            );
        }
        return $result;
    }

    /**
     * Get the information of the given block device. This is the
     * equivalent to the command 'blkid -p -o full <DEVICE>'.
     * @param string $deviceFile The device file, e.g. '/dev/sda1'.
     * @return array|null An associative array with the field
     *   \em devicefile and the detected ones, e.g. \em uuid, \em label
     *   (unescaped), \em type, \em usage or \em part_entry_scheme.
     *   Returns NULL if nothing has been detected. Note, like 'blkid',
     *   a partition table (whole disk) or a partition entry is detected,
     *   too, so the returned information does not necessarily contain
     *   the field \em type.
     * @throw \OMV\ExecException
     */
    public static function probe(string $deviceFile): ?array
    {
        $data = self::fromUdevProperties(\OMV\System\Udev::getProperties($deviceFile));
        if (empty($data)) {
            return null;
        }
        return array_merge(["devicefile" => $deviceFile], $data);
    }

    /**
     * Convert the udev properties of a block device to the information
     * that is reported by 'blkid'.
     * @param array $properties The udev properties of the block device.
     * @return array An associative array with the detected information,
     *   e.g. \em uuid, \em label (unescaped), \em type, \em usage or
     *   \em part_entry_scheme.
     */
    public static function fromUdevProperties(array $properties): array
    {
        // Map the udev property names to the blkid tag names. Properties
        // that are encoded by udev are marked.
        // See https://github.com/systemd/systemd/blob/main/src/udev/udev-builtin-blkid.c
        $map = [
            "ID_FS_TYPE" => ["type", false],
            "ID_FS_UUID_ENC" => ["uuid", true],
            "ID_FS_UUID_SUB_ENC" => ["uuid_sub", true],
            "ID_FS_LABEL_ENC" => ["label", true],
            "ID_FS_VERSION" => ["version", false],
            "ID_FS_USAGE" => ["usage", false],
            "ID_PART_ENTRY_SCHEME" => ["part_entry_scheme", false],
            "ID_PART_ENTRY_UUID" => ["part_entry_uuid", false],
            "ID_PART_ENTRY_TYPE" => ["part_entry_type", true],
            "ID_PART_ENTRY_NAME" => ["part_entry_name", true],
            "ID_PART_ENTRY_FLAGS" => ["part_entry_flags", false],
            "ID_PART_ENTRY_NUMBER" => ["part_entry_number", false],
            "ID_PART_ENTRY_OFFSET" => ["part_entry_offset", false],
            "ID_PART_ENTRY_SIZE" => ["part_entry_size", false],
            "ID_PART_ENTRY_DISK" => ["part_entry_disk", false],
        ];
        // Partitions inherit the partition table properties from their
        // parent device, but 'blkid' only reports them for the device
        // that contains the partition table.
        if ("partition" !== ($properties['DEVTYPE'] ?? "")) {
            $map["ID_PART_TABLE_TYPE"] = ["pttype", false];
            $map["ID_PART_TABLE_UUID"] = ["ptuuid", false];
        }
        $result = [];
        foreach ($map as $propertyName => list($tagName, $encoded)) {
            if (!array_key_exists($propertyName, $properties)) {
                continue;
            }
            $value = $properties[$propertyName];
            $result[$tagName] = $encoded ? \OMV\System\Udev::decode($value) : $value;
        }
        // Add the tags that are reported by 'blkid' in high-level
        // (cache-based) mode for partitions.
        if (array_key_exists("part_entry_uuid", $result)) {
            $result['partuuid'] = $result['part_entry_uuid'];
        }
        if (array_key_exists("part_entry_name", $result)) {
            $result['partlabel'] = $result['part_entry_name'];
        }
        return $result;
    }

    /**
     * Get the device file of a block device like 'blkid' reports it.
     * Device mapper devices are reported as '/dev/mapper/<NAME>' instead
     * of '/dev/dm-<N>'.
     * @param array $properties The udev properties of the block device.
     * @return string The device file, e.g. '/dev/sda1' or
     *   '/dev/mapper/vg0-lv0'.
     */
    private static function getDeviceFile(array $properties): string
    {
        if (!empty($properties['DM_NAME'])) {
            $deviceFile = sprintf("/dev/mapper/%s", $properties['DM_NAME']);
            if (is_block_device($deviceFile)) {
                return $deviceFile;
            }
        }
        return $properties['DEVNAME'];
    }
}
