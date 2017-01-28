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
__all__ = [ "DotDict" ]

class DotDict(dict):
	def __init__(self, value=None):
		if value is None:
			return
		if not isinstance(value, dict):
			raise TypeError("Expected dictionary.")
		for key in value:
			self.__setitem__(key, value[key])

	def setdefault(self, key, default):
		if not key in self:
			self[key] = default
		return self[key]

	def get(self, key, default=None):
		if DotDict.__contains__(self, key):
			return DotDict.__getitem__(self, key)
		return default

	def __getitem__(self, key):
		if key is None or "." not in key:
			return dict.__getitem__(self, key)
		first, rest = key.split(".", 1)
		branch = dict.__getitem__(self, first)
		if not isinstance(branch, DotDict):
			raise KeyError("Can't get '%s' in '%s' (%s)" %
				(rest, first, str(branch)))
		return branch[rest]

	__getattr__ = __getitem__

	def __setitem__(self, key, value):
		if not key is None and "." in key:
			first, rest = key.split(".", 1)
			branch = self.setdefault(first, DotDict())
			if not isinstance(branch, DotDict):
				branch = DotDict()
			branch[rest] = value
		else:
			if isinstance(value, dict) and not isinstance(value, DotDict):
				value = DotDict(value)
			dict.__setitem__(self, key, value)

	__setattr__ = __setitem__

	def __contains__(self, key):
		if key is None or not "." in key:
			return dict.__contains__(self, key)
		first, rest = key.split(".", 1)
		if not dict.__contains__(self, first):
			return False
		branch = dict.__getitem__(self, first)
		if not isinstance(branch, DotDict):
			return False
		return rest in branch
