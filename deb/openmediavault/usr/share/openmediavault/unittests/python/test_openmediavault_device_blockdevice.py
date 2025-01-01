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
import openmediavault.device
import pyudev


class MockedPyUdevDevice:
    def __init__(self):
        self._properties = {}
        self._links = {}

    def set_properties(self, properties):
        self._properties = properties

    @property
    def properties(self):
        return self._properties

    def set_links(self, links):
        self._links = links

    @property
    def device_links(self):
        return self._links

    @property
    def device_number(self):
        return 2065


class BlockDeviceTestCase(unittest.TestCase):
    def test_device_file(self):
        bd = openmediavault.device.BlockDevice('/dev/sda')
        self.assertIsInstance(bd.device_file, str)
        self.assertEqual(bd.device_file, '/dev/sda')

    @mock.patch('os.path.realpath', return_value='/dev/sdd1')
    def test_canonical_device_file(self, _):
        bd = openmediavault.device.BlockDevice(
            '/dev/disk/by-uuid/4B04EA317E4AA567'
        )
        self.assertEqual(bd.canonical_device_file, '/dev/sdd1')

    def test_device_name(self):
        bd = openmediavault.device.BlockDevice('/dev/sda')
        self.assertEqual(bd.device_name(), 'sda')

    @mock.patch('os.path.realpath', return_value='/dev/sdd1')
    def test_device_name_canonical(self, _):
        bd = openmediavault.device.BlockDevice(
            '/dev/disk/by-uuid/4B04EA317E4AA567'
        )
        self.assertEqual(bd.device_name(True), 'sdd1')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_udev_properties(self, mock_from_device_file):
        device = MockedPyUdevDevice()
        device.set_properties(
            {'DEVTYPE': 'disk', 'ID_ATA': '1', 'ID_ATA_FEATURE_SET_HPA': '1'}
        )
        mock_from_device_file.return_value = device
        bd = openmediavault.device.BlockDevice('/dev/sda')
        self.assertIsInstance(bd.udev_properties, dict)

    @mock.patch('pyudev.Devices.from_device_file')
    def test_has_udev_property_exists(self, mock_from_device_file):
        device = MockedPyUdevDevice()
        device.set_properties({'DEVNAME': '/dev/sda', 'ID_ATA': '1'})
        mock_from_device_file.return_value = device
        bd = openmediavault.device.BlockDevice('/dev/sda')
        self.assertTrue(bd.has_udev_property('DEVNAME'))

    @mock.patch('pyudev.Devices.from_device_file')
    def test_has_udev_property_not_exists(self, mock_from_device_file):
        device = MockedPyUdevDevice()
        device.set_properties({'DEVNAME': '/dev/sda', 'ID_ATA': '1'})
        mock_from_device_file.return_value = device
        bd = openmediavault.device.BlockDevice('/dev/sda')
        self.assertFalse(bd.has_udev_property('FOO'))

    @mock.patch('pyudev.Devices.from_device_file')
    def test_udev_property(self, mock_from_device_file):
        device = MockedPyUdevDevice()
        device.set_properties(
            {'DEVTYPE': 'disk', 'ID_ATA_FEATURE_SET_HPA': '1'}
        )
        mock_from_device_file.return_value = device
        bd = openmediavault.device.BlockDevice('/dev/sda')
        self.assertEqual(bd.udev_property('DEVTYPE'), 'disk')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_udev_property_default(self, mock_from_device_file):
        device = MockedPyUdevDevice()
        device.set_properties({'DEVTYPE': 'disk'})
        mock_from_device_file.return_value = device
        bd = openmediavault.device.BlockDevice('/dev/sda')
        self.assertEqual(bd.udev_property('ID_FS_LABEL', 'foo'), 'foo')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_udev_property_exception_1(self, mock_from_device_file):
        device = MockedPyUdevDevice()
        device.set_properties({'DEVTYPE': 'disk'})
        mock_from_device_file.return_value = device
        bd = openmediavault.device.BlockDevice('/dev/sda')
        self.assertRaises(KeyError, lambda: bd.udev_property('ID_FS_LABEL'))

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_udev_property_exception_2(self, mock_from_device_file):
        mock_from_device_file.side_effect = pyudev.DeviceNotFoundByFileError
        bd = openmediavault.device.BlockDevice('/dev/vda')
        self.assertRaises(
            pyudev.DeviceNotFoundByFileError,
            lambda: bd.udev_property('ID_FS_LABEL'),
        )

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_device_files(self, mock_from_device_file):
        device = MockedPyUdevDevice()
        device.set_links(['/dev/disk/by-id/foo', '/dev/bar'])
        mock_from_device_file.return_value = device
        bd = openmediavault.device.BlockDevice('/dev/sda')
        dev_links = bd.device_links
        self.assertIsInstance(dev_links, list)
        self.assertEqual(dev_links[0], '/dev/disk/by-id/foo')
        self.assertEqual(dev_links[1], '/dev/bar')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_device_file_by_id(self, mock_from_device_file):
        device = MockedPyUdevDevice()
        device.set_links(['/dev/disk/by-id/foo'])
        mock_from_device_file.return_value = device
        bd = openmediavault.device.BlockDevice('/dev/sda')
        self.assertTrue(bd.has_device_file_by_id())
        self.assertFalse(bd.has_device_file_by_path())
        self.assertEqual(bd.device_file_by_id, '/dev/disk/by-id/foo')
        self.assertIsNone(bd.device_file_by_path)

    @mock.patch('pyudev.Devices.from_device_file')
    def test_device_file_by_path(self, mock_from_device_file):
        device = MockedPyUdevDevice()
        device.set_links(['/dev/disk/by-path/xyz'])
        mock_from_device_file.return_value = device
        bd = openmediavault.device.BlockDevice('/dev/sda')
        self.assertFalse(bd.has_device_file_by_id())
        self.assertTrue(bd.has_device_file_by_path())
        self.assertIsNone(bd.device_file_by_id)
        self.assertEqual(bd.device_file_by_path, '/dev/disk/by-path/xyz')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_predictable_device_file_1(self, mock_from_device_file):
        device = MockedPyUdevDevice()
        device.set_links(['/dev/disk/by-path/xyz'])
        mock_from_device_file.return_value = device
        bd = openmediavault.device.BlockDevice('/dev/sda')
        self.assertEqual(bd.predictable_device_file, '/dev/disk/by-path/xyz')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_predictable_device_file_2(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice()
        bd = openmediavault.device.BlockDevice('/dev/sda')
        self.assertEqual(bd.predictable_device_file, '/dev/sda')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_device_number(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice()
        bd = openmediavault.device.BlockDevice('/dev/sda')
        self.assertIsInstance(bd.device_number, int)
        self.assertEqual(bd.device_number, 2065)
        self.assertIsInstance(bd.major_device_number, int)
        self.assertEqual(bd.major_device_number, 8)
        self.assertIsInstance(bd.minor_device_number, int)
        self.assertEqual(bd.minor_device_number, 17)


if __name__ == "__main__":
    unittest.main()
