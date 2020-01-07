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
__all__ = ["ICommand"]

import abc
import argparse
import os
import shutil
import json
import re
import sys
import tempfile

import openmediavault
import openmediavault.config.datamodel
import openmediavault.string


class ICommand(metaclass=abc.ABCMeta):  # lgtm[py/syntax-error]
    @property
    @abc.abstractmethod
    def description(self):
        """
        Get the module description.
        """

    @abc.abstractmethod
    def execute(self, *args):
        """
        Execute the command.
        :param args: The command arguments.
        :returns: Returns the return code.
        """


class CommandHelper:
    _backup_path = None

    def mkBackup(self):  # pylint: disable=invalid-name
        """
        .. deprecated:: 5.0
        """
        return self.create_backup()

    def create_backup(self):
        """
        Create a backup of the configuration database.
        :returns: Returns the path of the backup file, otherwise None.
        """
        config_path = openmediavault.getenv("OMV_CONFIG_FILE")
        if not os.path.exists(config_path):
            self._backup_path = False
            return None
        (fh, self._backup_path) = tempfile.mkstemp()
        shutil.copy(config_path, self._backup_path)
        return self._backup_path

    def unlinkBackup(self):  # pylint: disable=invalid-name
        """
        .. deprecated:: 5.0
        """
        return self.unlink_backup()

    def unlink_backup(self):
        """
        Unlink the backup of the configuration database.
        """
        if self._backup_path is None:
            raise RuntimeError("No configuration backup exists.")
        if not self._backup_path:
            return
        os.unlink(self._backup_path)
        self._backup_path = None

    def rollbackChanges(self):  # pylint: disable=invalid-name
        """
        .. deprecated:: 5.0
        """
        return self.rollback_changes()

    def rollback_changes(self):
        """
        Rollback all changes in the configuration database.
        """
        if self._backup_path is None:
            raise RuntimeError("No configuration backup exists.")
        if not self._backup_path:
            return
        shutil.copy(self._backup_path, openmediavault.getenv("OMV_CONFIG_FILE"))

    def argparse_is_uuid4(self, arg):
        """
        Check if the specified value is a valid UUID4.
        :param arg:	The value to check.
        :returns: The specified value.
        :raises argparse.ArgumentTypeError:
        """
        if not openmediavault.string.is_uuid4(arg):
            raise argparse.ArgumentTypeError("No valid UUID4.")
        return arg

    def argparse_is_json(self, arg):
        """
        Check if the specified value is a valid JSON string.
        :param arg:	The value to check.
        :returns: The specified value as Python dictionary.
        :raises argparse.ArgumentTypeError:
        """
        if not openmediavault.string.is_json(arg):
            raise argparse.ArgumentTypeError("No valid JSON.")
        return json.loads(arg)

    def argparse_is_json_stdin(self, arg):
        """
        Check if the specified value is a valid JSON string. Loads the
        data from STDIN if '-' is given.
        :param arg:	The value to check.
        :returns: The specified value as Python dictionary.
        :raises argparse.ArgumentTypeError:
        """
        if arg == "-":
            arg = sys.stdin.read()
        return self.argparse_is_json(arg)

    def argparse_is_datamodel_id(self, arg):
        """
        Check if the specified value is a valid datamodel ID.
        Example: conf.service.ftp
        :param arg:	The value to check.
        :returns: The specified value.
        :raises argparse.ArgumentTypeError:
        """
        if not re.match(r'^conf(\..+)?$', arg):
            raise argparse.ArgumentTypeError("No valid data model ID.")
        if "conf" == arg:
            return arg
        try:
            openmediavault.config.Datamodel(arg)
        except Exception as e:
            raise argparse.ArgumentTypeError(str(e))
        return arg
