# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2016 Volker Theile
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
from .environment import Environment
from .exceptions import *
from .productinfo import ProductInfo
from .log import *

import os
import subprocess

__all__ = [ 'Environment', 'IllegalArgumentError', 'ProductInfo' ]

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

def shell(args, redirect_stderr=False):
	"""
	Execute an external program in the default shell and returns a tuple
	with the command return code and output.

	:param redirect_stderr: If True, then stderr will be redirected to stdout.
	:type  redirect_stderr: bool
	"""
	with subprocess.Popen(args,
		bufsize=1,
		shell=False if isinstance(args, list) else True,
		env=dict(os.environ, LANG='C'),
		stderr=subprocess.STDOUT if redirect_stderr else None,
		stdout=subprocess.PIPE) as p:
		try:
			output = []
			while True:
				if p.poll() is not None:
					break
				line = p.stdout.readline().decode()
				if line:
					output.append(line)
					sys.stdout.write(line)
			return [ p.returncode, output ]
		except:
			p.kill()
			p.wait()
			raise
