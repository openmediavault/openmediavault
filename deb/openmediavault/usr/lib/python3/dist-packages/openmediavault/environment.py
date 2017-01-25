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

# Import global variables from '/etc/default/openmediavault'.
__etc_default_dict = {}
with open(DEFAULT_FILE) as reader:
	for line in reader.readlines():
		m = re.match(r"^(OMV_([A-Z0-9_]+))=(\")?([^\"]+)(\")?$",
			line.strip())
		if not m:
			continue
		# Append variable, e.g. SCRIPTS_DIR
		__etc_default_dict[m.group(2)] = m.group(4)
		# !!! DEPRECATED !!!
		# Append variable, e.g. OMV_SCRIPTS_DIR (equal to PHP OMV framework)
		__etc_default_dict[m.group(1)] = m.group(4)
locals().update(__etc_default_dict)

class Environment(object):
	"""
	Access the environment variables defined within the file
	/etc/default/openmediavault.
	"""

	def get(key, default=None):
		globals_dict = globals()
		if key in globals_dict:
			return globals_dict[key]
		if default is None:
			raise openmediavault.IllegalArgumentError(
			  "The environment variable '{}' does not exist in '{}'".format(
			  key, DEFAULT_FILE))
		return default

	def get_str(key, default=None):
		value = __class__.get(key, default)
		return str(value)

	def get_bool(key, default=None):
		value = __class__.get(key, default)
		return openmediavault.bool(value)

	def get_int(key, default=None):
		value = __class__.get(key, default)
		return int(value)

	def get_float(key, default=None):
		value = __class__.get(key, default)
		return float(value)

if __name__ == "__main__":
	import unittest

	class EnvironmentTestCase(unittest.TestCase):
		def test_get_bool(self):
			value = Environment.get_str("OMV_DEBUG_SCRIPT")
			self.assertTrue(isinstance(value, str))

		def test_get_bool(self):
			value = Environment.get_bool("OMV_DEBUG_PHP")
			self.assertTrue(isinstance(value, bool))

		def test_get_int(self):
			value = Environment.get_int("OMV_ENGINED_SO_SNDTIMEO")
			self.assertTrue(isinstance(value, int))

		def test_get_float(self):
			value = Environment.get_float("OMV_XXX_YYY_ZZZ")
			self.assertTrue(isinstance(value, float))

	unittest.main()
