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
import openmediavault.blockdevice


class BlockDeviceTestCase(unittest.TestCase):
    udevadm_output = '\n'.join([
        'DEVLINKS=/dev/disk/by-id/ata-IBM-DTTA-341050_WF0WABH1579 disk/by-id/scsi-SATA_IBM-DTTA-341050_WF0WABH1579 /dev/disk/by-path/pci-0000:00:02.5-scsi-0:0:0:0',
        'DEVNAME=/dev/sda',
        'DEVPATH=/devices/pci0000:00/0000:00:02.5/host0/target0:0:0/0:0:0:0/block/sda',
        'DEVTYPE=disk', 'ID_ATA=1', 'ID_ATA_FEATURE_SET_HPA=1'
    ]).encode()

    def test_is_device_file(self):
        self.assertFalse(openmediavault.blockdevice.is_device_file('/foo/bar'))
        self.assertTrue(openmediavault.blockdevice.is_device_file('/dev/sda'))

    def test_is_device_file_by(self):
        self.assertFalse(
            openmediavault.blockdevice.is_device_file_by('/dev/sda')
        )
        self.assertTrue(
            openmediavault.blockdevice.is_device_file_by(
                '/dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895'
            )
        )
        self.assertTrue(
            openmediavault.blockdevice.is_device_file_by(
                '/dev/disk/by-id/usb-Kingston_DataTraveler_G2_001CC0EC21ADF011C6A20E35-0:0-part1'
            )
        )
        self.assertTrue(
            openmediavault.blockdevice.
            is_device_file_by('/dev/disk/by-label/data')
        )

    def test_is_device_file_by_uuid(self):
        self.assertFalse(
            openmediavault.blockdevice.is_device_file_by_uuid('/dev/sda')
        )
        self.assertTrue(
            openmediavault.blockdevice.is_device_file_by_uuid(
                '/dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895'
            )
        )

    def test_is_device_file_by_id(self):
        self.assertFalse(
            openmediavault.blockdevice.is_device_file_by_id('/dev/sda')
        )
        self.assertTrue(
            openmediavault.blockdevice.is_device_file_by_id(
                '/dev/disk/by-id/wwn-0x4002c554a4d79cb9-part2'
            )
        )

    def test_is_device_file_by_label(self):
        self.assertFalse(
            openmediavault.blockdevice.is_device_file_by_label('/dev/sda')
        )
        self.assertTrue(
            openmediavault.blockdevice.
            is_device_file_by_label('/dev/disk/by-label/data')
        )

    def test_is_device_file_by_path(self):
        self.assertFalse(
            openmediavault.blockdevice.is_device_file_by_path('/dev/sda')
        )
        self.assertTrue(
            openmediavault.blockdevice.
            is_device_file_by_path('/dev/disk/by-path/pci-0000:00:17.0-ata-3')
        )

    def test_device_file(self):
        bd = openmediavault.blockdevice.BlockDevice('/dev/sda')
        self.assertIsInstance(bd.device_file, str)
        self.assertEqual(bd.device_file, '/dev/sda')

    @mock.patch('os.path.realpath', return_value='/dev/sdd1')
    def test_canonical_device_file(self, mock_realpath):
        bd = openmediavault.blockdevice.BlockDevice(
            '/dev/disk/by-uuid/4B04EA317E4AA567'
        )
        self.assertEqual(bd.canonical_device_file, '/dev/sdd1')

    def test_device_name(self):
        bd = openmediavault.blockdevice.BlockDevice('/dev/sda')
        self.assertEqual(bd.device_name(), 'sda')

    @mock.patch('os.path.realpath', return_value='/dev/sdd1')
    def test_device_name_canonical(self, mock_realpath):
        bd = openmediavault.blockdevice.BlockDevice(
            '/dev/disk/by-uuid/4B04EA317E4AA567'
        )
        self.assertEqual(bd.device_name(True), 'sdd1')

    @mock.patch(
        'openmediavault.subprocess.check_output', return_value=udevadm_output
    )
    def test_query_udev_properties(self, mock_check_output):
        bd = openmediavault.blockdevice.BlockDevice('/dev/sda')
        self.assertTrue(bd.query_udev_properties())
        self.assertFalse(bd.query_udev_properties())
        self.assertTrue(bd.query_udev_properties(force=True))

    @mock.patch(
        'openmediavault.subprocess.check_output', return_value=udevadm_output
    )
    def test_udev_properties(self, mock_check_output):
        bd = openmediavault.blockdevice.BlockDevice('/dev/sda')
        self.assertIsInstance(bd.udev_properties, dict)

    @mock.patch(
        'openmediavault.subprocess.check_output', return_value=udevadm_output
    )
    def test_has_udev_property_exists(self, mock_check_output):
        bd = openmediavault.blockdevice.BlockDevice('/dev/sda')
        self.assertTrue(bd.has_udev_property('DEVNAME'))

    @mock.patch(
        'openmediavault.subprocess.check_output', return_value=udevadm_output
    )
    def test_has_udev_property_not_exists(self, mock_check_output):
        bd = openmediavault.blockdevice.BlockDevice('/dev/sda')
        self.assertFalse(bd.has_udev_property('FOO'))

    @mock.patch(
        'openmediavault.subprocess.check_output', return_value=udevadm_output
    )
    def test_get_udev_property(self, mock_check_output):
        bd = openmediavault.blockdevice.BlockDevice('/dev/sda')
        self.assertEqual(bd.get_udev_property('DEVTYPE'), 'disk')

    @mock.patch(
        'openmediavault.subprocess.check_output', return_value=udevadm_output
    )
    def test_get_device_file_symlinks(self, mock_check_output):
        bd = openmediavault.blockdevice.BlockDevice('/dev/sda')
        dev_links = bd.get_device_file_symlinks()
        self.assertIsInstance(dev_links, list)
        self.assertEqual(
            dev_links[0], '/dev/disk/by-id/ata-IBM-DTTA-341050_WF0WABH1579'
        )
        self.assertEqual(
            dev_links[1],
            '/dev/disk/by-id/scsi-SATA_IBM-DTTA-341050_WF0WABH1579'
        )
        self.assertEqual(
            dev_links[2], '/dev/disk/by-path/pci-0000:00:02.5-scsi-0:0:0:0'
        )


if __name__ == "__main__":
    unittest.main()
