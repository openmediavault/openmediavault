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
import openmediavault.config
import openmediavault.device
import openmediavault.fs
import os
import re

# Import Salt libs
import salt.utils.network
from salt.utils.decorators.jinja import jinja_filter


def register_jinja_filters():
    """
    Call this function in a Salt state file to be able to use the
    custom Jinja filters shipped by this execution module in any
    Jinja template.
    Note, it is NOT necessary to call this function if any other
    function of this execution module is called in your Salt state
    file. In this case Salt has already registered all Jinja filters
    in this execution module.
    """
    pass


def is_ipv6_enabled():
    """
    Check whether IPv6 is enabled.
    :return: Return True if IPv6 is enabled, otherwise False.
    :rtype: bool
    """
    if not os.path.exists('/proc/net/if_inet6'):
        return False
    with open('/proc/net/if_inet6') as f:
        lines = f.readlines()
    # Filter unwanted interfaces.
    lines = [l for l in lines if not re.match(r'^\s+lo$', l)]
    return len(lines) > 0


@jinja_filter('network_prefix_len')
def get_net_size(mask):
    """
    Turns an IPv4 netmask into it's corresponding prefix length
    (255.255.255.0 -> 24 as in 192.168.1.10/24).
    :param mask: The network mask.
    :type mask: str
    :return: Returns the prefix length.
    :rtype: int
    """
    return salt.utils.network.get_net_size(mask)


@jinja_filter('is_dir')
def is_dir(path):
    """
    Check if path is an existing directory.
    :param path: The path to check.
    :type path: str
    :return: Return True if path is an existing directory,
        otherwise False.
    :rtype: bool
    """
    return os.path.isdir(os.path.expanduser(path))


@jinja_filter('is_file')
def is_file(path):
    """
    Check if path is an existing regular file. This follows symbolic
    links, so both islink() and isfile() can be true for the same path.
    :param path: The path to check.
    :type path: str
    :return: Return True if path is an existing regular file,
        otherwise False.
    :rtype: bool
    """
    return os.path.isfile(os.path.expanduser(path))


@jinja_filter('is_block_device')
def is_block_device(path):
    """
    Check if path is a block device.
    :param path: The path to check.
    :type path: str
    :return: Return True if path is a block device, otherwise False.
    :rtype: bool
    """
    return openmediavault.device.is_block_device(path)


@jinja_filter('is_device_file')
def is_device_file(path):
    """
    Check if path describes a device file, e.g. /dev/sda1.
    :param path: The path to check.
    :type path: str
    :return: Return True if path describes a device file,
        otherwise False.
    :rtype: bool
    """
    return openmediavault.device.is_device_file(path)


def is_rotational(path):
    """
    Check if device is rotational.
    :param path: The path to check.
    :type path: str
    :return: Return True if the device is rotational, otherwise False.
    :rtype: bool
    """
    if not is_device_file(path) or not is_block_device(path):
        return False
    sd = openmediavault.device.StorageDevice(path)
    return sd.is_rotational()


def get_fs_parent_device_file(id_):
    """
    Get the parent device of the specified filesystem.
    :param id_: The filesystem identifier, e.g.

    * /dev/sde1
    * /dev/disk/by-id/scsi-0QEMU_QEMU_HARDDISK_drive-scsi0-0-1-part1
    * /dev/cciss/c0d0p2
    * /dev/disk/by-id/md-name-vmpc01:data
    * 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
    * 7A48-BA97 (FAT)
    * 2ED43920D438EC29 (NTFS)

    :type id_: str
    :return: Returns the device file of the underlying storage device
      or ``None`` in case of an error.
    :rtype: str|None
    """
    fs = openmediavault.fs.Filesystem(id_)
    return fs.get_parent_device_file()


def get_root_filesystem():
    """
    Get the device file of the root filesystem, e.g. '/dev/sda1'.
    :return: The device file of the root filesystem.
    :rtype: str
    """
    fs = openmediavault.fs.Filesystem.from_mount_point('/')
    return fs.device_file
