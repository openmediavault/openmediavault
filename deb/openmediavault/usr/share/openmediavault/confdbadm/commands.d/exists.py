#!/usr/bin/env python3
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2025 Volker Theile
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
import argparse
import os.path
import sys

import openmediavault.confdbadm
import openmediavault.config.database


class Command(
    openmediavault.confdbadm.ICommand, openmediavault.confdbadm.CommandHelper
):
    @property
    def description(self):
        return "Check if configuration object(s) exists."

    def execute(self, *args):
        # Parse the command line arguments.
        parser = argparse.ArgumentParser(
            prog="%s %s" % (os.path.basename(args[0]), args[1]),
            description=self.description,
        )
        parser.add_argument(
            "id",
            type=self.argparse_is_datamodel_id,
            help="The data model ID, e.g. 'conf.system.sharedfolder'",
        )
        parser.add_argument("--filter", nargs="?", type=self.argparse_is_json)
        cmd_args = parser.parse_args(args[2:])
        # Get the filter.
        filter = None
        if cmd_args.filter:
            filter = openmediavault.config.DatabaseFilter(cmd_args.filter)
        # Query the database.
        db = openmediavault.config.Database()
        return 0 if db.exists(cmd_args.id, filter) else 1


if __name__ == "__main__":
    sys.argv.insert(1, "exists")
    command = Command()
    rc = command.execute(*sys.argv)
    sys.exit(rc)
