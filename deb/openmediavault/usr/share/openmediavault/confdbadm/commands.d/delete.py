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

import openmediavault.log

import openmediavault


class Command(
    openmediavault.confdbadm.ICommand, openmediavault.confdbadm.CommandHelper
):
    @property
    def description(self):
        return "Delete a configuration object."

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
        group = parser.add_mutually_exclusive_group()
        group.add_argument("--uuid", nargs="?", type=self.argparse_is_uuid4)
        group.add_argument("--filter", nargs="?", type=self.argparse_is_json)
        cmd_args = parser.parse_args(args[2:])
        # Create a backup of the configuration database.
        self.create_backup()
        # Get the database.
        db = openmediavault.config.Database()
        try:
            if cmd_args.filter:
                # Create the query filter.
                db_filter = openmediavault.config.DatabaseFilter(
                    cmd_args.filter
                )
                objs = db.delete_by_filter(cmd_args.id, db_filter)
            else:
                # Query the database.
                objs = db.get(cmd_args.id, cmd_args.uuid)
                if not isinstance(objs, list):
                    if objs is None:
                        objs = []
                    else:
                        objs = [objs]
                # Delete the configuration object(s).
                for obj in objs:
                    db.delete(obj)
        except Exception as e:
            rc = 1
            # Display the exception message.
            openmediavault.log.error(
                "Failed to delete the configuration object: %s", str(e)
            )
            # Rollback all changes.
            self.rollback_changes()
        finally:
            # Unlink the configuration database backup.
            self.unlink_backup()
            # Output the number of deleted configuration objects.
            if 0 == rc:
                print("Deleted %d object(s)" % len(objs))
        return rc


if __name__ == "__main__":
    sys.argv.insert(1, "delete")
    command = Command()
    rc = command.execute(*sys.argv)
    sys.exit(rc)
