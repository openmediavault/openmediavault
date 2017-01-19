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

class Command(omv.confdbadm.ICommand):
	@property
	def description(self):
		return "Update a configuration database object"

	def validate_args(self, *args):
		if 4 != len(args):
			return False
		return True

	def usage(self, *args):
		print("Usage: %s update <id> <uuid> <data>\n\n" \
			"Update the specidied configuration database object." %
			os.path.basename(args[0]))

	def execute(self, *args):
		rc = 1
		print("Not implemented yet.")
		return rc

if __name__ == "__main__":
	rc = 1
	command = Command();
	if not command.validate_args(*sys.argv):
		command.usage(*sys.argv)
	else:
		rc = command.execute(*sys.argv)
	sys.exit(rc)
