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
        # - /dev/sg0
        # - /dev/sg17
        device_name = re.sub(r'^/dev/', '', device_file)
        return re.match(r'^sg[0-9]+$', device_name) is not None

    def from_device_file(self, device_file):
        return StorageDevice(device_file)


class StorageDevice(openmediavault.device.StorageDevice):
    """
    Implements the storage device for SCSI generic devices.
    """

    @property
    def description(self):
        return 'SCSI Generic'

    @property
    def exists(self):
        return openmediavault.device.is_char_device(self.device_file)

    @property
    def type(self):
        """
        Get the SCSI type.
        @see http://www.tldp.org/HOWTO/SCSI-Generic-HOWTO/proc.html
        :return: Returns the SCSI type, e.g. 0->disk, 5->cdrom, 6->scanner,
            otherwise ``False``.
        :rtype: int|None
        """
        file = '/sys/class/scsi_generic/{}/device/type'.format(
            self.device_name(True)
        )
        try:
            with open(file, 'r') as f:
                return int(f.readline().strip())
        except (IOError, FileNotFoundError):
            pass
        return None

    @property
    def smart_device_type(self):
        return 'scsi'
