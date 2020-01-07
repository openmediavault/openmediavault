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
__all__ = ["bool", "getenv", "setenv"]

import openmediavault.settings


def bool(value):  # pylint: disable=redefined-builtin
    """
    Get the boolean value of a variable. A boolean True will be returned for
    the values 1, '1', 'on', 'yes', 'y' and 'true'.
    >>> assert bool(True) == True
    >>> assert bool("1") == True
    >>> assert bool("true") == True
    >>> assert bool("no") == False
    >>> assert bool("False") == False
    >>> assert bool(False) == False
    """
    if (
        type(value) == bool
    ):  # pylint: disable=unidiomatic-typecheck,comparison-with-callable
        return value
    if str(value).lower() in ["1", "on", "yes", "y", "true", "t"]:
        return True
    return False


def getenv(key, default=None, type="str"):  # pylint: disable=redefined-builtin
    """
    Get an environment variable, return None if it doesn't exist.
    :param key:     The name of the variable.
    :param default: The optional second argument can specify an alternate
                    default. Defaults to None.
    :param type:    The type in which the result value is converted.
                    Defaults to 'str'.
    :returns:       Returns the value of the requested environment variable.
    """
    value = openmediavault.settings.Environment.get_str(key, default)
    if "str" == type:
        pass
    elif "int" == type:
        value = int(value)
    elif "float" == type:
        value = float(value)
    elif "bool" == type:
        value = openmediavault.bool(value)
    else:
        raise TypeError("Converting to '%s' is not supported." % type)
    return value


def setenv(key, value):
    """
    Set an environment variable.
    :param key:		The name of the variable.
    :param value:	The value to set.
    :returns:		Returns the old value if it exists, otherwise None.
    """
    return openmediavault.settings.Environment.set(key, value)
