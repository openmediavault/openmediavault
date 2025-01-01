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
import glob
import subprocess
import sys

import dialog
import natsort
import openmediavault.firstaid
import openmediavault.procutils

import openmediavault


class Module(openmediavault.firstaid.IModule):
    @property
    def description(self):
        return "Restore configuration backup"

    def execute(self):
        d = dialog.Dialog(dialog="dialog")
        # Determine the first revision file which should look like
        # '<filename>.<revision>'.
        pathname = "%s.*" % openmediavault.getenv("OMV_CONFIG_FILE")
        configbaks = natsort.humansorted(glob.glob(pathname))
        # Does a auto-generated configuration backup exist?
        if not configbaks:
            d.msgbox(
                "No configuration backup found!",
                backtitle=self.description,
                height=5,
                width=34,
            )
            return 0
        # Get the latest configuration backup file.
        configbak = configbaks.pop()
        # Only show a diff, if there's a difference.
        rc = openmediavault.procutils.call(  # yapf: disable
            [
                "diff",
                "--brief",
                openmediavault.getenv("OMV_CONFIG_FILE"),
                configbak,
            ],
            stdout=subprocess.PIPE,
        )
        if rc == 0:
            d.msgbox(
                "There's no difference between the configuration "
                "files. Nothing to restore.",
                backtitle=self.description,
                height=6,
                width=58,
            )
            return 0
        # Display the differences?
        code = d.yesno(
            "Do you want to see the differences between the "
            "current configuration and the backup.",
            backtitle=self.description,
            height=6,
            width=46,
        )
        if code == d.ESC:
            return 0
        if code == d.OK:
            output = (
                "===================================================================\n"
                "All lines with '-' will be changed to the lines with '+'\n"
                "===================================================================\n"
            )
            p = openmediavault.procutils.Popen(
                [
                    "diff",
                    "--unified=1",
                    openmediavault.getenv("OMV_CONFIG_FILE"),
                    configbak,
                ],
                stdout=subprocess.PIPE,
                shell=False,
            )  # yapf: disable
            stdout, _ = p.communicate()
            output += stdout.decode()
            d.scrollbox(
                output,
                backtitle=self.description,
                height=18,
                width=72,
                clear=True,
            )
        # Restore configuration backup?
        code = d.yesno(
            "Do you want to restore the configuration backup? "
            "This will overwrite the actual configuration?",
            backtitle=self.description,
            height=6,
            width=57,
            defaultno=True,
        )
        if code != d.OK:
            return 0
        openmediavault.rpc.call(
            "Config", "revertChanges", {"filename": configbak}
        )
        print("Configuration backup successfully restored.")
        return 0


if __name__ == "__main__":
    module = Module()
    sys.exit(module.execute())
