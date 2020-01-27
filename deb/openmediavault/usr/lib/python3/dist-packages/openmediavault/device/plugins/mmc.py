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
import os
import re

import openmediavault.device


class StorageDevicePlugin(openmediavault.device.IStorageDevicePlugin):
    def match(self, device_file):
        # Examples:
        # - /dev/mmcblk0
        # - /dev/mmcblk0p1
        device_name = re.sub(r'^/dev/', '', device_file)
        return re.match(r'^mmcblk[0-9]+(p[0-9]+)*$', device_name) is not None

    def from_device_file(self, device_file):
        return StorageDevice(device_file)


class StorageDevice(openmediavault.device.StorageDevice):
    """
    Implements the storage device for Multimedia Card (MMC) devices.
    """

    @property
    def parent(self):
        # /dev/mmcblk0p1 -> /dev/mmcblk0
        parent_device_file = re.sub(r'(p\d+)$', '', self.canonical_device_file)
        if parent_device_file != self.canonical_device_file:
            return self.__class__(parent_device_file)
        return None

    @property
    def is_rotational(self):
        return False
