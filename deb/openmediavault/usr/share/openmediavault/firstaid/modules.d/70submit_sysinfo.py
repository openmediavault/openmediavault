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
import shutil
import socket
import sys
import tempfile
import time

import dialog
import openmediavault.firstaid
import openmediavault.procutils
import openmediavault.systemd
import pyudev


class Module(openmediavault.firstaid.IModule):
    @property
    def description(self):
        return "Submit diagnostic report to administrator"

    def execute(self):
        # Check if postfix is running.
        try:
            manager = openmediavault.systemd.Manager()
            unit = manager.get_unit("postfix.service")
            active = unit.active
        except Exception:  # pylint: disable=broad-except
            active = False
        if not active:
            d = dialog.Dialog(dialog="dialog")
            code = d.msgbox(
                "Failed to submit the system diagnostic "
                "report to the administrator account via email because "
                "the email notification service is disabled.",
                backtitle=self.description,
                height=7,
                width=56,
            )
            if code != d.OK:
                return 0
            code = d.yesno(
                "Do you want to copy the system diagnostic "
                "report onto an USB device?",
                backtitle=self.description,
                height=6,
                width=45,
            )
            if code != d.OK:
                return 0
            d.infobox(
                "Please connect the USB device now.",
                backtitle=self.description,
                height=3,
                width=38,
            )
            # Wait until USB device is plugged in.
            context = pyudev.Context()
            monitor = pyudev.Monitor.from_netlink(context)
            monitor.filter_by(subsystem="block", device_type="partition")
            monitor.start()
            for device in iter(monitor.poll, None):
                # Only process 'add' events.
                if device.action != "add":
                    continue
                # Only process partitions with a file systems.
                if not "ID_FS_TYPE" in device:
                    continue
                break
            d.infobox(
                "USB device {} detected. Please wait ...".format(
                    device.get("DEVNAME")
                ),
                backtitle=self.description,
                height=3,
                width=50,
            )
            try:
                mntdir = tempfile.mkdtemp()
                outfile = "{}/sysinfo-{}-{}.txt".format(
                    mntdir, socket.gethostname(), time.strftime("%Y%m%d%H%M")
                )
                openmediavault.procutils.check_call(
                    ["mount", device.get("DEVNAME"), mntdir]
                )
                with open(outfile, "w") as out:
                    openmediavault.procutils.check_call(
                        ["omv-sysinfo"], stdout=out
                    )
            except:  # pylint: disable=try-except-raise
                raise
            finally:
                openmediavault.procutils.check_call(
                    ["umount", device.get("DEVNAME")]
                )
                shutil.rmtree(mntdir)
            d.infobox(
                "You can disconnect the USB device now.",
                backtitle=self.description,
                height=3,
                width=42,
            )
        else:
            print(
                "Submitting system diagnostic report to the "
                "administrator account. Please check your email "
                "mailbox ..."
            )
            openmediavault.procutils.check_call(
                [
                    "omv-sysinfo",
                    "|",
                    "mail",
                    "-s",
                    "System diagnostic report",
                    "root",
                ]
            )
        return 0


if __name__ == "__main__":
    module = Module()
    sys.exit(module.execute())
