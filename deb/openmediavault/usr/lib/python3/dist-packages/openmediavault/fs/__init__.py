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
import openmediavault.device
import openmediavault.subprocess
import openmediavault.string
import os
import pyudev


class Filesystem(openmediavault.device.BlockDevice):
    _id = None

    def __init__(self, id_):
        self._id = id_
        super().__init__(None)

    @classmethod
    def from_root(cls):
        """
        Create a new filesystem for the root filesystem.
        :return: Return a :class:`Filesystem` object for the root filesystem.
        """
        # output = openmediavault.subprocess.check_output([
        #     'findmnt', '--first-only', '--noheadings', '--output=SOURCE', '/'
        # ])
        # id_ = output.decode().strip()
        st = os.stat('/')
        context = pyudev.Context()
        device = pyudev.Devices.from_device_number(context, 'block', st.st_dev)
        return Filesystem(device.device_node)

    @property
    def device_file(self):
        if self._device_file is None:
            if openmediavault.string.is_fs_uuid(self._id):
                # Find the filesystem.
                output = openmediavault.subprocess.check_output([
                    'findfs', 'UUID={}'.format(self._id)
                ])
                self._device_file = output.decode().strip()
            else:
                # We assume the specified filesystem identifier is
                # a device file.
                self._device_file = self._id
        return super().device_file

    def get_parent_device_file(self):
        """
        Get the parent device.

        * /dev/sdb1 => /dev/sdb
        * /dev/cciss/c0d0p2 => /dev/cciss/c0d0

        :return: Returns the device file of the underlying storage device
          or ``None`` in case of an error.
        :rtype: str|None
        """
        context = pyudev.Context()
        device = pyudev.Devices.from_device_file(context, self.device_file)
        parent_device = device.parent
        return parent_device.device_node if parent_device is not None else None
