# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2018 Volker Theile
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
import mock
import unittest
import openmediavault.storagedevice

from pyfakefs import fake_filesystem


class StorageDeviceTestCase(unittest.TestCase):
    fs = fake_filesystem.FakeFilesystem()
    f_open = fake_filesystem.FakeFileOpen(fs)
    f_os = fake_filesystem.FakeOsModule(fs)

    @mock.patch('builtins.open', new=f_open)
    @mock.patch('os.path.exists', new=f_os.path.exists)
    def test_model(self):
        self.fs.reset()
        self.fs.create_file(
            '/sys/block/sda/device/model', contents='''FooBar\n'''
        )
        sd = openmediavault.storagedevice.StorageDevice('/dev/sda')
        self.assertIsInstance(sd.model, str)
        self.assertEqual(sd.model, 'FooBar')

    @mock.patch(
        'openmediavault.subprocess.check_output', return_value='ID_SSD=0'
    )
    def test_is_rotational_1(self, mock_check_output):
        sd = openmediavault.storagedevice.StorageDevice('/dev/sda')
        self.assertIsInstance(sd.is_rotational, bool)
        self.assertTrue(sd.is_rotational)

    @mock.patch(
        'openmediavault.subprocess.check_output', return_value='ID_SSD=1'
    )
    def test_is_rotational_2(self, mock_check_output):
        sd = openmediavault.storagedevice.StorageDevice('/dev/sda')
        self.assertFalse(sd.is_rotational)

    @mock.patch(
        'openmediavault.subprocess.check_output',
        return_value='ID_ATA_ROTATION_RATE_RPM=0'
    )
    def test_is_rotational_3(self, mock_check_output):
        sd = openmediavault.storagedevice.StorageDevice('/dev/sda')
        self.assertFalse(sd.is_rotational)

    @mock.patch(
        'openmediavault.subprocess.check_output',
        return_value='ID_ATA_ROTATION_RATE_RPM=1000'
    )
    def test_is_rotational_4(self, mock_check_output):
        sd = openmediavault.storagedevice.StorageDevice('/dev/sda')
        self.assertTrue(sd.is_rotational)

    @mock.patch(
        'openmediavault.subprocess.check_output',
        return_value='ID_ATA_FEATURE_SET_AAM=0'
    )
    def test_is_rotational_5(self, mock_check_output):
        sd = openmediavault.storagedevice.StorageDevice('/dev/sda')
        self.assertFalse(sd.is_rotational)

    @mock.patch(
        'openmediavault.subprocess.check_output',
        return_value='ID_ATA_FEATURE_SET_AAM=1'
    )
    def test_is_rotational_6(self, mock_check_output):
        sd = openmediavault.storagedevice.StorageDevice('/dev/sda')
        self.assertTrue(sd.is_rotational)

    @mock.patch('openmediavault.subprocess.check_output', return_value='')
    @mock.patch('os.path.realpath', return_value='/dev/sda')
    @mock.patch('builtins.open', new=f_open)
    @mock.patch('os.path.exists', new=f_os.path.exists)
    def test_is_rotational_7(self, mock_realpath, mock_check_output):
        self.fs.reset()
        self.fs.create_file(
            '/sys/block/sda/queue/rotational', contents='''1\n'''
        )
        sd = openmediavault.storagedevice.StorageDevice('/dev/sda')
        self.assertTrue(sd.is_rotational)

    @mock.patch('openmediavault.subprocess.check_output', return_value='')
    @mock.patch('os.path.realpath', return_value='/dev/sda')
    @mock.patch('builtins.open', new=f_open)
    @mock.patch('os.path.exists', new=f_os.path.exists)
    def test_is_rotational_8(self, mock_realpath, mock_check_output):
        self.fs.reset()
        self.fs.create_file(
            '/sys/block/sda/queue/rotational', contents='''0\n'''
        )
        sd = openmediavault.storagedevice.StorageDevice('/dev/sda')
        self.assertFalse(sd.is_rotational)

    @mock.patch('openmediavault.subprocess.check_output', return_value='')
    @mock.patch('os.path.realpath', return_value='/dev/sda')
    @mock.patch('builtins.open', new=f_open)
    @mock.patch('os.path.exists', new=f_os.path.exists)
    def test_is_rotational_9(self, mock_realpath, mock_check_output):
        self.fs.reset()
        self.fs.create_file(
            '/sys/block/sda/device/model', contents='''I am a SSD\n'''
        )
        sd = openmediavault.storagedevice.StorageDevice('/dev/sda')
        self.assertTrue(sd.is_rotational)

    @mock.patch('openmediavault.subprocess.check_output', return_value='')
    @mock.patch('os.path.realpath', return_value='/dev/sda')
    @mock.patch('builtins.open', new=f_open)
    @mock.patch('os.path.exists', new=f_os.path.exists)
    def test_is_rotational_10(self, mock_realpath, mock_check_output):
        self.fs.reset()
        self.fs.create_file(
            '/sys/block/sda/device/model', contents='''Foo Bar\n'''
        )
        sd = openmediavault.storagedevice.StorageDevice('/dev/sda')
        self.assertFalse(sd.is_rotational)


if __name__ == "__main__":
    unittest.main()
