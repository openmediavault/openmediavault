#!/usr/bin/phpunit -c/etc/openmediavault
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
require_once("openmediavault/autoloader.inc");
require_once("openmediavault/functions.inc");

class test_openmediavault_system_filesystem_blkid extends \PHPUnit\Framework\TestCase
{
    public function test_partition_with_filesystem()
    {
        $data = \OMV\System\Filesystem\Blkid::fromUdevProperties([
            "DEVNAME" => "/dev/loop0p2",
            "DEVTYPE" => "partition",
            "ID_PART_TABLE_UUID" => "d62fb27f-d86d-427f-8d2f-7fb484cb5c80",
            "ID_PART_TABLE_TYPE" => "gpt",
            "ID_FS_LABEL" => "A_B\"C",
            "ID_FS_LABEL_ENC" => "A\\x20B\\x22C",
            "ID_FS_UUID" => "53bf05d8-337b-41d0-96a2-5fd84300565f",
            "ID_FS_UUID_ENC" => "53bf05d8-337b-41d0-96a2-5fd84300565f",
            "ID_FS_VERSION" => "1.0",
            "ID_FS_BLOCKSIZE" => "1024",
            "ID_FS_TYPE" => "ext4",
            "ID_FS_USAGE" => "filesystem",
            "ID_PART_ENTRY_SCHEME" => "gpt",
            "ID_PART_ENTRY_NAME" => "Linux\\x20filesystem",
            "ID_PART_ENTRY_UUID" => "c08f2d9c-bbbe-432f-be6a-321066958799",
            "ID_PART_ENTRY_TYPE" => "0fc63daf-8483-4772-8e79-3d69d8477de4",
            "ID_PART_ENTRY_NUMBER" => "2",
            "ID_PART_ENTRY_OFFSET" => "43008",
            "ID_PART_ENTRY_SIZE" => "88031",
            "ID_PART_ENTRY_DISK" => "7:0",
        ]);
        $this->assertEquals([
            "type" => "ext4",
            "uuid" => "53bf05d8-337b-41d0-96a2-5fd84300565f",
            "label" => "A B\"C",
            "version" => "1.0",
            "usage" => "filesystem",
            "part_entry_scheme" => "gpt",
            "part_entry_uuid" => "c08f2d9c-bbbe-432f-be6a-321066958799",
            "part_entry_type" => "0fc63daf-8483-4772-8e79-3d69d8477de4",
            "part_entry_name" => "Linux filesystem",
            "part_entry_number" => "2",
            "part_entry_offset" => "43008",
            "part_entry_size" => "88031",
            "part_entry_disk" => "7:0",
            "partuuid" => "c08f2d9c-bbbe-432f-be6a-321066958799",
            "partlabel" => "Linux filesystem",
        ], $data);
        // Partitions inherit the partition table properties of the
        // parent device, they must not be reported.
        $this->assertArrayNotHasKey("pttype", $data);
        $this->assertArrayNotHasKey("ptuuid", $data);
    }

    public function test_btrfs_multi_device()
    {
        $data = \OMV\System\Filesystem\Blkid::fromUdevProperties([
            "DEVNAME" => "/dev/vdd",
            "DEVTYPE" => "disk",
            "ID_FS_UUID" => "86026b6c-6dfd-4dad-bc2b-1acc2b8fe47a",
            "ID_FS_UUID_ENC" => "86026b6c-6dfd-4dad-bc2b-1acc2b8fe47a",
            "ID_FS_UUID_SUB" => "67b82ab8-e661-4ba6-b9d8-b675664cb27f",
            "ID_FS_UUID_SUB_ENC" => "67b82ab8-e661-4ba6-b9d8-b675664cb27f",
            "ID_FS_TYPE" => "btrfs",
            "ID_FS_USAGE" => "filesystem",
        ]);
        $this->assertEquals([
            "type" => "btrfs",
            "uuid" => "86026b6c-6dfd-4dad-bc2b-1acc2b8fe47a",
            "uuid_sub" => "67b82ab8-e661-4ba6-b9d8-b675664cb27f",
            "usage" => "filesystem",
        ], $data);
    }

    public function test_disk_with_partition_table()
    {
        $data = \OMV\System\Filesystem\Blkid::fromUdevProperties([
            "DEVNAME" => "/dev/sda",
            "DEVTYPE" => "disk",
            "ID_PART_TABLE_UUID" => "f10e0785-5614-4ffb-b38e-379bb3d3aa1c",
            "ID_PART_TABLE_TYPE" => "gpt",
        ]);
        $this->assertEquals([
            "pttype" => "gpt",
            "ptuuid" => "f10e0785-5614-4ffb-b38e-379bb3d3aa1c",
        ], $data);
    }

    public function test_empty_device()
    {
        $data = \OMV\System\Filesystem\Blkid::fromUdevProperties([
            "DEVNAME" => "/dev/loop2",
            "DEVTYPE" => "disk",
            "USEC_INITIALIZED" => "203712857",
        ]);
        $this->assertEquals([], $data);
    }

    public function test_empty_partition()
    {
        // An empty partition inherits the partition table properties of
        // its parent device and has a partition entry, but it does not
        // contain a signature itself, i.e. neither a file system type nor
        // a partition table is reported.
        $data = \OMV\System\Filesystem\Blkid::fromUdevProperties([
            "DEVNAME" => "/dev/loop0p1",
            "DEVTYPE" => "partition",
            "ID_PART_TABLE_UUID" => "d62fb27f-d86d-427f-8d2f-7fb484cb5c80",
            "ID_PART_TABLE_TYPE" => "gpt",
            "ID_PART_ENTRY_SCHEME" => "gpt",
            "ID_PART_ENTRY_UUID" => "448b0167-c250-49f9-966c-d4f1a4212181",
            "ID_PART_ENTRY_TYPE" => "0fc63daf-8483-4772-8e79-3d69d8477de4",
            "ID_PART_ENTRY_NUMBER" => "1",
            "ID_PART_ENTRY_OFFSET" => "2048",
            "ID_PART_ENTRY_SIZE" => "40960",
            "ID_PART_ENTRY_DISK" => "7:0",
        ]);
        $this->assertNotEmpty($data);
        $this->assertArrayNotHasKey("type", $data);
        $this->assertArrayNotHasKey("pttype", $data);
        $this->assertArrayNotHasKey("ptuuid", $data);
    }
}
