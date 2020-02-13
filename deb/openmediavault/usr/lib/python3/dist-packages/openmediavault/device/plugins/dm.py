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

import openmediavault.collections
import openmediavault.device
import openmediavault.subprocess


class StorageDevicePlugin(openmediavault.device.IStorageDevicePlugin):
    def match(self, device_file):
        # Examples:
        # - /dev/mapper/sda-crypt
        # - /dev/dm-2
        device_name = re.sub(r'^/dev/', '', device_file)
        return re.match(r'^(mapper/.+)|(dm-\d+)$', device_name) is not None

    def from_device_file(self, device_file):
        sd = StorageDevice(device_file)
        subsystem = sd.subsystem
        if subsystem == 'lvm':
            return StorageDeviceLVM(device_file)
        if subsystem in ['crypt', 'dmraid', 'mpath', 'part']:
            # Fall through to default implementation.
            pass
        return StorageDevice(device_file)


class StorageDevice(openmediavault.device.StorageDevice):
    """
    Implements the storage device for device-mapper devices.
    @see http://en.wikipedia.org/wiki/Device_mapper
    """

    @property
    def description(self):
        return 'Device Mapper'

    @property
    def subsystem(self):
        """
        Get the device mapper subsystem, e.g. 'lvm', 'crypt',
        'dmraid', 'mpath', 'part', ...
        :return: Returns the device mapper subsystem or ``None``
            on failure.
        :rtype: str|None
        """
        # The DM_UUID prefix should be set to subsystem owning the device:
        # LVM, CRYPT, DMRAID, MPATH, PART
        # @see https://github.com/karelzak/util-linux/blob/master/misc-utils/lsblk.c#L389
        file = '/sys/block/{}/dm/uuid'.format(self.device_name(True))
        try:
            with open(file, 'r') as f:
                content = f.readline().strip()
                # LVM-EOVOgvd6WfgpbYBVvXdlde7g3Lsgyhi31UL0XD8KuxuPdI9awDCm8RSfUgn5TZQR
                # CRYPT-LUKS2-f53432fa858947c1930fee05b2658b95-mytest
                parts = content.split('-')
                if len(parts) >= 2:
                    return parts[0].lower()
        except (IOError, FileNotFoundError):
            pass
        return None

    @property
    def name(self):
        """
        Get the name of the device mapper device.
        :return: Returns the name of the device or an empty string on failure.
        :rtype: str
        """
        file = '/sys/block/{}/dm/name'.format(self.device_name(True))
        try:
            with open(file, 'r') as f:
                return f.readline().strip()
        except (IOError, FileNotFoundError):
            pass
        return ''

    @property
    def uuid(self):
        """
        Get the UUID of the device.
        Examples:
        - LVM-EOVOgvd6WfgpbYBVvXdlde7g3Lsgyhi31UL0XD8KuxuPdI9awDCm8RSfUgn5TZQR
        - CRYPT-LUKS2-f53432fa858947c1930fee05b2658b95-mytest
        :return: Returns the UUID of the device or an empty string on failure.
        :rtype: str
        """
        file = '/sys/block/{}/dm/uuid'.format(self.device_name(True))
        try:
            with open(file, 'r') as f:
                return f.readline().strip()
        except (IOError, FileNotFoundError):
            pass
        return ''

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


