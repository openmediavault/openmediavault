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
import os
import sys
import subprocess

def shell(args, verbose=False):
	"""
	Execute an external program in the default shell and returns a tuple
	with the command return code and output.

	:param verbose: If True, then the process output will be written to stdout.
		Default is False.
	:type  verbose: bool
	"""
	with subprocess.Popen(args,
		bufsize=1,
		shell=False if isinstance(args, list) else True,
		env=dict(os.environ, LANG='C'),
		stderr=subprocess.PIPE,
		stdout=subprocess.PIPE) as p:
		try:
			stdout = []
			while True:
				if p.poll() is not None:
					break
				stdout_line = p.stdout.readline().decode()
				if stdout_line:
					stdout.append(stdout_line)
					if verbose:
						sys.stdout.write(stdout_line)
			return (p.returncode, "".join(stdout))
		except:
			p.kill()
			p.wait()
			raise
