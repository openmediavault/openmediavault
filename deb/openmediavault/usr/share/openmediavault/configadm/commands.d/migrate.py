#!/usr/bin/env python3
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
import os
import re
import sys
from distutils.version import LooseVersion
import openmediavault as omv

class Command(omv.configadm.ICommand):
	@property
	def description(self):
		return "Apply database migrations"

	def usage(self):
		print("Usage: ... migrate <plugin> <version>\n\n" +
			"Migrate the configuration database of the specified plugin.")

	def execute(self, *args):
		# Check the command line arguments.
		if 2 != len(args):
			self.usage()
			return 1
		plugin = args[0]
		version = args[1]
		migrations_dir = omv.getenv("OMV_CONFIGADM_MIGRATE_MIGRATIONS_DIR",
			"/usr/share/openmediavault/dbmigrations");
		# Collect the migrations to be executed and sort them.
		# !!! Note, we can not use natsort.humansorted here because it
		# does not sort the list in the correct order, e.g.
		# openmediavault_3.0.11
		# openmediavault_3.0.12
		# openmediavault_3.0.23
		# openmediavault_3.0.38
		# openmediavault_3.0.9
		versions = []
		for name in os.listdir(migrations_dir):
			# Split the script name into its parts:
			# <PLUGINNAME>_<VERSION>
			parts = re.split(r'_', name)
			if plugin != parts[0]:
				continue
			if LooseVersion(parts[1]) < LooseVersion(version):
				continue
			versions.append(parts[1])
		versions = sorted(versions, key=lambda version: LooseVersion(version))
		# Execute the configuration database migration scripts.
		print("Applying migrations ...")
		for version in versions:
			name = "%s_%s" % (plugin, version)
			path = os.path.join(migrations_dir, name)
			# Test if the script is executable.
			if not os.access(path, os.X_OK):
				omv.log.error("The script '%s' is not executable", path)
				return 101
			print(" * %s" % name)
			omv.subprocess.check_call([ path ])
		print("Done")
		return 0

if __name__ == "__main__":
	command = Command();
	sys.exit(command.execute(*sys.argv[1:]))
