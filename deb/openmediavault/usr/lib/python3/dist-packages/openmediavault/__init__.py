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
from .confdbadm import *
from .firstaid import *
from .environment import *
from .exceptions import *
from .productinfo import *
from .log import *
from .rpc import *
from .systemd import *
from .string import *
from .subprocess import *
from .collections import *

from openmediavault.json import *
from openmediavault.datamodel import *
from openmediavault.config import *

import re

def bool(x):
	"""
	Get the boolean value of a variable. A boolean TRUE will be returned for
    the values 1, '1', 'on', 'yes', 'y' and 'true'.
	"""
	result = False
	# Boolean 'true' => '1'
	if str(x).lower() in [ "1", "on", "yes", "y", "true" ]:
		result = True
	return result

def getenv(key, default=None):
	"""
	Get an environment variable, return None if it doesn't exist.
    The optional second argument can specify an alternate default.
    key, default and the result are string.
	"""
	return Environment.get_str(key, default)
