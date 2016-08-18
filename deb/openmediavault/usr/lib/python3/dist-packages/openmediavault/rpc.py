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
import json
import subprocess

class RpcException(Exception):
	def __init__(self, message, code, trace):
		super().__init__(message)
		self._code = code
		self._trace = trace

	@property
	def code(self):
		return self._code

	@property
	def trace(self):
		return self._trace

def call(service, method, params=None):
	args = [ "omv-rpc", service, method ]
	if params:
		# Convert dictionary to JSON string.
		args.append(json.dumps(params))
	# Execute the shell command.
	p = subprocess.Popen(
		args,
		shell=False,
		env=dict(os.environ, LANG='C'),
		stderr=subprocess.PIPE,
		stdout=subprocess.PIPE)
	(stdout, stderr) = p.communicate()
	if p.returncode != 0:
		response = json.loads(stderr.decode())
		raise RpcException(**response["error"])
	stdout = json.loads(stdout.decode())
	return stdout
