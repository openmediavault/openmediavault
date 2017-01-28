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
import openmediavault
import openmediavault.confdbadm
import openmediavault.log
import openmediavault.subprocess

class Command(openmediavault.confdbadm.ICommand,
	openmediavault.confdbadm.CommandHelper):
	@property
	def description(self):
		return "Apply configuration database migrations"

	def validate_args(self, *args):
		if 3 != len(args):
			return False
		return True

	def usage(self, *args):
		print("Usage: %s migrate <id> <version>\n\n" \
			"Migrate the configuration database for the specified " \
			"datamodel ID." % os.path.basename(args[0]))

	def execute(self, *args):
		rc = 1
		datamodel_id = args[1]
		version = args[2]
		migrations = {}
		migrations_dir = openmediavault.getenv("OMV_CONFDB_MIGRATIONS_DIR",
			"/usr/share/openmediavault/confdb/migrations.d");
		# Collect the migrations to be executed.
		for name in os.listdir(migrations_dir):
			# Split the script name into its parts:
			# <DATAMODELID>_<VERSION>.<EXT>
			parts = re.split(r'_', os.path.splitext(name)[0])
			if 2 != len(parts):
				continue
			if datamodel_id != parts[0]:
				continue
			if LooseVersion(parts[1]) < LooseVersion(version):
				continue
			migrations[parts[1]] = name
		try:
			# Create a backup of the configuration database.
			self.mkBackup()
			# Execute the configuration database migration scripts.
			for version in sorted(migrations, key=lambda v: LooseVersion(v)):
				name = "%s_%s" % (datamodel_id, version)
				path = os.path.join(migrations_dir, migrations[version])
				# Test if the script is executable.
				if not os.access(path, os.X_OK):
					raise RuntimeError("The script '%s' is not " \
						"executable" % path)
				# Execute the script.
				print("  Running migration %s" % name)
				openmediavault.subprocess.check_call([ path ])
			rc = 0
		except Exception as e:
			# Display the exception message.
			openmediavault.log.error("Failed to apply migrations: %s" % str(e))
			# Rollback all changes.
			self.rollbackChanges()
		finally:
			# Unlink the configuration database backup.
			self.unlinkBackup()
		return rc

if __name__ == "__main__":
	rc = 1
	command = Command();
	if not command.validate_args(*sys.argv):
		command.usage(*sys.argv)
	else:
		rc = command.execute(*sys.argv)
	sys.exit(rc)
