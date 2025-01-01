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
        self.assertIsInstance(sd.model, str)
        self.assertEqual(sd.model, 'Foo Bar')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_model_2(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice({})
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertIsInstance(sd.model, str)
        self.assertEqual(sd.model, '')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_vendor_1(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_VENDOR_ENC': 'Foo\\x20Bar\\x20\\x20'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertIsInstance(sd.vendor, str)
        self.assertEqual(sd.vendor, 'Foo Bar  ')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_vendor_2(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice({})
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertIsInstance(sd.vendor, str)
        self.assertEqual(sd.vendor, '')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_serial_1(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_SERIAL_SHORT': 'Foo_Bar'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertIsInstance(sd.serial, str)
        self.assertEqual(sd.serial, 'Foo Bar')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_get_serial_2(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice({})
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertIsInstance(sd.serial, str)
        self.assertEqual(sd.serial, '')

    @mock.patch('builtins.open', new=f_open)
    @mock.patch('os.path.exists', new=f_os.path.exists)
    def test_is_removable_1(self):
        self.fs.create_file('/sys/block/sdx/removable', contents='''1\n''')
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertTrue(sd.is_removable)

    @mock.patch('builtins.open', new=f_open)
    @mock.patch('os.path.exists', new=f_os.path.exists)
    def test_is_removable_2(self):
        self.fs.create_file('/sys/block/sdx/removable',
                            contents='''FooBar\n''')
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertFalse(sd.is_removable)

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_1(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice({
                                                                'ID_SSD': '0'})
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        is_rotational = sd.is_rotational
        self.assertIsInstance(is_rotational, bool)
        self.assertTrue(is_rotational)

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_2(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice({
                                                                'ID_SSD': '1'})
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertFalse(sd.is_rotational)

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_3(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_ATA_ROTATION_RATE_RPM': '0'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertFalse(sd.is_rotational)

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_4(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_ATA_ROTATION_RATE_RPM': '1000'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertTrue(sd.is_rotational)

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_5(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_ATA_FEATURE_SET_AAM': '0'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertFalse(sd.is_rotational)

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_6(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_ATA_FEATURE_SET_AAM': '1'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertTrue(sd.is_rotational)

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
        self.assertTrue(sd.is_rotational)

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
        self.assertFalse(sd.is_rotational)

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_9(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_MODEL_ENC': 'I\\x20am\\x20a\\x20SSD'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertFalse(sd.is_rotational)

    @mock.patch('pyudev.Devices.from_device_file')
    def test_is_rotational_10(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_MODEL_ENC': 'Foo\\x20Bar'}
        )
        sd = openmediavault.device.StorageDevice('/dev/sdx')
        self.assertTrue(sd.is_rotational)

    def test_from_device_file_bcache(self):
        sd = openmediavault.device.StorageDevice.from_device_file(
            '/dev/bcache1'
        )
        self.assertIsInstance(
            sd, openmediavault.device.plugins.bcache.StorageDevice
        )

    def test_from_device_file_cciss_1(self):
        sd = openmediavault.device.StorageDevice.from_device_file(
            '/dev/cciss/c0d0p2'
        )
        self.assertIsInstance(
            sd, openmediavault.device.plugins.cciss.StorageDevice
        )
        self.assertEqual(sd.parent.device_file, '/dev/cciss/c0d0')
        self.assertTrue(sd.is_raid)
        self.assertEqual(sd.device_name(False), 'cciss!c0d0p2')
        self.assertEqual(sd.smart_device_type, '')

    def test_from_device_file_cciss_2(self):
        sd = openmediavault.device.StorageDevice.from_device_file(
            '/dev/cciss/c0d0'
        )
        self.assertIsInstance(
            sd, openmediavault.device.plugins.cciss.StorageDevice
        )
        self.assertEqual(sd.device_name(False), 'cciss!c0d0')
        self.assertEqual(sd.smart_device_type, 'cciss,0')

    @mock.patch('pyudev.Devices.from_device_file')
    def test_from_device_file_cdrom(self, mock_from_device_file):
        mock_from_device_file.return_value = MockedPyUdevDevice(
            {'ID_CDROM_MEDIA': '0'}
        )
        sd = openmediavault.device.StorageDevice.from_device_file('/dev/sr2')
        self.assertIsInstance(
            sd, openmediavault.device.plugins.cdrom.StorageDevice
        )
        self.assertTrue(sd.is_read_only)
        self.assertFalse(sd.is_media_available)
        self.assertEqual(sd.size(), 0)

    @mock.patch('builtins.open', new=f_open)
    @mock.patch('os.path.exists', new=f_os.path.exists)
    @mock.patch('os.path.realpath', new=f_os.path.realpath)
    def test_from_device_file_dm_1(self):
        self.fs.create_symlink('/dev/mapper/test', '/dev/dm-1')
        self.fs.create_file('/sys/block/dm-1/dm/name', contents='''foo\n''')
        sd = openmediavault.device.StorageDevice.from_device_file(
            '/dev/mapper/test'
        )
        self.assertIsInstance(
            sd, openmediavault.device.plugins.dm.StorageDevice
        )
        self.assertEqual(sd.canonical_device_file, '/dev/dm-1')
        self.assertEqual(sd.device_name(True), 'dm-1')
        self.assertEqual(sd.name, 'foo')

    @mock.patch('builtins.open', new=f_open)
    @mock.patch('os.path.exists', new=f_os.path.exists)
    def test_from_device_file_dm_2(self):
        self.fs.create_file(
            '/sys/block/dm-2/dm/uuid', contents='''CRYPT-gyhi31UL0XD8\n'''
        )
        sd = openmediavault.device.StorageDevice.from_device_file('/dev/dm-2')
        self.assertIsInstance(
            sd, openmediavault.device.plugins.dm.StorageDevice
        )
        self.assertEqual(sd.canonical_device_file, '/dev/dm-2')
        self.assertEqual(sd.device_name(False), 'dm-2')
        self.assertEqual(sd.uuid, 'CRYPT-gyhi31UL0XD8')

    @mock.patch('builtins.open', new=f_open)
    @mock.patch('os.path.exists', new=f_os.path.exists)
    def test_from_device_file_dm_3(self):
        self.fs.create_file(
            '/sys/block/dm-3/dm/uuid', contents='''LVM-EOVOgvd6Wfgpb\n'''
        )
        sd = openmediavault.device.StorageDevice.from_device_file('/dev/dm-3')
        self.assertIsInstance(
            sd, openmediavault.device.plugins.dm.StorageDeviceLVM
        )

    def test_from_device_file_fio(self):
        sd = openmediavault.device.StorageDevice.from_device_file('/dev/fioa1')
        self.assertIsInstance(
            sd, openmediavault.device.plugins.fio.StorageDevice
        )
        self.assertFalse(sd.is_rotational)
        self.assertFalse(sd.has_smart_support)

    def test_from_device_file_hd(self):
        sd = openmediavault.device.StorageDevice.from_device_file('/dev/hdc3')
        self.assertIsInstance(
            sd, openmediavault.device.plugins.hd.StorageDevice
        )
        self.assertEqual(sd.parent.device_file, '/dev/hdc')
        self.assertEqual(sd.smart_device_type, 'ata')

    def test_from_device_file_i2o(self):
        sd = openmediavault.device.StorageDevice.from_device_file(
            '/dev/i2o/hdb1'
        )
        self.assertIsInstance(
            sd, openmediavault.device.plugins.i2o.StorageDevice
        )
        self.assertEqual(sd.parent.device_file, '/dev/i2o/hdb')
        self.assertTrue(sd.is_raid)

    def test_from_device_file_loop(self):
        sd = openmediavault.device.StorageDevice.from_device_file('/dev/loop2')
        self.assertIsInstance(
            sd, openmediavault.device.plugins.loop.StorageDevice
        )

    def test_from_device_file_md(self):
        sd = openmediavault.device.StorageDevice.from_device_file('/dev/md0p1')
        self.assertIsInstance(
            sd, openmediavault.device.plugins.md.StorageDevice
        )
        self.assertEqual(sd.parent.device_file, '/dev/md0')
        self.assertTrue(sd.is_raid)

    def test_from_device_file_mmc(self):
        sd = openmediavault.device.StorageDevice.from_device_file(
            '/dev/mmcblk0p1'
        )
        self.assertIsInstance(
            sd, openmediavault.device.plugins.mmc.StorageDevice
        )
        self.assertEqual(sd.parent.device_file, '/dev/mmcblk0')
        self.assertFalse(sd.is_rotational)

    def test_from_device_file_nvm(self):
        sd = openmediavault.device.StorageDevice.from_device_file(
            '/dev/nvme0n1p1'
        )
        self.assertIsInstance(
            sd, openmediavault.device.plugins.nvm.StorageDevice
        )
        self.assertEqual(sd.parent.device_file, '/dev/nvme0n1')
        self.assertFalse(sd.is_rotational)
        self.assertEqual(sd.smart_device_type, 'nvme')

    def test_from_device_file_rbd(self):
        sd = openmediavault.device.StorageDevice.from_device_file(
            '/dev/rbd1p1')
        self.assertIsInstance(
            sd, openmediavault.device.plugins.rbd.StorageDevice
        )
        self.assertEqual(sd.parent.device_file, '/dev/rbd1')
        self.assertFalse(sd.is_rotational)

    @mock.patch('pyudev.Devices.from_device_file')
    @mock.patch('openmediavault.device.plugins.sd.HCTL.from_dev_path')
    def test_from_device_file_sd(self, mock_hctl, mock_from_device_file):
        mock_hctl.return_value = openmediavault.device.plugins.sd.HCTL(
            2, 0, 0, 0)
        mock_from_device_file.return_value = MockedPyUdevDevice({
            'ID_BUS': 'usb',
            'DEVPATH': '/devices/pci0000:00/0000:00:17.0/ata3/host2/target2:0:0/2:0:0:0/block/sda'
        })
        sd = openmediavault.device.StorageDevice.from_device_file('/dev/sda1')
        self.assertIsInstance(
            sd, openmediavault.device.plugins.sd.StorageDevice
        )
        self.assertEqual(sd.parent.device_file, '/dev/sda')
        self.assertTrue(sd.is_usb)
        self.assertEqual(sd.smart_device_type, 'sat')

    def test_from_device_file_sg(self):
        sd = openmediavault.device.StorageDevice.from_device_file('/dev/sg17')
        self.assertIsInstance(
            sd, openmediavault.device.plugins.sg.StorageDevice
        )
        self.assertEqual(sd.smart_device_type, 'scsi')

    def test_from_device_file_virtio(self):
        sd = openmediavault.device.StorageDevice.from_device_file('/dev/vda1')
        self.assertIsInstance(
            sd, openmediavault.device.plugins.virtio.StorageDevice
        )
        self.assertEqual(sd.parent.device_file, '/dev/vda')
        self.assertEqual(sd.smart_device_type, 'sat')

    def test_from_device_file_xen(self):
        sd = openmediavault.device.StorageDevice.from_device_file(
            '/dev/xvdtq37'
        )
        self.assertIsInstance(
            sd, openmediavault.device.plugins.xen.StorageDevice
        )
        self.assertEqual(sd.parent.device_file, '/dev/xvdtq')

    @mock.patch('builtins.open', new=f_open)
    @mock.patch('pyudev.Devices.from_device_file')
    @mock.patch('openmediavault.device.plugins.sd.HCTL.from_dev_path')
    def test_hpsa_raid(self, mock_hctl, mock_from_device_file):
        self.fs.create_file('/sys/class/scsi_disk/0:1:0:0/device/raid_level',
                            contents='''RAID 1(+0)\n''')
        self.fs.create_file('/sys/class/scsi_host/host0/proc_name',
                            contents='''hpsa\n''')
        mock_hctl.return_value = openmediavault.device.plugins.sd.HCTL(
            0, 1, 0, 0)
        mock_from_device_file.return_value = MockedPyUdevDevice({
            'ID_BUS': 'scsi',
            'DEVPATH': '/devices/pci0000:00/0000:00:01.0/0000:04:00.0/host0'
                       '/target0:1:0/0:1:0:0/block/sdd'
        })
        sd = openmediavault.device.StorageDevice.from_device_file('/dev/sdd')
        self.assertIsInstance(
            sd, openmediavault.device.plugins.sd.StorageDeviceHPSA
        )
        self.assertTrue(sd.is_raid)
        self.assertEqual(sd.smart_device_type, '')

    @mock.patch('builtins.open', new=f_open)
    @mock.patch('pyudev.Devices.from_device_file')
    @mock.patch('openmediavault.device.plugins.sd.HCTL.from_dev_path')
    def test_hpsa_hba(self, mock_hctl, mock_from_device_file):
        self.fs.create_file('/sys/class/scsi_disk/1:0:5:0/device/raid_level',
                            contents='''N/A\n''')
        self.fs.create_file('/sys/class/scsi_host/host1/proc_name',
                            contents='''hpsa\n''')
        mock_hctl.return_value = openmediavault.device.plugins.sd.HCTL(
            1, 0, 5, 0)
        mock_from_device_file.return_value = MockedPyUdevDevice({
            'ID_BUS': 'scsi',
            'DEVPATH': '/devices/pci0000:00/0000:00:02.2/0000:02:00.0/host1'
                       '/scsi_host/host1/port-1:6/end_device-1:6/target1:0:5'
                       '/1:0:5:0/block/sde'
        })
        sd = openmediavault.device.StorageDevice.from_device_file('/dev/sde')
        self.assertIsInstance(
            sd, openmediavault.device.plugins.sd.StorageDeviceHPSA
        )
        self.assertFalse(sd.is_raid)
        self.assertEqual(sd.smart_device_type, 'cciss,4')


if __name__ == "__main__":
    unittest.main()
