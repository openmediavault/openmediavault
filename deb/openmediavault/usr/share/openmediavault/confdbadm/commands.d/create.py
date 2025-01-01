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
import os
import sys

import openmediavault.confdbadm
import openmediavault.log
import openmediavault.procutils

import openmediavault


class Command(
    openmediavault.confdbadm.ICommand, openmediavault.confdbadm.CommandHelper
):
    @property
    def description(self):
        return "Create the default configuration for a data model."

    def execute(self, *args):
        rc = 1
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
        cmd_args = parser.parse_args(args[2:])
        # Find the script.
        create_dir = openmediavault.getenv(
            "OMV_CONFDB_CREATE_DIR", "/usr/share/openmediavault/confdb/create.d"
        )
        script_name = ""
        for name in os.listdir(create_dir):
            # Split the script name into its parts:
            # <DATAMODELID>.<EXT>
            if cmd_args.id == os.path.splitext(name)[0]:
                script_name = name
                break
        try:
            # Create a backup of the configuration database.
            self.create_backup()
            # Test if the script exists and is executable.
            script_path = os.path.join(create_dir, script_name)
            if not os.path.exists(script_path):
                raise RuntimeError(
                    "The script '%s' does not exist" % script_name
                )
            if not os.access(script_path, os.X_OK):
                raise RuntimeError(
                    "The script '%s' is not executable" % script_name
                )
            # Execute the script.
            openmediavault.procutils.check_call([script_path])
            rc = 0
        except Exception as e:
            # Display the exception message.
            openmediavault.log.error(
                "Failed to create the default configuration: %s", str(e)
            )
            # Rollback all changes.
            self.rollback_changes()
        finally:
            # Unlink the configuration database backup.
            self.unlink_backup()
        return rc


if __name__ == "__main__":
    sys.argv.insert(1, "create")
    command = Command()
    rc = command.execute(*sys.argv)
    sys.exit(rc)
