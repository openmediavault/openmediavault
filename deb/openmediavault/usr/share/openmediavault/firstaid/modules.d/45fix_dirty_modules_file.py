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
import json
import os
import sys

import openmediavault.firstaid

import openmediavault


class Module(openmediavault.firstaid.IModule):
    @property
    def description(self):
        return "Check configuration status file"

    def execute(self):
        print("Checking configuration status file. Please wait ...")
        path = openmediavault.getenv(
            "OMV_ENGINED_DIRTY_MODULES_FILE",
            "/var/lib/openmediavault/dirtymodules.json",
        )
        if not os.path.exists(path):
            print("No configuration changes file found.")
            return 0
        # Try to load the JSON file. If it fails, then remove it because
        # in this case the file content is no valid JSON string and it
        # is not possible to restore it.
        try:
            with open(path) as json_file:
                json.load(json_file)
            print("The configuration status file is valid.")
        except Exception:
            print("Removing invalid configuration status file.")
            os.unlink(json_file)
        return 0


if __name__ == "__main__":
    module = Module()
    sys.exit(module.execute())
