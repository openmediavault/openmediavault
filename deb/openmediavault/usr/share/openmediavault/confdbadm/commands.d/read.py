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
import openmediavault.confdbadm
import openmediavault.config.object
import openmediavault.config.database

class Command(openmediavault.confdbadm.ICommand,
		openmediavault.confdbadm.CommandHelper):
	@property
	def description(self):
		return "Read configuration database objects"

	def validate_args(self, *args):
		if 2 > len(args):
			return False
		return True

	def usage(self, *args):
		print("Usage: %s read [--uuid=UUID|--filter=FILTER] <id>\n\n" \
			"Read the specified configuration database object." %
			os.path.basename(args[0]))

	def execute(self, *args):
		rc = 0
		# Parse the command line arguments.
		parser = argparse.ArgumentParser()
		parser.add_argument("id")
		group1 = parser.add_mutually_exclusive_group()
		group1.add_argument("--defaults", action="store_true")
		group2 = group1.add_mutually_exclusive_group()
		group2.add_argument("--uuid", nargs="?", type=self.argparse_is_uuid4)
		group2.add_argument("--filter", nargs="?", type=self.argparse_is_json)
		cmd_args = parser.parse_args(args[1:])
		# Get the configuration object with its default values?
		if cmd_args.defaults:
			objs = openmediavault.config.Object(cmd_args.id)
		else:
			# Query the database.
			db = openmediavault.config.Database()
			if cmd_args.filter:
				filter = openmediavault.config.DatabaseFilter(cmd_args.filter)
				objs = db.get_by_filter(cmd_args.id, filter)
			else:
				objs = db.get(cmd_args.id, cmd_args.uuid)
		# Print the configuration objects to STDOUT.
		if isinstance(objs, list):
			# Print list of objects as JSON string.
			data = []
			for obj in objs:
				data.append(obj.get_dict())
			print(json.dumps(data))
		else:
			print(objs)
		return rc

if __name__ == "__main__":
	rc = 1
	command = Command();
	if not command.validate_args(*sys.argv):
		command.usage(*sys.argv)
	else:
		rc = command.execute(*sys.argv)
	sys.exit(rc)
