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
import os.path
import re
import sys

import openmediavault.confdbadm
import openmediavault.log
import openmediavault.procutils
import packaging.version

import openmediavault


class Command(
    openmediavault.confdbadm.ICommand, openmediavault.confdbadm.CommandHelper
):
    @property
    def description(self):
        return "Apply configuration migrations."

    def argparse_is_version(self, arg):
        try:
            _ = packaging.version.Version(arg)
        except packaging.version.InvalidVersion:
            raise argparse.ArgumentTypeError("No valid version")

        # Extract the upstream version.
        # [epoch:]upstream_version[-debian_revision]
        # https://www.debian.org/doc/debian-policy/ch-controlfields.html#version
        # Note, the regex for the upstream version is simplified.
        parts = re.match(
            r'^(\d:)?([^:-]+)(-[a-z0-9\+\.~]+)?$', arg, flags=re.IGNORECASE
        )
        return parts.group(2)

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
        parser.add_argument("version", type=self.argparse_is_version)
        cmd_args = parser.parse_args(args[2:])
        # Get the migrations.
        migrations = {}
        migrations_dir = openmediavault.getenv(
            "OMV_CONFDB_MIGRATIONS_DIR",
            "/usr/share/openmediavault/confdb/migrations.d",
        )
        # Collect the migrations to be executed.
        for name in os.listdir(migrations_dir):
            # Split the script name into its parts:
            # <DATAMODELID>_<VERSION>.<EXT>
            parts = re.split(r'_', os.path.splitext(name)[0])
            if 2 != len(parts):
                continue
            if cmd_args.id != parts[0]:
                continue
            if packaging.version.Version(parts[1]) < packaging.version.Version(cmd_args.version):
                continue
            migrations[parts[1]] = name
        try:
            # Create a backup of the configuration database.
            self.create_backup()
            # Execute the configuration database migration scripts.
            for cmd_args.version in sorted(
                migrations, key=lambda v: packaging.version.Version(v)
            ):
                name = "%s_%s" % (cmd_args.id, cmd_args.version)
                path = os.path.join(
                    migrations_dir, migrations[cmd_args.version]
                )
                # Test if the script is executable.
                if not os.access(path, os.X_OK):
                    raise RuntimeError(
                        "The script '%s' is not executable" % path
                    )
                # Execute the script.
                print("  Running migration %s" % name)
                openmediavault.procutils.check_call([path])
            rc = 0
        except Exception as e:
            # Display the exception message.
            openmediavault.log.error("Failed to apply migrations: %s", str(e))
            # Rollback all changes.
            self.rollback_changes()
        finally:
            # Unlink the configuration database backup.
            self.unlink_backup()
        return rc


if __name__ == "__main__":
    sys.argv.insert(1, "migrate")
    command = Command()
    rc = command.execute(*sys.argv)
    sys.exit(rc)
