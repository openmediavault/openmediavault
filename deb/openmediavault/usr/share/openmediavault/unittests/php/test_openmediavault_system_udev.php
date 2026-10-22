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

class test_openmediavault_system_udev extends \PHPUnit\Framework\TestCase
{
    public function test_parse_database()
    {
        $lines = [
            "P: /devices/virtual/block/loop0",
            "M: loop0",
            "U: block",
            "T: disk",
            "N: loop0",
            "S: disk/by-diskseq/16",
            "E: DEVPATH=/devices/virtual/block/loop0",
            "E: SUBSYSTEM=block",
            "E: DEVNAME=/dev/loop0",
            "E: DEVTYPE=disk",
            "E: ID_PART_TABLE_TYPE=gpt",
            "",
            "P: /devices/virtual/net/lo",
            "M: lo",
            "E: DEVPATH=/devices/virtual/net/lo",
            "E: SUBSYSTEM=net",
            "E: INTERFACE=lo",
            "",
            "",
            "P: /devices/virtual/block/loop0/loop0p2",
            "E: SUBSYSTEM=block",
            "E: DEVNAME=/dev/loop0p2",
            "E: ID_FS_LABEL=A_B\"C",
            "E: ID_FS_LABEL_ENC=A\\x20B\\x22C",
            "E: ID_FS_VERSION=LVM2 001",
            "E: ID_FOO=a=b",
        ];
        $devices = \OMV\System\Udev::parseDatabase($lines);
        $this->assertCount(3, $devices);
        $this->assertEquals([
            "DEVPATH" => "/devices/virtual/block/loop0",
            "SUBSYSTEM" => "block",
            "DEVNAME" => "/dev/loop0",
            "DEVTYPE" => "disk",
            "ID_PART_TABLE_TYPE" => "gpt",
        ], $devices[0]);
        $this->assertEquals("net", $devices[1]['SUBSYSTEM']);
        // The last device has no trailing empty line.
        $this->assertEquals("/dev/loop0p2", $devices[2]['DEVNAME']);
        $this->assertEquals("A_B\"C", $devices[2]['ID_FS_LABEL']);
        $this->assertEquals("A\\x20B\\x22C", $devices[2]['ID_FS_LABEL_ENC']);
        $this->assertEquals("LVM2 001", $devices[2]['ID_FS_VERSION']);
        $this->assertEquals("a=b", $devices[2]['ID_FOO']);
    }

    public function test_parse_database_empty()
    {
        $this->assertEquals([], \OMV\System\Udev::parseDatabase([]));
        $this->assertEquals([], \OMV\System\Udev::parseDatabase(["", ""]));
    }

    public function test_parse_properties()
    {
        $properties = \OMV\System\Udev::parseProperties([
            "DEVNAME=/dev/vdb",
            "ID_FS_VERSION=LVM2 001",
            "ID_FS_TYPE=LVM2_member",
            "EMPTY=",
            "invalid line",
        ]);
        $this->assertEquals([
            "DEVNAME" => "/dev/vdb",
            "ID_FS_VERSION" => "LVM2 001",
            "ID_FS_TYPE" => "LVM2_member",
            "EMPTY" => "",
        ], $properties);
    }

    public function test_decode()
    {
        $this->assertEquals("My Data", \OMV\System\Udev::decode("My\\x20Data"));
        $this->assertEquals("A B\"C", \OMV\System\Udev::decode("A\\x20B\\x22C"));
        $this->assertEquals("/root/t1.img", \OMV\System\Udev::decode("\\x2froot\\x2ft1.img"));
        $this->assertEquals("Müller", \OMV\System\Udev::decode("Müller"));
        $this->assertEquals("plain", \OMV\System\Udev::decode("plain"));
        $this->assertEquals("", \OMV\System\Udev::decode(""));
    }

    public function test_decode_single_pass()
    {
        // An encoded backslash must not be decoded again.
        $this->assertEquals("a\\x41", \OMV\System\Udev::decode("a\\x5cx41"));
    }
}
