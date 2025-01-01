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
import netifaces
import openmediavault.firstaid
import openmediavault.rpc


class Module(openmediavault.firstaid.IModule):
    @property
    def description(self):
        return "Configure workbench"

    def execute(self):
        # Default values.
        port = None
        sslport = 443
        enablessl = False
        forcesslonly = False
        sslcertificateref = ""
        # Get the HTTP port.
        d = dialog.Dialog(dialog="dialog")
        while not port:
            (code, port) = d.inputbox(
                "Please enter the port to access the Workbench via HTTP.",
                backtitle=self.description,
                clear=True,
                height=8,
                width=64,
                init="80",
            )
            if code != d.OK:
                return 0
            if not port:
                d.msgbox(
                    "The field must not be empty.",
                    backtitle=self.description,
                    height=5,
                    width=32,
                )
                continue
            if not port.isdigit():
                port = None
                d.msgbox(
                    "Please enter a valid port.",
                    backtitle=self.description,
                    height=5,
                    width=32,
                )
                continue
            port = int(port)
            if port not in range(1, 65535):
                port = None
                d.msgbox(
                    "Please enter a valid port.",
                    backtitle=self.description,
                    height=5,
                    width=32,
                )
                continue
        # Before asking to enable HTTPS check if there are any SSL
        # certificates available.
        ssl_certs = openmediavault.rpc.call(
            "CertificateMgmt",
            "getList",
            {"start": 0, "limit": -1, "sortfield": None, "sortdir": None},
        )
        if ssl_certs["total"] > 0:
            code = d.yesno(
                "Do you want to enable HTTPS?",
                backtitle=self.description,
                height=5,
                width=32,
            )
            if code == d.ESC:
                return 0
            if code == d.OK:
                enablessl = True
                sslport = None
                # Get the port for HTTPS.
                while not sslport:
                    (code, sslport) = d.inputbox(
                        "Please enter the port "
                        "to access the Workbench via HTTPS.",
                        backtitle=self.description,
                        clear=True,
                        height=8,
                        width=65,
                        init="443",
                    )
                    if code != d.OK:
                        return 0
                    if not sslport:
                        d.msgbox(
                            "The field must not be empty.",
                            backtitle=self.description,
                            height=5,
                            width=32,
                        )
                        continue
                    if not sslport.isdigit():
                        sslport = None
                        d.msgbox(
                            "Please enter a valid port.",
                            backtitle=self.description,
                            height=5,
                            width=32,
                        )
                        continue
                    sslport = int(sslport)
                    if sslport not in range(1, 65535):
                        sslport = None
                        d.msgbox(
                            "Please enter a valid port.",
                            backtitle=self.description,
                            height=5,
                            width=32,
                        )
                        continue
                # Display dialog to choose SSL certificate
                choices = []
                for idx, ssl_cert in enumerate(ssl_certs["data"]):
                    choices.append([str(idx + 1), ssl_cert["comment"]])
                (code, tag) = d.menu(
                    "Please select an SSL certificate.",
                    backtitle=self.description,
                    clear=True,
                    height=15,
                    width=65,
                    menu_height=8,
                    choices=choices,
                )
                if code in (d.CANCEL, d.ESC):
                    return 0
                ssl_cert = ssl_certs["data"][int(tag) - 1]
                sslcertificateref = ssl_cert["uuid"]
                # Enable HTTPS only?
                code = d.yesno(
                    "Do you want to enable HTTPS only?",
                    backtitle=self.description,
                    defaultno=True,
                    height=5,
                    width=40,
                )
                if code == d.ESC:
                    return 0
                if code == d.OK:
                    forcesslonly = True
        # Update the configuration.
        print("Updating Workbench settings. Please wait ...")
        openmediavault.rpc.call(
            "WebGui",
            "setSettings",
            {
                "port": port,
                "enablessl": enablessl,
                "sslport": sslport,
                "forcesslonly": forcesslonly,
                "sslcertificateref": sslcertificateref,
                "timeout": 5,
            },
        )
        # Apply the configuration changes.
        openmediavault.rpc.call(
            "Config", "applyChanges", {"modules": [], "force": False}
        )
        print("The Workbench settings were successfully changed.")
        # Display the URL's available to reach the web control panel.
        print("\nThe Workbench is reachable via URL:")
        for if_name in netifaces.interfaces():
            # Skip unwanted network interface devices.
            if if_name in ["lo"]:
                continue
            if_addrs = netifaces.ifaddresses(if_name)
            for if_familiy in (netifaces.AF_INET, netifaces.AF_INET6):
                if if_familiy in if_addrs:
                    for if_addr in if_addrs[if_familiy]:
                        if not forcesslonly:
                            print(
                                "{}: http://{}:{}".format(
                                    if_name, if_addr["addr"], port
                                )
                            )
                        if enablessl:
                            print(
                                "{}: https://{}:{}".format(
                                    if_name, if_addr["addr"], sslport
                                )
                            )
        return 0


if __name__ == "__main__":
    module = Module()
    sys.exit(module.execute())
