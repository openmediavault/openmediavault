# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2017 Volker Theile
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
__all__ = [ "ICommand" ]

import abc
import os
import argparse
import shutil
import json
import tempfile
import openmediavault
import openmediavault.string

class ICommand(metaclass=abc.ABCMeta):
	@abc.abstractproperty
	def description(self):
		"""
		Get the module description.
		"""

	@abc.abstractmethod
	def validate_args(self, *args):
		"""
		Validate the command arguments.
		:param args: The command arguments to validate.
		:returns: Returns True if the arguments are valid, otherwise False.
		"""

	@abc.abstractmethod
	def usage(self, *args):
		"""
		Display the command help.
		:param args: The command arguments.
		"""

	@abc.abstractmethod
	def execute(self, *args):
		"""
		Execute the command.
		:param args: The command arguments.
		:returns: Returns the return code.
		"""

class CommandHelper():
	_backup_path = None

	def mkBackup(self):
		"""
		Create a backup of the configuration database.
		:returns: Returns the path of the backup file, otherwise None.
		"""
		config_path = openmediavault.getenv("OMV_CONFIG_FILE")
		if not os.path.exists(config_path):
			self._backup_path = False
			return None
		(fh, self._backup_path) = tempfile.mkstemp();
		shutil.copy(config_path, self._backup_path)
		return self._backup_path

	def unlinkBackup(self):
		"""
		Unlink the backup of the configuration database.
		"""
		if self._backup_path is None:
			raise RuntimeError("No configuration backup exists")
		if not self._backup_path:
			return
		os.unlink(self._backup_path)
		self._backup_path = None

	def rollbackChanges(self):
		"""
		Rollback all changes in the configuration database.
		"""
		if self._backup_path is None:
			raise RuntimeError("No configuration backup exists")
		if not self._backup_path:
			return
		shutil.copy(self._backup_path, openmediavault.getenv(
			"OMV_CONFIG_FILE"))

	def argparse_is_uuid4(self, arg):
		"""
		Check if the specified value is a valid UUID4.
		:param arg:	The value to check.
		:returns:	The specified value.
		"""
		if not openmediavault.string.is_uuid4(arg):
			raise argparse.ArgumentTypeError("Value is no valid UUID4")
		return arg

	def argparse_is_json(self, arg):
		"""
		Check if the specified value is a valid JSON string.
		:param arg:	The value to check.
		:returns:	The specified value as Python dictionary.
		"""
		if not openmediavault.string.is_json(arg):
			raise argparse.ArgumentTypeError("Value is no valid JSON")
		return json.loads(arg)
