# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2025 Volker Theile
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

import openmediavault.stringutils


class StringTestCase(unittest.TestCase):
    def test_is_json(self):
        valid = openmediavault.stringutils.is_json('{"a": 10}')
        self.assertTrue(valid)

    def test_is_json_fail(self):
        valid = openmediavault.stringutils.is_json("abc")
        self.assertFalse(valid)

    def test_is_uuid4(self):
        valid = openmediavault.stringutils.is_uuid4(
            "e063327c-e4a4-11e6-8039-cf7ee3ff4874"
        )
        self.assertTrue(valid)

    def test_is_uuid4_fail_1(self):
        valid = openmediavault.stringutils.is_uuid4("abc")
        self.assertFalse(valid)

    def test_is_uuid4_fail_2(self):
        valid = openmediavault.stringutils.is_uuid4(None)
        self.assertFalse(valid)

    def test_is_uuid4_fail_3(self):
        valid = openmediavault.stringutils.is_uuid4("")
        self.assertFalse(valid)

    def test_is_uuid4_fail_4(self):
        valid = openmediavault.stringutils.is_uuid4(1234)
        self.assertFalse(valid)

    def test_is_fs_uuid_ext4(self):
        valid = openmediavault.stringutils.is_fs_uuid(
            "f5fce952-e4a4-11e6-accf-175f1e865654"
        )
        self.assertTrue(valid)

    def test_is_fs_uuid_fat(self):
        valid = openmediavault.stringutils.is_fs_uuid("7A48-BA97")
        self.assertTrue(valid)

    def test_is_fs_uuid_ntfs(self):
        valid = openmediavault.stringutils.is_fs_uuid("2ED43920D438EC29")
        self.assertTrue(valid)

    def test_is_fs_uuid_iso9660(self):
        valid = openmediavault.stringutils.is_fs_uuid("2015-01-13-21-48-46-00")
        self.assertTrue(valid)

    def test_is_fs_uuid_fail_1(self):
        valid = openmediavault.stringutils.is_fs_uuid("xyz")
        self.assertFalse(valid)

    def test_is_fs_uuid_fail_2(self):
        valid = openmediavault.stringutils.is_fs_uuid(None)
        self.assertFalse(valid)

    def test_is_fs_uuid_fail_3(self):
        valid = openmediavault.stringutils.is_fs_uuid("")
        self.assertFalse(valid)

    def test_is_fs_uuid_fail_4(self):
        valid = openmediavault.stringutils.is_fs_uuid(1234)
        self.assertFalse(valid)

    def test_escape_blank(self):
        self.assertEqual(
            openmediavault.stringutils.escape_blank('foo bar  xyz'),
            'foo\\x20bar\\x20\\x20xyz',
        )

    def test_escape_blank_octal(self):
        self.assertEqual(
            openmediavault.stringutils.escape_blank('foo bar  xyz', True),
            'foo\\040bar\\040\\040xyz',
        )

    def test_unescape_blank(self):
        self.assertEqual(
            openmediavault.stringutils.unescape_blank(
                'foo\\x20bar\\x20\\x20xyz'),
            'foo bar  xyz',
        )

    def test_unescape_blank_octal(self):
        self.assertEqual(
            openmediavault.stringutils.unescape_blank(
                'foo\\040bar\\040\\040xyz', True
            ),
            'foo bar  xyz',
        )

    def test_binary_format_1(self):
        result = openmediavault.stringutils.binary_format(1073741824)
        self.assertIsInstance(result, str)
        self.assertEqual(result, '1.00 GiB')

    def test_binary_format_2(self):
        result = openmediavault.stringutils.binary_format('2048')
        self.assertIsInstance(result, str)
        self.assertEqual(result, '2.00 KiB')

    def test_binary_format_3(self):
        result = openmediavault.stringutils.binary_format(4096, precision=0)
        self.assertIsInstance(result, str)
        self.assertEqual(result, '4 KiB')

    def test_binary_format_4(self):
        result = openmediavault.stringutils.binary_format(
            2048, return_json=True)
        self.assertIsInstance(result, dict)
        self.assertEqual(result['value'], 2.0)
        self.assertEqual(result['unit'], 'KiB')

    def test_binary_format_5(self):
        result = openmediavault.stringutils.binary_format(
            1073741824, max_unit='MiB')
        self.assertIsInstance(result, str)
        self.assertEqual(result, '1024.00 MiB')

    def test_binary_format_6(self):
        result = openmediavault.stringutils.binary_format(
            2048, precision=0, origin_unit='KiB', max_unit='MiB'
        )
        self.assertIsInstance(result, str)
        self.assertEqual(result, '2 MiB')

    def test_path_prettify_1(self):
        self.assertEqual(
            openmediavault.stringutils.path_prettify('/a/b/c'), '/a/b/c/'
        )

    def test_path_prettify_2(self):
        self.assertEqual(
            openmediavault.stringutils.path_prettify('x/y///'), 'x/y/')

    def test_add_slashes(self):
        self.assertEqual(openmediavault.stringutils.add_slashes("'"), "\\'")
        self.assertEqual(openmediavault.stringutils.add_slashes('"'), '\\"')
        self.assertEqual(openmediavault.stringutils.add_slashes('\\'), '\\\\')
        self.assertEqual(openmediavault.stringutils.add_slashes('$'), '\\$')
        self.assertEqual(openmediavault.stringutils.add_slashes('`'), '\\`')

    def test_yesno_1(self):
        self.assertEqual(openmediavault.stringutils.yesno(True), 'yes')

    def test_yesno_2(self):
        self.assertEqual(openmediavault.stringutils.yesno(False), 'no')

    def test_yesno_3(self):
        self.assertEqual(openmediavault.stringutils.yesno(
            True, 'foo,bar,baz'), 'foo')


if __name__ == "__main__":
    unittest.main()
