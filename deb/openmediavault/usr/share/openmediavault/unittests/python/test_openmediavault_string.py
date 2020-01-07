# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2020 Volker Theile
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
import openmediavault.string


class StringTestCase(unittest.TestCase):
    def test_is_json(self):
        valid = openmediavault.string.is_json('{"a": 10}')
        self.assertTrue(valid)

    def test_is_json_fail(self):
        valid = openmediavault.string.is_json("abc")
        self.assertFalse(valid)

    def test_is_uuid4(self):
        valid = openmediavault.string.is_uuid4(
            "e063327c-e4a4-11e6-8039-cf7ee3ff4874"
        )
        self.assertTrue(valid)

    def test_is_uuid4_fail(self):
        valid = openmediavault.string.is_uuid4("abc")
        self.assertFalse(valid)

    def test_is_fs_uuid_ext4(self):
        valid = openmediavault.string.is_fs_uuid(
            "f5fce952-e4a4-11e6-accf-175f1e865654"
        )
        self.assertTrue(valid)

    def test_is_fs_uuid_fat(self):
        valid = openmediavault.string.is_fs_uuid("7A48-BA97")
        self.assertTrue(valid)

    def test_is_fs_uuid_ntfs(self):
        valid = openmediavault.string.is_fs_uuid("2ED43920D438EC29")
        self.assertTrue(valid)

    def test_is_fs_uuid_iso9660(self):
        valid = openmediavault.string.is_fs_uuid("2015-01-13-21-48-46-00")
        self.assertTrue(valid)

    def test_is_fs_uuid_fail(self):
        valid = openmediavault.string.is_fs_uuid("xyz")
        self.assertFalse(valid)


if __name__ == "__main__":
    unittest.main()
