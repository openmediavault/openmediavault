# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2019 Volker Theile
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
import pyudev
import subprocess

import openmediavault
import openmediavault.device
import openmediavault.subprocess
import openmediavault.string


def make_mount_path(id_):
    """
    Build the mount path from any given string, device file or
    file system UUID.
    :param id_: The device file or the file system UUID.
    :type id_: str
    :return: Returns the mount path, e.g

    * /srv/6c5be784-50a8-440c-9d25-aab99b9c6fb1/
    * /srv/_dev_disk_by-id_wwn-0x5000cca211cc703c-part1/

    :rtype: str
    """
    mount_path = os.path.join(
        openmediavault.getenv("OMV_MOUNT_DIR", "/srv"), id_.replace('/', '_')
    )
    return openmediavault.string.path_prettify(mount_path)


class Filesystem(openmediavault.device.BlockDevice):
    _id = None

    def __init__(self, id_):
        self._id = id_
        super().__init__(None)

    @classmethod
    def from_mount_point(cls, path):
        """
        Create a new filesystem for the specified mount point.
        :param path: The mount point, e.g.
            - /
            - /srv/dev-disk-by-id-scsi-0QEMU_QEMU_HARDDISK_drive-scsi0-0-1-part1
        :type path: str
        :return: Return a :class:`Filesystem` object for the mount point
            or ``None`` in case of an error.
        :rtype: :class:`Filesystem`
        :raises: :exc:`~exceptions.Exception`, if the given path is no
            mount point.
        """
        if not os.path.ismount(path):
            raise Exception('Path \'{}\' is not a mount point'.format(path))
        st = os.stat(path)
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

    @property
    def uuid(self):
        if openmediavault.string.is_fs_uuid(self._id):
            return self._id
        return self.get_udev_property('ID_FS_UUID')

    def get_parent_device_file(self):
        """
        Get the parent device.

        * /dev/sdb1 => /dev/sdb
        * /dev/cciss/c0d0p2 => /dev/cciss/c0d0
        * /dev/md0 => /dev/md0
        * /dev/dm-0 => /dev/dm-0

        :return: Returns the device file of the underlying storage device
            or ``None`` in case of an error.
        :rtype: str|None
        """
        context = pyudev.Context()
        device = pyudev.Devices.from_device_file(context, self.device_file)
        if device and device.parent:
            if device.parent.device_node is not None:
                return device.parent.device_node
        """
        Device mapper devices must be specially treated.
        """
        if re.match(r'^(dm-|md)\d+$', self.device_name(True)):
            return self.device_file
        return None

    def has_label(self):
        """
        Check if the filesystem has a label.
        :return: Returns ``True`` if the filesystem has a label,
            otherwise ``False``.
        :rtype: bool
        """
        return self.has_udev_property('ID_FS_LABEL_ENC')

    def get_label(self):
        """
        Get the filesystem label.
        :return: Returns the label of the filesystem.
        :rtype: str
        """
        return openmediavault.string.unescape_blank(
            self.get_udev_property('ID_FS_LABEL_ENC', '')
        )

    def get_type(self):
        """
        Get the filesystem type, e.g. 'ext3' or 'vfat'.
        :return: The filesystem type.
        :rtype: str
        """
        return self.get_udev_property('ID_FS_TYPE')

    def get_partition_scheme(self):
        """
        Get the partition scheme, e.g. 'gpt', 'mbr', 'apm' or 'dos'.
        :return: Returns the partition scheme, otherwise ``None`` on
            failure or if it does not exist.
        :rtype: str|None
        """
        return self.get_udev_property('ID_PART_ENTRY_SCHEME', None)

    def get_mount_point(self):
        """
        Get the mount point of the filesystem.
        :return: The mount point of the filesystem or None.
        :rtype: str|None
        """
        try:
            output = openmediavault.subprocess.check_output([
                'findmnt', '--canonicalize', '--first-only', '--noheadings',
                '--output=TARGET', '--raw', self.canonical_device_file
            ])
            # Examples:
            # /media/8c982ec2-8aa7-4fe2-a912-7478f0429e06
            # /srv/_dev_disk_by-id_dm-name-vg01-lv01
            # /srv/dev-disk-by-label-xx\x20yy
            return openmediavault.string.unescape_blank(
                output.decode().strip()
            )
        except subprocess.CalledProcessError:
            pass
        return None

    def is_mounted(self):
        """
        Check if a filesystem is mounted.
        """
        try:
            _ = openmediavault.subprocess.check_output([
                'findmnt', '--canonicalize', '--first-only', '--noheadings',
                '--raw', '--nofsroot', self.canonical_device_file
            ])
            return True
        except subprocess.CalledProcessError:
            pass
        return False

    def mount(self, path=None):
        """
        Mount the filesystem.
        :param path: The mount directory. Defaults to ``None``.
        :type path: str
        """
        args = ['mount', '--verbose', '--source', self.device_file]
        if path is not None:
            args.append(path)
        _ = openmediavault.subprocess.check_output(args)

    def umount(self, force=False, lazy=False):
        """
        Unmount the filesystem.
        :param force: Set to `True`` to force an unmount. Defaults
            to ``False``.
        :type force: bool
        :param lazy: Set to ``True`` to detach the filesystem from
            the file hierarchy now, and clean up all references to
            this filesystem as soon as it is not busy anymore.
            Defaults to ``False``.
        :type lazy: bool
        """
        args = ['umount', '--verbose']
        if force:
            args.append('--force')
        if lazy:
            args.append('--lazy')
        args.append(self.device_file)
        _ = openmediavault.subprocess.check_output(args)

    def remove(self):
        """
        Remove the filesystem.
        """
        # Whether the partition schema is 'dos' then it is necessary to
        # erase the MBR before, otherwise 'wipefs' fails, e.g.:
        # wipefs: error: / dev / sdh1: appears to contain 'dos' partition table
        if self.get_partition_scheme() in ['dos', 'vfat']:
            # http://en.wikipedia.org / wiki / Master_boot_record
            _ = openmediavault.subprocess.check_output([
                'dd', 'if=/dev/zero', 'of={}'.format(self.device_file),
                'count=1'
            ])
        _ = openmediavault.subprocess.check_output([
            'wipefs', '--all', self.device_file
        ])

    def grow(self):
        """
        Grow the filesystem.
        """
        raise NotImplementedError

    def shrink(self):
        """
        Shrink the filesystem.
        """
        raise NotImplementedError
