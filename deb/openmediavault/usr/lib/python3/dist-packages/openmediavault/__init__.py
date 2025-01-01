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
__all__ = ["bool", "getenv", "setenv"]

from typing import Any, Optional

import openmediavault.settings
import openmediavault.utils


def bool(value):  # pylint: disable=redefined-builtin
    """
    Get the boolean value of a variable. A boolean True will be returned for
    the values 1, '1', 'on', 'yes', 'y' and 'true'.
    @deprecated
    """
    return openmediavault.utils.to_bool(value)


def getenv(key: str, default: Optional[Any] = None, return_type: Optional[str] = "str"):
    """
    Get an environment variable, return None if it doesn't exist.
    :param key: The name of the variable.
    :param default: The optional second argument can specify an alternate
      default. Defaults to None.
    :param return_type: The type in which the result value is converted.
      Defaults to 'str'.
    :returns: Returns the value of the requested environment variable.
    """
    if "str" == return_type:
        return openmediavault.settings.Environment.get_str(key, default)
    elif "int" == return_type:
        return openmediavault.settings.Environment.get_int(key, default)
    elif "float" == return_type:
        return openmediavault.settings.Environment.get_float(key, default)
    elif "bool" == return_type:
        return openmediavault.settings.Environment.get_bool(key, default)
    raise TypeError("Converting to '{}' is not supported.".format(return_type))


def setenv(key: str, value: Any) -> Any:
    """
    Set an environment variable.
    :param key: The name of the variable.
    :param value: The value to set.
    :returns: Returns the old value if it exists, otherwise None.
    """
    return openmediavault.settings.Environment.set(key, value)
