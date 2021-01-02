#!/usr/bin/env python3
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2021 Volker Theile
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
import re
import sys

import dialog
import openmediavault.firstaid
import openmediavault.subprocess


class Module(openmediavault.firstaid.IModule):
    @property
    def description(self):
        return "Reset failed login attempt counter"

    def execute(self):
        choices = []
        output = openmediavault.subprocess.check_output(["pam_tally2"])
        for line in output.splitlines():
            m = re.match(r"^(\S+)\s+((\d+)\s+(.+))$", line.decode().strip())
            if not m:
                continue
            choices.append([m.group(1), m.group(2)])
        if not choices:
            print("No locked/banned users or candidates exists.")
            return 0
        d = dialog.Dialog(dialog="dialog")
        (code, tag) = d.menu(
            "Please select a user to reset the failed "
            "login attempt counter.",
            backtitle=self.description,
            clear=True,
            height=13,
            width=68,
            menu_height=5,
            choices=choices,
        )
        if code in (d.CANCEL, d.ESC):
            return 0
        username = tag
        print(
            "Reset failed login attempt counter for user '{}'.".format(
                username)
        )
        openmediavault.subprocess.check_call(
            ["pam_tally2", "--quiet", "--reset=0", "--user", username]
        )
        return 0


if __name__ == "__main__":
    module = Module()
    sys.exit(module.execute())
