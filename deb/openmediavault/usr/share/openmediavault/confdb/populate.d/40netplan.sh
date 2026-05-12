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
import glob
import ipaddress
import os
import sys
from typing import Dict, Optional

import openmediavault.config.database
import openmediavault.config.object
import openmediavault.log
import yaml


def load_netplan_yaml(file_path) -> Optional[Dict]:
    try:
        with open(file_path, 'r') as fd:
            config = yaml.safe_load(fd)
        return config
    except FileNotFoundError:
        openmediavault.log.error(f"File '{file_path}' not found")
        return None
    except yaml.YAMLError as e:
        openmediavault.log.error(f"Failed to parse YAML: {e}")
        return None
    except Exception as e:
        openmediavault.log.error(f"An unexpected error occurred: {e}")
        return None


def process_netplan_config(config: Dict) -> None:
    # network:
    #   version: 2
    #   ethernets:
    #     enp1s0:
    #       nameservers:
    #         addresses:
    #           - 192.168.122.1
    #         search:
    #           - internal
    #       optional: true
    #       ipv6-privacy: true
    #       dhcp4: true
    #       dhcp6: true
    #
    # network:
    #   ethernets:
    #     eth0:
    #       match:
    #         macaddress: "52:54:00:f0:90:b3"
    #       dhcp4: yes
    #       dhcp4-overrides:
    #         use-dns: true
    #         use-domains: true
    #       dhcp6: no
    #       link-local: []
    #
    # network:
    #   ethernets:
    #     eth1:
    #       match:
    #         macaddress: "52:54:00:2b:49:36"
    #       addresses:
    #       - 1.2.3.4/24
    #       routes:
    #       - to: 0.0.0.0/0
    #         via: 1.0.0.4
    #       dhcp4: no
    #       dhcp6: no
    #       link-local: []
    #       nameservers:
    #         addresses:
    #         - 6.6.6.6
    #         - 4.4.4.4
    #
    # network:
    #   ethernets:
    #     eth0:
    #       dhcp4: no
    #       dhcp6: no
    #       addresses:
    #         - 2001:db8:1234:5678::10/64
    #       routes:
    #         - to: ::/0
    #           via: 2001:db8:1234:5678::1
    #       nameservers:
    #         addresses:
    #           - 2001:4860:4860::8888
    #           - 2001:4860:4860::8844

    if "network" not in config:
        return

    if "ethernets" in config["network"]:
        ethernets = config["network"]["ethernets"]
        for iface_name, iface_config in ethernets.items():
            obj = openmediavault.config.Object("conf.system.network.interface")
            obj.set_dict({
                "devicename": iface_name,
                "type": "ethernet",
            })
            if iface_config.get("dhcp4", False):
                obj.set("method", "dhcp")
            else:
                if "addresses" in iface_config:
                    for address in iface_config["addresses"]:
                        address_iface = ipaddress.ip_interface(address)
                        if address_iface.version != 4:
                            continue
                        obj.set_dict({
                            "method": "static",
                            "address": address_iface.ip,
                            "netmask": address_iface.network.netmask,
                        })
                        if "routes" in iface_config:
                            for route in iface_config["routes"]:
                                if route["to"] == "0.0.0.0/0":
                                    obj.set("gateway", route["via"])
                                    break
                        # Only process the first found IPv4 address.
                        break
            if iface_config.get("dhcp6", False):
                obj.set("method6", "dhcp")
            else:
                if "addresses" in iface_config:
                    for address in iface_config["addresses"]:
                        address_iface = ipaddress.ip_interface(address)
                        if address_iface.version != 6:
                            continue
                        obj.set_dict({
                            "method6": "static",
                            "address6": address_iface.ip,
                            "netmask6": address_iface.network.prefixlen,
                        })
                        if "routes" in iface_config:
                            for route in iface_config["routes"]:
                                if route["to"] == "::/0":
                                    obj.set("gateway6", route["via"])
                                    break
                        # Only process the first found IPv6 address.
                        break
            if "nameservers" in iface_config:
                nameservers = iface_config["nameservers"]
                if "addresses" in nameservers:
                    obj.set("dnsservers", " ".join(nameservers["addresses"]))
                if "search" in nameservers:
                    obj.set("dnssearch", " ".join(nameservers["search"]))
            db = openmediavault.config.Database()
            db.set(obj)


def main():
    for file_path in glob.iglob(os.path.join("/etc/netplan", '*.yaml')):
        config = load_netplan_yaml(file_path)
        if config is None:
            continue
        process_netplan_config(config)
    return 0


if __name__ == "__main__":
    sys.exit(main())
