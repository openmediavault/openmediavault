# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2021 Volker Theile
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
import socket

import openmediavault.config
import openmediavault.device
import openmediavault.fs
import openmediavault.net
import openmediavault.string
# Import Salt libs
import salt.utils.network
from salt.utils.decorators.jinja import jinja_filter, jinja_test


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
    result = False
    s = None
    try:
        s = socket.socket(socket.AF_INET6)
        s.bind(('::1', 0))
        result = True
    except Exception:
        pass
    finally:
        if s:
            s.close()
    return result


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


@jinja_filter('is_link')
def is_link(path):
    """
    Check if path is a symbolic link.
    :param path: The path to check.
    :type path: str
    :return: Return True if path is a symbolic link, otherwise False.
    :rtype: bool
    """
    return os.path.islink(os.path.expanduser(path))


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
    sd = openmediavault.device.StorageDevice.from_device_file(
        os.path.realpath(path)
    )
    return sd.is_rotational


@jinja_filter('is_uuid4')
def is_uuid4(value):
    """
    Finds out whether a variable is an UUID v4.
    :param value: The variable being evaluated.
    :type value: str
    :return: Returns ``True`` if the variable is an UUIDv4,
        otherwise ``False``.
    :rtype: bool
    """
    return openmediavault.string.is_uuid4(value)


@jinja_filter('is_fs_uuid')
def is_fs_uuid(value):
    """
    Finds out whether a variable is a filesystem UUID.

    Example:
    - 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
    - 7A48-BA97 (FAT)
    - 2ED43920D438EC29 (NTFS)
    - 2015-01-13-21-48-46-00 (ISO9660)

    :param value: The variable being evaluated.
    :type value: str
    :return: Returns ``True`` if the variable is a filesystem UUID,
        otherwise ``False``.
    :rtype: bool
    """
    return openmediavault.string.is_fs_uuid(value)


def get_fs_parent_device_file(id_):
    """
    Get the parent device file of the specified filesystem.
    :param id_: The filesystem identifier, e.g.

    * /dev/sde1 => /dev/sde
    * /dev/disk/by-id/scsi-0QEMU_QEMU_HARDDISK_drive-scsi0-0-1-part1 => .../scsi-0QEMU_QEMU_HARDDISK_drive-scsi0-0-1
    * /dev/cciss/c0d0p2 => /dev/cciss/c0d0
    * /dev/disk/by-id/md-name-vmpc01:data => /dev/disk/by-id/md-name-vmpc01:data
    * 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
    * 7A48-BA97 (FAT)
    * 2ED43920D438EC29 (NTFS)

    :type id_: str
    :return: Returns the device file of the underlying storage device
      or ``None`` in case of an error.
    :rtype: str|None
    """
    fs = openmediavault.fs.Filesystem(id_)
    return fs.parent_device_file


def get_root_filesystem():
    """
    Get the device file of the root filesystem, e.g. '/dev/sda1'.
    :return: The device file of the root filesystem.
    :rtype: str
    """
    fs = openmediavault.fs.Filesystem.from_mount_point('/')
    return fs.device_file


@jinja_filter('make_mount_path')
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
    return openmediavault.fs.make_mount_path(id_)


@jinja_filter('strip')
def strip(value, chars=None):
    """
    Returns a copy of the string with the leading and trailing
    characters removed.
    :param value: The string to process.
    :type value: str
    :param chars: String specifying the set of characters to be
        removed. If omitted or None, the chars argument defaults
        to removing whitespace. Defaults to ``None``.
    :type chars: str|None
    :return: Returns the stripped string.
    :rtype: str
    """
    assert isinstance(value, str)
    return value.strip(chars)


@jinja_filter('lstrip')
def lstrip(value, chars=None):
    """
    Returns a copy of the string with leading characters removed.
    :param value: The string to process.
    :type value: str
    :param chars: String specifying the set of characters to be
        removed. If omitted or None, the chars argument defaults
        to removing whitespace. Defaults to ``None``.
    :type chars: str|None
    :return: Returns the stripped string.
    :rtype: str
    """
    assert isinstance(value, str)
    return value.lstrip(chars)


@jinja_filter('rstrip')
def rstrip(value, chars=None):
    """
    Returns a copy of the string with trailing characters removed.
    :param value: The string to process.
    :type value: str
    :param chars: String specifying the set of characters to be
        removed. If omitted or None, the chars argument defaults
        to removing whitespace. Defaults to ``None``.
    :type chars: str|None
    :return: Returns the stripped string.
    :rtype: str
    """
    assert isinstance(value, str)
    return value.rstrip(chars)


@jinja_filter('add_slashes')
def add_slashes(value):
    """
    Prefix certain characters of a string with '\'.
    :param value: The string to be escaped.
    :type value: str
    :return: Returns a string with backslashes added before characters
        that need to be escaped. These characters are:
        * backslash (\)
        * single quote (')
        * double quote (")
    :rtype: str
    """
    return openmediavault.string.add_slashes(value)


@jinja_filter('path_prettify')
def path_prettify(path):
    """
    Make sure the directory path ends with a slash.
    :param path: The path to process.
    :type path: str
    :return: Returns the prettified path.
    :rtype: str
    """
    return openmediavault.string.path_prettify(path)


@jinja_filter('path_basename')
def path_basename(path):
    """
    Return the base name of pathname path.
    :param path: The string to process.
    :type path: str
    :return: Returns the base name.
    :rtype: str
    """
    return os.path.basename(path)


@jinja_filter('path_dirname')
def path_dirname(path):
    """
    Return the directory name of pathname path.
    :param path: The string to process.
    :type path: str
    :return: Returns the directory name.
    :rtype: str
    """
    return os.path.dirname(path)


@jinja_filter('path_realpath')
def path_realpath(path):
    """
    Return the canonical path of the specified filename, eliminating any
    symbolic links encountered in the path.
    :param path: The path to process.
    :type path: str
    :return: Returns canonical path.
    :rtype: str
    """
    return os.path.realpath(path)


@jinja_filter('escape_blank')
def escape_blank(value, octal=False):
    """
    Escape a string to be used in a shell environment. Blank characters
    will be replaced with their hexadecimal (\x20) or octal (\040)
    representation.

    Example:
    - /srv/dev-disk-by-label-xx yy => /srv/dev-disk-by-label-xx\x20yy
    - /srv/dev-disk-by-label-xx yy => /srv/dev-disk-by-label-xx\040yy

    :param value: The value that will be escaped.
    :type value: str
    :param octal: If ``True``, convert to octal values, otherwise
        hexadecimal. Defaults to ``False``.
    :type octal: bool
    :return: The escaped string.
    :rtype: str
    """
    assert isinstance(value, str)
    return openmediavault.string.escape_blank(value, octal)


@jinja_test('match_netif_type')
def match_netif_type(name, type_):
    """
    Test a network interface name if it matches the given type.
    :param name: The interface name to test.
    :type name: str
    :param type_: The interface type, e.g. 'ethernet', 'wifi', 'bond',
      'bridge' or 'vlan'.
    :type type_: str
    :return: Returns ``True`` if the network interface name matches the
      specified type.
    :rtype: bool
    """
    result = False
    if type_ == 'ethernet':
        result = openmediavault.net.is_ethernet(name)
    elif type_ == 'wifi':
        result = openmediavault.net.is_wifi(name)
    elif type_ == 'bond':
        result = openmediavault.net.is_bond(name)
    elif type_ == 'bridge':
        result = openmediavault.net.is_bridge(name)
    elif type_ == 'vlan':
        result = openmediavault.net.is_vlan(name)
    return result
