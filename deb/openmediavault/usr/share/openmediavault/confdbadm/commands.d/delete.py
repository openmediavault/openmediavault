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
import argparse
import json
import openmediavault
import openmediavault.log

class Command(openmediavault.confdbadm.ICommand,
	openmediavault.confdbadm.CommandHelper):
	@property
	def description(self):
		return "Delete the configuration"

	def validate_args(self, *args):
		if 2 > len(args):
			return False
		return True

	def usage(self, *args):
		print("Usage: %s delete [--uuid=UUID|--filter=FILTER] <id>\n\n" \
			"Delete the specified configuration database object." %
			os.path.basename(args[0]))

	def execute(self, *args):
		rc = 0
		# Parse the command line arguments.
		parser = argparse.ArgumentParser()
		parser.add_argument("id")
		group = parser.add_mutually_exclusive_group()
		group.add_argument("--uuid", nargs="?")
		group.add_argument("--filter", nargs="?")
		cmd_args = parser.parse_args(args[1:])
		# Create a backup of the configuration database.
		self.mkBackup()
		# Get the database.
		db = openmediavault.config.Database()
		try:
			if cmd_args.filter:
				# Create the query filter.
				filter = openmediavault.config.DatabaseFilter(json.loads(
					cmd_args.filter))
				objs = db.delete_by_filter(cmd_args.id, filter)
			else:
				# Query the database.
				objs = db.get(cmd_args.id, cmd_args.uuid)
				if not isinstance(objs, list):
					if objs is None:
						objs = []
					else:
						objs = [ objs ]
				# Delete the configuration object(s).
				for obj in objs:
					db.delete(obj)
		except Exception as e:
			rc = 1
			# Display the exception message.
			openmediavault.log.error("Failed to delete the " \
				"configuration object: %s" % str(e))
			# Rollback all changes.
			self.rollbackChanges()
		finally:
			# Unlink the configuration database backup.
			self.unlinkBackup()
			# Output the number of deleted configuration objects.
			print("Deleted %d object(s)" % len(objs))
		return rc

if __name__ == "__main__":
	rc = 1
	command = Command();
	if not command.validate_args(*sys.argv):
		command.usage(*sys.argv)
	else:
		rc = command.execute(*sys.argv)
	sys.exit(rc)