class StorageDeviceLVM(StorageDevice):
    """
    Implements the storage device for LVM logical volumes.
    """

    def __init__(self, device_file):
        """
        :param device_file: Specifies the device file, e.g. /dev/dm-1,
            /dev/vg0/lv0 or /dev/mapper/vg0-lv0.
        :type device_file: str
        """
        # Call parent constructor.
        super().__init__(device_file)

        # Init internals
        self._cached = False
        self._lv_name = None
        self._lv_attr = None
        self._vg_name = None
        self._vg_attr = None
        self._kernel_major = None
        self._kernel_minor = None
        self._size = None
        self._uuid = None

        # Any devices of the form /dev/dm-n are for internal use only
        # and should never be used.
        #
        # Example:
        # # lvdisplay --noheadings --separator ... --unit b /dev/dm-0
        #   Volume group "dm-0" not found
        #   Skipping volume group dm-0
        #
        # Because of that we simply use device files that look like
        # /dev/mapper/<xyz>.
        if re.match(r'^/dev/dm-\d+$', self.device_file):
            name = self.name
            if name:
                self._device_file = os.path.join('/dev', 'mapper', name)

    def _get_data(self):
        if self._cached:
            return

        output = openmediavault.subprocess.check_output(
            [
                'lvdisplay',
                '--noheadings',
                '--separator "|"',
                '-C',
                '-o lv_uuid,lv_name,vg_name,lv_size,lv_attr,'
                'lv_kernel_major,lv_kernel_minor,vg_attr',
                '--unit b',
                self.device_file,
            ]
        )
        # CRMY9K-pnVP-oxNv-bimZ-1WoX-FoAe-yLgGwk|lv01_snapshot_20170412-150418|vg01|569410322432B|swi-aos---|253|1
        parts = output.decode().split('|')

        self._uuid = parts[0]
        self._lv_name = parts[1]
        self._vg_name = parts[2]
        self._size = parts[3][:-1]
        self._lv_attr = parts[4]
        self._kernel_major = int(parts[5])
        self._kernel_minor = int(parts[6])
        self._vg_attr = parts[7]

        self._cached = True

    @property
    def description(self):
        """
        :rtype: str
        """
        return 'LVM Logical Volume'

    @property
    def exists(self):
        """
        Checks if the logical volume exists.
        :rtype: bool
        """
        try:
            self._get_data()
        except Exception:
            return False
        return 0 < len(self.lv_uuid)

    @property
    def uuid(self):
        """
        Get the UUID of the logical volume.
        :return: The UUID of the logical volume.
        :rtype: str
        """
        self._get_data()
        return self._uuid

    @property
    def lv_name(self):
        """
        Get the name of the logical volume.
        :return: Returns the logical volume name.
        :rtype: str
        """
        self._get_data()
        return self._lv_name

    @property
    def vg_name(self):
        """
        Get the name of the volume group.
        :return: Returns the volume group name.
        :rtype: str
        """
        self._get_data()
        return self._vg_name

    @property
    def lv_device_file(self):
        """
        Get the path of the logical volume.
        :return: The logical volume path, e.g. /dev/vg0/lvol0.
        :rtype: str
        """
        self._get_data()
        return os.path.join('/dev', self.vg_name, self.lv_name)

    def size(self):
        """
        Get the size of the logical volume in bytes.
        :return: The size of the logical volume in bytes.
        :rtype: int
        """
        self._get_data()
        return int(self._size)

    @property
    def lv_attributes(self):
        """
        Get the logical volume attributes.
        @see http://www.unixarena.com/2013/08/redhat-linux-lvm-volume-attributes.html
        :rtype: dict
        """
        self._get_data()
        # Example:
        # - owi-aos---
        # - swi-a-s---
        return {
            'origin': 'o' == self._lv_attr[0, 1],
            'snapshot': 's' == self._lv_attr[0, 1],
            'invalidsnapshot': 'S' == self._lv_attr[0, 1],
            'mirrored': 'm' == self._lv_attr[0, 1],
            'virtual': 'v' == self._lv_attr[0, 1],
            'pvmove': 'p' == self._lv_attr[0, 1],
            'state': {
                'active': 'a' == self._lv_attr[4, 1],
                'suspended': 's' == self._lv_attr[4, 1],
                'invalidsnapshot': 'I' == self._lv_attr[4, 1],
                'invalidsuspendedsnapshot': 'S' == self._lv_attr[4, 1],
            },
            'device': {
                'open': 'o' == self._lv_attr[5, 1],
                'unknown': 'X' == self._lv_attr[5, 1],
            },
        }

    @property
    def is_media_available(self):
        attr = openmediavault.collections.DotDict(self.lv_attributes)
        return attr.get('state.active')
