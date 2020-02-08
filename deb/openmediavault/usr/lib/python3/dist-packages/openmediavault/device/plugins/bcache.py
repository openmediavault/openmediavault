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
        device_name = re.sub(r'^/dev/', '', device_file)
        return re.match(r'^bcache[0-9]+$', device_name) is not None

    def from_device_file(self, device_file):
        return StorageDevice(device_file)


class StorageDevice(openmediavault.device.StorageDevice):
    @property
    def description(self):
        return 'Block layer cache'

    @property
    def slaves(self):
        """
        Get the slave devices of this device.
        :return: A list of device files.
        :rtype: list[str]
        """
        # Make sure the canonical device file is used to extract the
        # name of the device.
        path = '/sys/block/{}/slaves'.format(self.canonical_device_file)
        if not os.path.exists(path):
            return []
        result = []
        for name in os.listdir(path):
            file_path = os.path.join(path, name)
            if os.path.islink(file_path):
                result.append(os.path.join('/dev', name))
        return result
