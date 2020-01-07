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
import functools
import re

from .block import BlockDevice

import openmediavault.string


class StorageDevice(BlockDevice):
    @functools.lru_cache()
    def get_model(self):
        """
        Get the device model.
        :return: The device model, otherwise an empty string.
        :rtype: str
        """
        return openmediavault.string.unescape_blank(
            self.get_udev_property('ID_MODEL_ENC', '')
        )

    @functools.lru_cache()
    def get_vendor(self):
        """
        Get the device vendor.
        :return: The device vendor, otherwise an empty string.
        :rtype: str
        """
        return openmediavault.string.unescape_blank(
            self.get_udev_property('ID_VENDOR_ENC', '')
        )

    @functools.lru_cache()
    def get_serial(self):
        """
        Get the device serial number.
        :return: The device serial number, otherwise an empty string.
        :rtype: str
        """
        serial = self.get_udev_property('ID_SERIAL_SHORT', '')
        return serial.replace('_', ' ')

    @functools.lru_cache()
    def is_rotational(self):
        """
        Check if the device is of rotational or non-rotational type.
        See https://www.kernel.org/doc/Documentation/block/queue-sysfs.txt
        :return: Return True if device is rotational, otherwise False.
        :rtype: bool
        """
        if self.has_udev_property('ID_SSD'):
            # If ID_SSD is not 1 then it is rotational.
            return self.get_udev_property('ID_SSD') != '1'
        if self.has_udev_property('ID_ATA_ROTATION_RATE_RPM'):
            # If ID_ATA_ROTATION_RATE_RPM is non-zero then it is rotational.
            return self.get_udev_property('ID_ATA_ROTATION_RATE_RPM') != '0'
        if self.has_udev_property('ID_ATA_FEATURE_SET_AAM'):
            # If ID_ATA_FEATURE_SET_AAM is non-zero then it is rotational.
            return self.get_udev_property('ID_ATA_FEATURE_SET_AAM') != '0'
        # Use kernel attribute.
        file = '/sys/block/{}/queue/rotational'.format(self.device_name(True))
        try:
            with open(file, 'r') as f:
                # 0 => SSD, 1 => HDD
                return f.readline().strip() != '0'
        except (IOError, FileNotFoundError):
            pass
        # Use heuristic.
        return 'SSD' not in self.get_model()

    @functools.lru_cache()
    def is_removable(self):
        """
        Check if the device is removable.
        :return: Returns ``True`` if the device is removable,
            otherwise ``False``.
        :rtype: bool
        """
        file = '/sys/block/{}/removable'.format(self.device_name(True))
        try:
            with open(file, 'r') as f:
                return f.readline().strip() == '1'
        except (IOError, FileNotFoundError):
            pass
        return False

    @functools.lru_cache()
    def is_usb(self):
        """
        Check if the given device is an USB device.
        :return: Returns ``True`` if the device is connected via USB,
            otherwise ``False``.
        :rtype: bool
        """
        # Identify USB devices via 'ID_BUS=usb'.
        if self.has_udev_property('ID_BUS'):
            return self.get_udev_property('ID_BUS') == 'usb'
        # Identify USB devices via 'ID_USB_DRIVER=usb-storage'.
        if self.has_udev_property('ID_USB_DRIVER'):
            return self.get_udev_property('ID_USB_DRIVER') == 'usb-storage'
        # Identify USB devices via 'ID_DRIVE_THUMB=1'.
        if self.has_udev_property('ID_DRIVE_THUMB'):
            return self.get_udev_property('ID_DRIVE_THUMB') == '1'
        # Identify USB devices via 'ID_PATH=xxx-usb-xxx'.
        # Example:
        # ID_PATH=pci-0000:02:02.0-usb-0:1:1.0-scsi-0:0:0:0
        # ID_PATH=pci-0000:00:12.2-usb-0:3:1.0-scsi-0:0:0:0
        if self.has_udev_property('ID_PATH'):
            value = self.get_udev_property('ID_PATH')
            return re.match(r'^.+-usb-.+$', value)
        return False

    def wipe(self):
        """
        Wipe the storage device.
        """
        _ = openmediavault.subprocess.check_output(
            ['sgdisk', '--zap-all', self.device_file]
        )
