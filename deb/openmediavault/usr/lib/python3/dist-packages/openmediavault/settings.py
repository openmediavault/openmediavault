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
__all__ = ["Environment", "EnvironmentVariable"]

import re
from typing import Dict, Union

import natsort
import openmediavault.utils

DEFAULT_FILE = "/etc/default/openmediavault"


EnvironmentVariableValue = Union[bool, float, int, str]


class EnvironmentVariable:
    __slots__ = ('key', 'value')

    def __init__(self, key: str, value: EnvironmentVariableValue):
        self.key = key
        self.value = value

    def __bool__(self):
        return openmediavault.utils.to_bool(self.value)

    def __str__(self):
        return str(self.value)

    def __int__(self):
        return int(self.value)

    def __float__(self):
        return float(self.value)


class Environment:
    """
    Access the environment variables defined within the file
    /etc/default/openmediavault.
    """

    _env_vars = Dict[str, EnvironmentVariable]

    @staticmethod
    def load():
        Environment.clear()
        try:
            with open(DEFAULT_FILE) as fd:
                for line in fd.readlines():
                    m = re.match(
                        r"^(OMV_([A-Z0-9_]+))=(\")?([^\"]+)(\")?$", line.strip()
                    )
                    if not m:
                        continue
                    # Append variable, e.g. SCRIPTS_DIR
                    Environment.set(m.group(2), m.group(4))
                    # Append variable, e.g. OMV_SCRIPTS_DIR (equal to PHP OMV framework)
                    Environment.set(m.group(1), m.group(4))
        except (IOError, FileNotFoundError):
            pass

    @staticmethod
    def save():
        with open(DEFAULT_FILE, 'w') as fd:
            for key, value in natsort.humansorted(
                Environment.as_dict().items()
            ):
                if key.startswith('OMV_'):
                    fd.write('{}="{}"\n'.format(key, value))

    @staticmethod
    def clear():
        Environment._env_vars = {}

    @staticmethod
    def as_dict() -> Dict[str, EnvironmentVariableValue]:
        """
        Get the environment variables as key/value dictionary where key
        is a string and the value a boolean, float, integer or string.
        :returns: Returns the environment variables as Python dictionary.
        """
        return {
            key: env_var.value
            for (key, env_var) in Environment._env_vars.items()
        }

    @staticmethod
    def _get(key: str, default=None) -> EnvironmentVariable:
        value = Environment._env_vars.get(key)
        # An environment variable can not contain `None`.
        if value is None and default is None:
            raise KeyError(
                "The environment variable '%s' does not exist in '%s'"
                % (key, DEFAULT_FILE)
            )
        return EnvironmentVariable(key, default) if value is None else value

    @staticmethod
    def get(key: str, default=None) -> EnvironmentVariableValue:
        env_var = Environment._get(key, default)
        return env_var.value

    @staticmethod
    def get_str(key: str, default=None) -> str:
        env_var = Environment._get(key, default)
        return str(env_var)

    @staticmethod
    def get_bool(key: str, default=None) -> bool:
        env_var = Environment._get(key, default)
        return bool(env_var)

    @staticmethod
    def get_int(key: str, default=None) -> int:
        env_var = Environment._get(key, default)
        return int(env_var)

    @staticmethod
    def get_float(key: str, default=None) -> float:
        env_var = Environment._get(key, default)
        return float(env_var)

    @staticmethod
    def set(key: str, value: EnvironmentVariableValue) -> EnvironmentVariableValue:
        """
        Set a new key/value pair.
        :returns: Returns the old value if existing, otherwise None.
        """
        env_var = Environment._env_vars.get(key)
        Environment._env_vars[key] = EnvironmentVariable(key, value)
        return env_var.value if env_var else None

    @staticmethod
    def unset(key: str) -> EnvironmentVariableValue:
        env_var = Environment._env_vars.pop(key, None)
        return env_var.value if env_var else None


# Auto-import environment variables from '/etc/default/openmediavault'.
Environment.load()
