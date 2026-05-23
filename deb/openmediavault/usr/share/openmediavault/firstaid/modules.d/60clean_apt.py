#!/usr/bin/env python3
#
# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2026 Volker Theile
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.
import shutil
import sys
from pathlib import Path

import openmediavault.firstaid
import openmediavault.procutils

import openmediavault


class Module(openmediavault.firstaid.IModule):
    @property
    def description(self):
        return "Refresh APT package lists and clean caches"

    def execute(self):
        print("Cleaning the APT cache files ...")
        openmediavault.procutils.check_call(["apt", "clean"])
        print("Removing APT lists...")
        for item in Path("/var/lib/apt/lists").iterdir():
            if item.name == "partial":
                continue
            if item.is_dir():
                shutil.rmtree(item)
                print(f"removed directory '{item}'")
            else:
                item.unlink()
                print(f"removed '{item}'")
        print("Removing lock files ...")
        paths = [
            "/var/lib/apt/lists/lock",
            "/var/cache/apt/archives/lock",
        ]
        for path in paths:
            try:
                openmediavault.procutils.check_call(["rm", "-fv", path])
            except Exception as e:
                print(f"WARNING: could not remove {path}: {e}")
        print("Ensuring partial directory exists ...")
        openmediavault.procutils.check_call(
            ["mkdir", "-p", "/var/lib/apt/lists/partial"]
        )
        print("Refreshing package indexes from configured APT repositories ...")
        openmediavault.procutils.check_call(
            ["apt-get", "update", "--fix-missing"])
        return 0


if __name__ == "__main__":
    module = Module()
    sys.exit(module.execute())
