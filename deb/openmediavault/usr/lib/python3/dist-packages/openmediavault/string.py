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
    "camelcase_to_underscore", "truncate", "is_json", "is_uuid4", "is_fs_uuid"
]

import re
import json
import uuid


def camelcase_to_underscore(value):
    return re.sub("(((?<=[a-z])[A-Z])|([A-Z](?![A-Z]|$)))", "_\\1",
                  value).lower().strip("_")


def truncate(value, max_len):
    return value[:max_len] + (value[max_len:] and "...")


def is_json(value):
    """
    Finds out whether a string is JSON.
    :param str: The string being evaluated.
    :returns: True if the string is JSON, otherwise False.
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
    :param var: The variable being evaluated.
    :returns: Returns True if the variable is an UUIDv4, otherwise False.
    """
    try:
        _ = uuid.UUID(value, version=4)
    except Exception:
        return False
    return True


def is_fs_uuid(value):
    """
    Finds out whether a variable is a filesystem UUID, e.g.
    - 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
    - 7A48-BA97 (FAT)
    - 2ED43920D438EC29 (NTFS)
    - 2015-01-13-21-48-46-00 (ISO9660)
    See http://wiki.ubuntuusers.de/UUID
    :param value: The variable being evaluated.
    :returns: Returns True if the variable is a filesystem UUID,
              otherwise False.
    """
    # Check if it is an UUID v4.
    if is_uuid4(value):
        return True
    # Check if it is a NTFS, FAT or ISO9660 filesystem identifier.
    return None != re.match(
        r'^([a-f0-9]{4}-[a-f0-9]{4}|[a-f0-9]{16}|'
        '[0-9]{4}-[0-9]{2}-[0-9]{2}-[0-9]{2}-[0-9]{2}-[0-9]{2}-[0-9]{2})$',
        value,
        flags=re.IGNORECASE
    )
