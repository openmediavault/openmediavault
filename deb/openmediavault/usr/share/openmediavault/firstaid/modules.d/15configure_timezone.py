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
import sys

import dialog
import openmediavault.firstaid
import openmediavault.rpc


class Module(openmediavault.firstaid.IModule):
    @property
    def description(self):
        return "Configure time zone"

    def execute(self):
        d = dialog.Dialog(dialog="dialog")
        # Get the current time settings.
        time_settings = openmediavault.rpc.call(
            "System",
            "getTimeSettings",
        )
        # Get a list of supported time zones.
        timezones = openmediavault.rpc.call(
            "System",
            "getTimeZoneList",
        )
        choices = []
        for idx, timezone in enumerate(timezones):
            choices.append([str(idx + 1), timezone])
        (code, tag) = d.menu(
            "Please select a time zone.",
            backtitle=self.description,
            clear=True,
            height=18,
            width=65,
            menu_height=8,
            choices=choices,
        )
        if code in (d.CANCEL, d.ESC):
            return 0
        time_settings["timezone"] = timezones[int(tag) - 1]
        # Update the configuration.
        print("Updating time zone. Please wait ...")
        openmediavault.rpc.call(
            "System",
            "setTimeSettings",
            time_settings,
        )
        # Apply the configuration changes.
        openmediavault.rpc.call(
            "Config", "applyChanges", {
                "modules": ["timezone"],
                "force": False
            }
        )
        print("The time zone has been changed successfully.")
        return 0


if __name__ == "__main__":
    module = Module()
    sys.exit(module.execute())
