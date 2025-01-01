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
        return "Change Workbench administrator password"

    def execute(self):
        d = dialog.Dialog(dialog="dialog")
        password = password_conf = None
        while not password or (password != password_conf):
            while not password:
                (code, password) = d.passwordbox(
                    "Please enter the new password.",
                    backtitle=self.description,
                    insecure=True,
                    clear=True,
                    height=8,
                    width=34,
                )
                if code != d.OK:
                    return 0
                if not password:
                    d.msgbox(
                        "The password must not be empty.",
                        backtitle=self.description,
                        height=5,
                        width=35,
                    )
            while not password_conf:
                (code, password_conf) = d.passwordbox(
                    "Please confirm the new password.",
                    backtitle=self.description,
                    insecure=True,
                    clear=True,
                    height=8,
                    width=36,
                )
                if code != d.OK:
                    return 0
                if not password_conf:
                    d.msgbox(
                        "The password must not be empty.",
                        backtitle=self.description,
                        height=5,
                        width=35,
                    )
            if password != password_conf:
                password = password_conf = None
                d.msgbox(
                    "The passwords don't match.",
                    backtitle=self.description,
                    height=5,
                    width=30,
                )
        print("Updating Workbench administrator password. Please wait ...")
        openmediavault.rpc.call(
            "UserMgmt", "setPasswordByContext", {"password": password}
        )
        # openmediavault.rpc.call("Config", "applyChanges",
        # 	{ "modules": [], "force": False })
        print(
            "The Workbench administrator password was successfully "
            "changed."
        )
        return 0


if __name__ == "__main__":
    module = Module()
    sys.exit(module.execute())
