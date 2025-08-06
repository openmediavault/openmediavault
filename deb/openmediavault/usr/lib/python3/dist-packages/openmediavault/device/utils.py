# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2025 Volker Theile
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.
import os
import re
import stat


def is_block_device(path) -> bool:
    """
    Check if path is a block device, e.g. /dev/sda1.
    :param path: The path to check.
    :type path: str
    :return: Return ``True`` if path describes a block device,
        otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    try:
        st = os.stat(path)
    except FileNotFoundError:
        return False
    return stat.S_ISBLK(st.st_mode)


def is_char_device(path) -> bool:
    """
    Check if path is a character device, e.g. /dev/sg0.
    :param path: The path to check.
    :type path: str
    :return: Return ``True`` if path describes a character device,
        otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    try:
        st = os.stat(path)
    except FileNotFoundError:
        return False
    return stat.S_ISCHR(st.st_mode)


def is_device_file(path) -> bool:
    """
    Check if path describes a device file, e.g.

    * /dev/sda1
    * /dev/disk/by-label/data
    * /dev/mapper/sda-crypt
    * /dev/dm-2

    Note, only the specified string is evaluated.

    :param path: The path to check.
    :type path: str
    :return: Return ``True`` if path describes a device file,
        otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    return bool(re.match(r'^/dev/.+$', path))


def is_device_file_by(path) -> bool:
    """
    Check if path describes a device file, e.g.

    * /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
    * /dev/disk/by-id/usb-Kingston_DataTraveler_G2_001CC0EC21ADF011C6A20E35-0:0-part1
    * /dev/disk/by-label/data

    Note, only the specified string is evaluated.

    :param path: The path to check.
    :type path: str
    :return: Return ``True`` if path describes a device file,
        otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    return bool(re.match(r'^/dev/disk/by-\S+/.+$', path))


def is_device_file_by_uuid(path) -> bool:
    """
    Check if path describes a device file, e.g.

    * /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
    * /dev/disk/by-uuid/2ED43920D438EC29 (NTFS)

    Note, only the specified string is evaluated.

    :param path: The path to check.
    :type path: str
    :return: Return ``True`` if path describes a device file,
        otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    return bool(re.match(r'^/dev/disk/by-uuid/.+$', path))


def is_device_file_by_id(path) -> bool:
    """
    Check if path describes a device file, e.g.

    * /dev/disk/by-id/usb-Kingston_DataTraveler_G2_001CC0EC21ADF011C6A20E35-0:0-part1
    * /dev/disk/by-id/wwn-0x4002c554a4d79cb9-part2

    Note, only the specified string is evaluated.

    :param path: The path to check.
    :type path: str
    :return: Return ``True`` if path describes a device file,
        otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    return bool(re.match(r'^/dev/disk/by-id/.+$', path))


def is_device_file_by_label(path) -> bool:
    """
    Check if path describes a device file, e.g.

    * /dev/disk/by-label/data

    Note, only the specified string is evaluated.

    :param path: The path to check.
    :type path: str
    :return: Return ``True`` if path describes a device file,
        otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    return bool(re.match(r'^/dev/disk/by-label/.+$', path))


def is_device_file_by_path(path) -> bool:
    """
    Check if path describes a device file, e.g.

    * /dev/disk/by-path/pci-0000:00:17.0-ata-3
    * /dev/disk/by-path/pci-0000:00:17.0-ata-3-part2

    Note, only the specified string is evaluated.

    :param path: The path to check.
    :type path: str
    :return: Return ``True`` if path describes a device file,
        otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(path, str):
        return False
    return bool(re.match(r'^/dev/disk/by-path/.+$', path))
