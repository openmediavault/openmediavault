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
__all__ = [
	"camelcase_to_underscore",
	"truncate",
	"is_json"
]

import re
import json

def camelcase_to_underscore(str):
	return re.sub("(((?<=[a-z])[A-Z])|([A-Z](?![A-Z]|$)))",
		"_\\1", str).lower().strip("_")

def truncate(str, max_len):
	return str[:max_len] + (str[max_len:] and "...")

def is_json(str):
	"""
	Finds out whether a string is JSON.
	:param str: The string being evaluated.
	:returns: True if the string is JSON, otherwise False.
	"""
	try:
		json_object = json.loads(str)
	except ValueError:
		return False
	return True
