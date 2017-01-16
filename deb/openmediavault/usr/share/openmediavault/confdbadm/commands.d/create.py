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
import sys
import openmediavault as omv

class Command(omv.confdbadm.ICommand, omv.confdbadm.CommandHelper):
	@property
	def description(self):
		return "Create the default configuration per plugin"

	def validate_args(self, *args):
		if 2 != len(args):
			return False
		return True

	def usage(self, *args):
		print("Usage: %s create <plugin>\n\nCreate the default " \
			"configuration for the specified plugin." %
			os.path.basename(args[0]))

	def execute(self, *args):
		rc = 1
		plugin = args[1]
		create_dir = omv.getenv("OMV_CONFDB_CREATE_DIR",
			"/usr/share/openmediavault/confdb/create.d");
		script_name = ""
		for name in os.listdir(create_dir):
			# Split the script name into its parts:
			# <PLUGINNAME>.<EXT>
			if plugin == os.path.splitext(name)[0]:
				script_name = os.path.join(create_dir, name)
				break
		try:
			# Create a backup of the configuration database.
			self.mkBackup()
			# Test if the script exists and is executable.
			script_path = os.path.join(create_dir, script_name)
			if not os.exists(script_path):
				raise RuntimeError("The script '%s' does not exist" %
					script_name)
			if not os.access(script_path, os.X_OK):
				raise RuntimeError("The script '%s' is not " \
					"executable" % script_name)
			# Execute the script.
			omv.subprocess.check_call([ script_path ])
			print("Done")
			rc = 0
		except Exception as e:
			# Display the exception message.
			omv.log.error("Failed to create the default configuration: %s" %
				str(e))
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
