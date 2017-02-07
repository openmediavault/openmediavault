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
__all__ = [ "Environment" ]

import re
import openmediavault

DEFAULT_FILE = "/etc/default/openmediavault"

class Environment(object):
	_values = {}

	"""
	Access the environment variables defined within the file
	/etc/default/openmediavault.
	"""

	@staticmethod
	def load():
		with open(DEFAULT_FILE) as reader:
			for line in reader.readlines():
				m = re.match(r"^(OMV_([A-Z0-9_]+))=(\")?([^\"]+)(\")?$",
					line.strip())
				if not m:
					continue
				# Append variable, e.g. SCRIPTS_DIR
				Environment._values[m.group(2)] = m.group(4)
				# !!! DEPRECATED !!!
				# Append variable, e.g. OMV_SCRIPTS_DIR (equal to PHP OMV framework)
				Environment._values[m.group(1)] = m.group(4)

	@staticmethod
	def get(key, default=None):
		if key in Environment._values:
			return Environment._values[key]
		if default is None:
			raise KeyError(
				"The environment variable '%s' does not exist in '%s'" %
				(key, DEFAULT_FILE))
		return default

	@staticmethod
	def get_str(key, default=None):
		value = __class__.get(key, default)
		return str(value)

	@staticmethod
	def get_bool(key, default=None):
		value = __class__.get(key, default)
		return openmediavault.bool(value)

	@staticmethod
	def get_int(key, default=None):
		value = __class__.get(key, default)
		return int(value)

	@staticmethod
	def get_float(key, default=None):
		value = __class__.get(key, default)
		return float(value)

	@staticmethod
	def set(key, value):
		result = None
		if key in Environment._values:
			result = Environment._values[key]
		Environment._values[key] = value
		return result

# Auto-import environment variables from '/etc/default/openmediavault'.
Environment.load()
