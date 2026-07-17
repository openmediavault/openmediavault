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
import sys

import dialog
import openmediavault.config
import openmediavault.firstaid


class Module(openmediavault.firstaid.IModule):
    @property
    def description(self):
        return "Disable Workbench administrator's 2FA Email OTP"

    def execute(self):
        d = dialog.Dialog(dialog="dialog")
        code = d.yesno(
            "Do you really want to disable the Workbench administrator's 2FA Email OTP?",
            backtitle=self.description,
            height=6,
            width=47,
            defaultno=True,
        )
        if code != d.OK:
            return 0
        print("Disabling Workbench administrator's 2FA Email OTP. Please wait ...")
        db = openmediavault.config.Database()
        objs = db.get_by_filter(
            "conf.system.tfaemailotp",
            openmediavault.config.DatabaseFilter(
                {
                    'operator': 'stringEquals',
                    'arg0': 'name',
                    'arg1': openmediavault.getenv(
                        "OMV_WEBGUI_ADMINUSER_NAME", "admin"
                    )
                }
            ),
            max_result=1,
        )
        if len(objs) == 0:
            print("The Workbench administrator has not enabled 2FA Email OTP.")
            return 0
        obj = objs[0]
        obj.set("enabled", False)
        db.set(obj)
        print(
            "The Workbench administrator's 2FA Email OTP has been successfully disabled."
        )
        return 0


if __name__ == "__main__":
    module = Module()
    sys.exit(module.execute())
