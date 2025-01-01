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
import json
import os.path
import sys

import openmediavault.confdbadm
import openmediavault.config.database
import openmediavault.config.object


class Command(
    openmediavault.confdbadm.ICommand, openmediavault.confdbadm.CommandHelper
):
    @property
    def description(self):
        return "Read configuration object(s)."

    def execute(self, *args):
        rc = 0
        # Parse the command line arguments.
        parser = argparse.ArgumentParser(
            prog="%s %s" % (os.path.basename(args[0]), args[1]),
            description=self.description,
        )
        parser.add_argument(
            "id",
            type=self.argparse_is_datamodel_id,
            help="The data model ID, e.g. 'conf.service.ssh'",
        )
        parser.add_argument(
            "--prettify",
            action="store_true",
            help="Prettifies the output, by adding spaces and indentation.",
        )
        group1 = parser.add_mutually_exclusive_group()
        group1.add_argument(
            "--defaults", action="store_true", help="Print the default values."
        )
        group2 = group1.add_mutually_exclusive_group()
        group2.add_argument("--uuid", nargs="?", type=self.argparse_is_uuid4)
        group2.add_argument("--filter", nargs="?", type=self.argparse_is_json)
        cmd_args = parser.parse_args(args[2:])
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
        # Prepare the output.
        if isinstance(objs, list):
            data = [obj.get_dict() for obj in objs]
        else:
            if not isinstance(objs, openmediavault.config.Object):
                return 1
            data = objs.get_dict()
        # Print the configuration objects as JSON to STDOUT.
        if cmd_args.prettify:
            print(json.dumps(data, sort_keys=True, indent=4))
        else:
            print(json.dumps(data))
        return rc


if __name__ == "__main__":
    sys.argv.insert(1, "read")
    command = Command()
    rc = command.execute(*sys.argv)
    sys.exit(rc)
