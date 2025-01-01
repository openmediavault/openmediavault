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

import openmediavault.device


class DeviceTestCase(unittest.TestCase):
    def test_is_device_file(self):
        self.assertFalse(openmediavault.device.is_device_file('/foo/bar'))
        self.assertTrue(openmediavault.device.is_device_file('/dev/sda'))

    def test_is_device_file_by(self):
        self.assertFalse(openmediavault.device.is_device_file_by('/dev/sda'))
        self.assertTrue(
            openmediavault.device.is_device_file_by(
                '/dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895'
            )
        )
        self.assertTrue(
            openmediavault.device.is_device_file_by(
                '/dev/disk/by-id/usb-Kingston_DataTraveler_G2_001CC0EC21ADF011C6A20E35-0:0-part1'
            )
        )
        self.assertTrue(
            openmediavault.device.is_device_file_by('/dev/disk/by-label/data')
        )

    def test_is_device_file_by_uuid(self):
        self.assertFalse(
            openmediavault.device.is_device_file_by_uuid('/dev/sda')
        )
        self.assertTrue(
            openmediavault.device.is_device_file_by_uuid(
                '/dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895'
            )
        )

    def test_is_device_file_by_id(self):
        self.assertFalse(
            openmediavault.device.is_device_file_by_id('/dev/sda'))
        self.assertTrue(
            openmediavault.device.is_device_file_by_id(
                '/dev/disk/by-id/wwn-0x4002c554a4d79cb9-part2'
            )
        )

    def test_is_device_file_by_label(self):
        self.assertFalse(
            openmediavault.device.is_device_file_by_label('/dev/sda')
        )
        self.assertTrue(
            openmediavault.device.is_device_file_by_label(
                '/dev/disk/by-label/data'
            )
        )

    def test_is_device_file_by_path(self):
        self.assertFalse(
            openmediavault.device.is_device_file_by_path('/dev/sda')
        )
        self.assertTrue(
            openmediavault.device.is_device_file_by_path(
                '/dev/disk/by-path/pci-0000:00:17.0-ata-3'
            )
        )


if __name__ == "__main__":
    unittest.main()
