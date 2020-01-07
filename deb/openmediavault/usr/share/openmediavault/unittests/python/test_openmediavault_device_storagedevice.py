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
import mock
import unittest
import openmediavault.device

from pyfakefs import fake_filesystem


class MockedPyUdevDevice:
    def __init__(self, properties):
        self._properties = properties

    @property
    def properties(self):
        return self._properties


class StorageDeviceTestCase(unittest.TestCase):
    fs = fake_filesystem.FakeFilesystem()
    f_open = fake_filesystem.FakeFileOpen(fs)
    f_os = fake_filesystem.FakeOsModule(fs)

    def setUp(self):
        self.fs.reset()

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_model_1(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_MODEL_ENC': 'Foo\\x20Bar'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertIsInstance(sd.get_model(), str)
        self.assertEqual(sd.get_model(), 'Foo Bar')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_model_2(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice({})
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertIsInstance(sd.get_model(), str)
        self.assertEqual(sd.get_model(), '')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_vendor_1(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_VENDOR_ENC': 'Foo\\x20Bar\\x20\\x20'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertIsInstance(sd.get_vendor(), str)
        self.assertEqual(sd.get_vendor(), 'Foo Bar  ')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_vendor_2(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice({})
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertIsInstance(sd.get_vendor(), str)
        self.assertEqual(sd.get_vendor(), '')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_serial_1(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_SERIAL_SHORT': 'Foo_Bar'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertIsInstance(sd.get_serial(), str)
        self.assertEqual(sd.get_serial(), 'Foo Bar')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_serial_2(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice({})
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertIsInstance(sd.get_serial(), str)
        self.assertEqual(sd.get_serial(), '')

    @mock.patch('builtins.open', new=f_open)
    @mock.patch('os.path.exists', new=f_os.path.exists)
    def test_is_removable_1(self):
        self.fs.create_file('/sys/block/sdx/removable', contents='''1\n''')
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertTrue(sd.is_removable())

    @mock.patch('builtins.open', new=f_open)
    @mock.patch('os.path.exists', new=f_os.path.exists)
    def test_is_removable_2(self):
        self.fs.create_file('/sys/block/sdx/removable', contents='''FooBar\n''')
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertFalse(sd.is_removable())

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_1(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice({'ID_SSD': '0'})
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        is_rotational = sd.is_rotational()
        self.assertIsInstance(is_rotational, bool)
        self.assertTrue(is_rotational)

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_2(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice({'ID_SSD': '1'})
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertFalse(sd.is_rotational())

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_3(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_ATA_ROTATION_RATE_RPM': '0'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertFalse(sd.is_rotational())

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_4(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_ATA_ROTATION_RATE_RPM': '1000'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertTrue(sd.is_rotational())

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_5(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_ATA_FEATURE_SET_AAM': '0'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertFalse(sd.is_rotational())

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_6(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_ATA_FEATURE_SET_AAM': '1'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertTrue(sd.is_rotational())

    @mock.patch('pyudev.Devices.from_device_file')
    @mock.patch('os.path.realpath', return_value='/dev/sdx')
    @mock.patch('builtins.open', new=f_open)
    @mock.patch('os.path.exists', new=f_os.path.exists)
    def test_is_rotational_7(self, _, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice({})
        self.fs.create_file(
            '/sys/block/sdx/queue/rotational', contents='''1\n'''
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertTrue(sd.is_rotational())

    @mock.patch('pyudev.Devices.from_device_file')
    @mock.patch('os.path.realpath', return_value='/dev/sdx')
    @mock.patch('builtins.open', new=f_open)
    @mock.patch('os.path.exists', new=f_os.path.exists)
    def test_is_rotational_8(self, _, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice({})
        self.fs.create_file(
            '/sys/block/sdx/queue/rotational', contents='''0\n'''
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertFalse(sd.is_rotational())

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_9(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_MODEL_ENC': 'I\\x20am\\x20a\\x20SSD'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertFalse(sd.is_rotational())

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_10(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_MODEL_ENC': 'Foo\\x20Bar'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertTrue(sd.is_rotational())


if __name__ == "__main__":
    unittest.main()
