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
import openmediavault.config.object


class Command(
    openmediavault.confdbadm.ICommand, openmediavault.confdbadm.CommandHelper
):
    @property
    def description(self):
        return "Update a configuration object."

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
            "data",
            type=self.argparse_is_json_stdin,
            help="The JSON data. Set to '-' to read from STDIN.",
        )
        cmd_args = parser.parse_args(args[2:])
        # Create the configuration object.
        obj = openmediavault.config.Object(cmd_args.id)
        obj.set_dict(cmd_args.data)
        # Put the configuration object.
        db = openmediavault.config.Database()
        db.set(obj)
        return rc


if __name__ == "__main__":
    sys.argv.insert(1, "update")
    command = Command()
    rc = command.execute(*sys.argv)
    sys.exit(rc)
