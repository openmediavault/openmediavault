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
__all__ = [
    "camelcase_to_underscore",
    "truncate",
    "is_json",
    "is_uuid4",
    "is_fs_uuid",
]

import re
import json
import uuid


def camelcase_to_underscore(value):
    return (
        re.sub("(((?<=[a-z])[A-Z])|([A-Z](?![A-Z]|$)))", "_\\1", value)
        .lower()
        .strip("_")
    )


def truncate(value, max_len):
    return value[:max_len] + (value[max_len:] and "...")


def is_json(value):
    """
    Finds out whether a string is JSON.
    :param value: The string being evaluated.
    :type value: str
    :return: Returns ``True`` if the string is JSON, otherwise ``False``.
    :rtype: bool

    """
    if not isinstance(value, str):
        return False
    try:
        _ = json.loads(value)
    except ValueError:
        return False
    return True


def is_uuid4(value):
    """
    Finds out whether a variable is an UUID v4.
    :param value: The variable being evaluated.
    :type value: str
    :return: Returns ``True`` if the variable is an UUIDv4,
        otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(value, str):
        return False
    try:
        _ = uuid.UUID(value, version=4)
    except ValueError:
        return False
    return True


def is_fs_uuid(value):
    """
    Finds out whether a variable is a filesystem UUID.

    Example:
    - 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
    - 7A48-BA97 (FAT)
    - 2ED43920D438EC29 (NTFS)
    - 2015-01-13-21-48-46-00 (ISO9660)

    See http://wiki.ubuntuusers.de/UUID
    :param value: The variable being evaluated.
    :type value: str
    :return: Returns ``True`` if the variable is a filesystem UUID,
        otherwise ``False``.
    :rtype: bool
    """
    if not isinstance(value, str):
        return False
    # Check if it is an UUID v4.
    if is_uuid4(value):
        return True
    # Check if it is a NTFS, FAT or ISO9660 filesystem identifier.
    return None != re.match(
        r'^([a-f0-9]{4}-[a-f0-9]{4}|[a-f0-9]{16}|'
        '[0-9]{4}-[0-9]{2}-[0-9]{2}-[0-9]{2}-[0-9]{2}-[0-9]{2}-[0-9]{2})$',
        value,
        flags=re.IGNORECASE,
    )


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
    return value.replace(' ', '\\040' if octal else '\\x20')


def unescape_blank(value, octal=False):
    """
    Unescape a string. A hexadecimal (\x20) or octal (\040) blank will
    be replaced by their ASCII representation.

    Example:
    - /srv/dev-disk-by-label-xx\x20yy => /srv/dev-disk-by-label-xx yy
    - /srv/dev-disk-by-label-xx\040yy => /srv/dev-disk-by-label-xx yy

    :param value: The value that will be unescaped.
    :type value: str
    :param octal: If ``True``, convert octal values, otherwise
        hexadecimal. Defaults to ``False``.
    :type octal: bool
    :return: The unescaped string.
    :rtype: str
    """
    return value.replace('\\040' if octal else '\\x20', ' ')


def binary_format(
    value, precision=2, origin_unit='B', max_unit='YiB', return_json=False
):
    """
    Convert a value into the highest possible binary unit.
    :param value: The number to convert (per default this is in Bytes).
    :type value: str|int
    :param precision: Defaults to 2.
    :param origin_unit: Defaults to ``B``.
    :param max_unit: Defaults to ``YiB``.
    :param return_json: Return value and unit as JSON object.
        Defaults to ``False``.
    :return: The converted string value including the unit or dictionary
        with the keys ``value`` and ``unit``.
    :rtype: str
    """
    units = ['B', 'KiB', 'MiB', 'GiB', 'TiB', 'PiB', 'EiB', 'ZiB', 'YiB']
    value = float(value)
    exp = units.index(origin_unit)
    max_exp = units.index(max_unit)

    while value >= 1024 and exp < max_exp:
        exp += 1
        value = value / 1024

    if not return_json:
        result = '{:.{prec}f} {}'.format(value, units[exp], prec=precision)
    else:
        result = {'value': value, 'unit': units[exp]}
    return result


def path_prettify(path):
    """
    Make sure the directory path ends with a slash.
    >>> assert path_prettify('/foo/bar') == '/foo/bar/'
    >>> assert path_prettify('/foo/bar//') == '/foo/bar/'
    :param path: The path to process.
    :type path: str
    :return: Returns the prettified path.
    :rtype: str
    """
    assert isinstance(path, str)
    return '{}/'.format(path.rstrip('/'))


def add_slashes(value):
    """
    Prefix certain characters of a string with '\'.
    :param value: The string to be escaped.
    :type value: str
    :return: Returns a string with backslashes added before characters
        that need to be escaped. These characters are:
        * backslash (\\)
        * single quote (')
        * double quote (")
    :rtype: str
    """
    assert isinstance(value, str)
    for i in ['\\', '\'', '"']:
        value = value.replace(i, '\\{}'.format(i))
    return value
