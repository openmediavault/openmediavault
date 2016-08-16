#!/usr/bin/env python3
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
import sys
import subprocess
import openmediavault as omv

class Module:
	def get_description(self):
		return "Clear local upload package repository"

	def execute(self):
		try:
			print("Clear out the local repository of uploaded package " \
				"files. Please wait ...")
			path = omv.getenv("OMV_DPKGARCHIVE_DIR")
			subprocess.call([ "rm", "-fv", "{}/*.deb".format(path) ])
			subprocess.call("cd {} && apt-ftparchive packages . > Packages".format(
				path), shell=True)
			subprocess.call([ "apt-get", "update" ])
		except Exception as e:
			omv.log.error(str(e))
			return 1
		return 0

if __name__ == "__main__":
	module = Module();
	sys.exit(module.execute())
