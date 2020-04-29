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
import re

import openmediavault.device


class StorageDevicePlugin(openmediavault.device.IStorageDevicePlugin):
    def match(self, device_file):
        device_name = re.sub(r'^/dev/', '', device_file)
        return re.match(r'^sd[a-z]+[0-9]*$', device_name) is not None

    def from_device_file(self, device_file):
        sd = openmediavault.device.StorageDevice(device_file)
        if sd.host_driver == 'arcmsr':
            # Areca RAID controller
            return StorageDeviceARCMSR(device_file)
        if sd.host_driver == 'hpsa':
            # Logical drives from HP Smart Array RAID controllers
            # are accessed via the SCSI disk driver, so we need to
            # identify such drives because these must be handled
            # different in some cases (e.g. S.M.A.R.T.).
            # @see https://linux.die.net/man/4/hpsa
            return StorageDeviceHPSA(device_file)
        return StorageDevice(device_file)


class StorageDevice(openmediavault.device.StorageDevice):
    """
    Implements the storage device for SCSI disk devices.
    """

    @property
    def parent(self):
        # /dev/sda1 -> /dev/sda
        parent_device_file = re.sub(r'(\d+)$', '', self.canonical_device_file)
        if parent_device_file != self.canonical_device_file:
            return self.__class__(parent_device_file)
        return None

    @property
    def description(self):
        return 'SCSI Disk'

    @property
    def smart_device_type(self):
        return 'sat' if self.is_usb else ''


class StorageDeviceARCMSR(StorageDevice):
    """
    Implements the storage device for Areca RAID controllers using
    the arcmsr driver.
    """

    @property
    def serial(self):
        if self.has_udev_property('SCSI_IDENT_SERIAL'):
            return self.udev_property('SCSI_IDENT_SERIAL')
        return super().serial

    @property
    def is_raid(self):
        return True


class StorageDeviceHPSA(StorageDevice):
    """
    Implements the storage device for HP Smart Array RAID controllers
    using the hpsa driver.
    """

    @property
    def is_raid(self):
        return True
