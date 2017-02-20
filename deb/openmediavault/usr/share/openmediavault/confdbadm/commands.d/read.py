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
import openmediavault.config.database

class Command(openmediavault.confdbadm.ICommand):
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
		group = parser.add_mutually_exclusive_group()
		group.add_argument("--uuid", nargs="?", type=self.argparse_is_uuid4)
		group.add_argument("--filter", nargs="?", type=self.argparse_is_json)
		cmd_args = parser.parse_args(args[1:])
		# Query the database.
		db = openmediavault.config.Database()
		if cmd_args.filter:
			filter = openmediavault.config.DatabaseFilter(
				json.loads(cmd_args.filter))
			objs = db.get_by_filter(cmd_args.id, filter)
		else:
			objs = db.get(cmd_args.id, cmd_args.uuid)
		# Print the configuration objects to STDOUT.
		if isinstance(objs, list):
			for obj in objs:
				print(obj)
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
