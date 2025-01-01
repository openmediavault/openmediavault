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

import mock
import openmediavault.confdbadm


class ConfDbAdmTestCase(unittest.TestCase):
    def setUp(self):
        self.command_helper = openmediavault.confdbadm.CommandHelper()

    def test_has_methods(self):
        self.assertTrue(hasattr(self.command_helper, 'create_backup'))
        self.assertTrue(hasattr(self.command_helper, 'unlink_backup'))
        self.assertTrue(hasattr(self.command_helper, 'rollback_changes'))

    def test_create_backup_fail(self):
        openmediavault.setenv("OMV_CONFIG_FILE", "xyz.conf")
        self.assertIsNone(self.command_helper.create_backup())
        self.assertFalse(self.command_helper._backup_path)

    @mock.patch("os.path.exists", return_value=True)
    @mock.patch("shutil.copy")
    def test_create_backup(self, mock_copy, mock_exists):
        openmediavault.setenv("OMV_CONFIG_FILE", "abc.conf")
        result = self.command_helper.create_backup()
        self.assertIsNotNone(result)
        self.assertIsNotNone(self.command_helper._backup_path)
        self.assertEqual(result, self.command_helper._backup_path)
        mock_copy.assert_called_with("abc.conf", result)

    def test_unlink_backup_fail(self):
        with self.assertRaises(Exception) as ctx:
            self.command_helper.unlink_backup()
            self.assertEqual(
                str(ctx.exception), "No configuration backup exists."
            )

    @mock.patch("os.unlink")
    def test_unlink_backup_fail_2(self, mock_unlink):
        self.command_helper._backup_path = False
        self.command_helper.unlink_backup()
        mock_unlink.assert_not_called()

    @mock.patch("os.unlink")
    def test_unlink_backup(self, mock_unlink):
        self.command_helper._backup_path = "foo.conf"
        self.command_helper.unlink_backup()
        mock_unlink.assert_called_once()
        self.assertIsNone(self.command_helper._backup_path)

    def test_rollback_changes_fail(self):
        with self.assertRaises(Exception) as ctx:
            self.command_helper.rollback_changes()
            self.assertEqual(
                str(ctx.exception), "No configuration backup exists."
            )

    @mock.patch("shutil.copy")
    def test_rollback_changes_fail_2(self, mock_copy):
        self.command_helper._backup_path = False
        self.command_helper.rollback_changes()
        mock_copy.assert_not_called()

    @mock.patch("shutil.copy")
    def test_rollback_changes(self, mock_copy):
        openmediavault.setenv("OMV_CONFIG_FILE", "xyz.conf")
        self.command_helper._backup_path = "bar.conf"
        self.command_helper.rollback_changes()
        mock_copy.assert_called_with("bar.conf", "xyz.conf")

    def test_argparse_is_uuid4(self):
        uuid = "e4a987ee-dfd1-11e8-bae1-2321e792b9bd"
        self.assertEqual(uuid, self.command_helper.argparse_is_uuid4(uuid))

    def test_argparse_is_uuid4_fail(self):
        with self.assertRaises(Exception) as ctx:
            self.command_helper.argparse_is_uuid4("xyz")
            self.assertEqual(str(ctx.exception), "No valid UUID4.")

    def test_argparse_is_json(self):
        self.assertDictEqual(
            {"a": 0}, self.command_helper.argparse_is_json('{"a": 0}')
        )

    def test_argparse_is_json_fail(self):
        with self.assertRaises(Exception) as ctx:
            self.command_helper.argparse_is_json("abc")
            self.assertEqual(str(ctx.exception), "No valid JSON.")

    @mock.patch("sys.stdin.read", return_value='{"foo": "bar"}')
    def test_argparse_is_json_stdin(self, mock_read):
        self.assertDictEqual(
            {"foo": "bar"}, self.command_helper.argparse_is_json_stdin("-")
        )

    def test_argparse_is_datamodel_id_fail(self):
        with self.assertRaises(Exception) as ctx:
            self.command_helper.argparse_is_datamodel_id("xyz")
            self.assertEqual(str(ctx.exception), "No valid data model ID.")

    def test_argparse_is_datamodel_id_1(self):
        self.assertEqual(
            self.command_helper.argparse_is_datamodel_id("conf"), "conf"
        )

    @mock.patch("openmediavault.config.Datamodel")
    def test_argparse_is_datamodel_id_2(self, mock_datamodel):
        self.assertEqual(
            self.command_helper.argparse_is_datamodel_id("conf.service.ssh"),
            "conf.service.ssh",
        )


if __name__ == "__main__":
    unittest.main()
