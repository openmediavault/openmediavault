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
import os
import sys

import openmediavault.confdbadm
import openmediavault.procutils

import openmediavault


class Command(openmediavault.confdbadm.ICommand):
    @property
    def description(self):
        return "Populates the database."

    def execute(self, *args):
        # Execute all scripts located in the directory.
        scripts_dir = openmediavault.getenv(
            "OMV_CONFDB_POPULATE_DIR",
            "/usr/share/openmediavault/confdb/populate.d",
        )
        for script_name in os.listdir(scripts_dir):
            if script_name in ['README.md']:
                continue
            # Make sure the script is executable.
            script_path = os.path.join(scripts_dir, script_name)
            if not os.access(script_path, os.X_OK):
                raise RuntimeError(
                    "The script '%s' is not executable" % script_name
                )
            openmediavault.procutils.check_call([script_path])
        return 0


if __name__ == "__main__":
    sys.argv.insert(1, "populate")
    command = Command()
    rc = command.execute(*sys.argv)
    sys.exit(rc)
