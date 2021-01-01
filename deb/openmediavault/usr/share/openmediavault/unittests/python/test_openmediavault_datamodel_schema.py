# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2021 Volker Theile
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
# along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
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
            ),
        )

    def test_check_format_sharename(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("data", {"format": "sharename"}, "field1")

    def test_check_format_sharename_fail(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                ".foo", {"format": "sharename"}, "field1"
            ),
        )

    def test_check_format_domainname(self):
        schema = openmediavault.datamodel.Schema({})
        schema._check_format("test.com", {"format": "domainname"}, "field1")

    def test_check_format_domainname_fail(self):
        schema = openmediavault.datamodel.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "te:t#com", {"format": "domainname"}, "field1"
            ),
        )


if __name__ == "__main__":
    unittest.main()
