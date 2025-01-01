# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
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
# along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
__all__ = ["to_bool"]

from typing import Any


def to_bool(value: Any) -> bool:
    """
    Get the boolean value of a variable. A boolean True will be returned for
    the values 1, '1', 'on', 'yes', 'y' and 'true'.
    >>> assert to_bool(True) == True
    >>> assert to_bool("1") == True
    >>> assert to_bool("true") == True
    >>> assert to_bool("no") == False
    >>> assert to_bool("False") == False
    >>> assert to_bool(False) == False
    """
    if type(value) == bool:
        return value
    if str(value).lower() in ["1", "on", "yes", "y", "true", "t"]:
        return True
    return False
