#!/usr/bin/env dash
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

set -e

. /etc/default/openmediavault
. /usr/share/openmediavault/scripts/helper-functions

OMV_INTERFACES_CONFIG=${OMV_INTERFACES_CONFIG:-"/etc/network/interfaces"}

[ ! -e "${OMV_INTERFACES_CONFIG}" ] && exit 0

# Get the DNS name servers.
# Note, we are setting all name servers for each interface because this is
# much less complicated.
dnsnameservers=""
for dnsnameserver in $(grep -iP "^\s*dns-nameservers\s+.*$" ${OMV_INTERFACES_CONFIG} |
        sed -e 's/\s*dns-nameservers\s\+\(.\+\)/\1/gi'); do
    dnsnameservers=$(omv_trim "${dnsnameservers} ${dnsnameserver}")
done

# Configure ethernet network interfaces.
# Understanding systemd’s predictable network device names:
# https://github.com/systemd/systemd/blob/master/src/udev/udev-builtin-net_id.c
grep -iP "^\s*iface\s+(eth[0-9]+|en[a-z0-9]+)\s+(inet|inet6)\s+(static|dhcp)" ${OMV_INTERFACES_CONFIG} |
    while read type devname family method; do
        # Skip IPv6 for now. Should be fixed in a future version.
        [ "inet6" = "${family}" ] && continue
        # Add interface if it does not already exist.
        if ! omv-confdbadm exists --filter "{\"operator\":\"and\", \
                \"arg0\":{\"operator\":\"stringEquals\",\"arg0\":\"type\", \
                \"arg1\":\"ethernet\"},\"arg1\":{\"operator\":\"stringEquals\", \
                \"arg0\":\"devicename\",\"arg1\":\"${devname}\"}}" \
                "conf.system.network.interface"; then
            address=""
            netmask=""
            gateway=""
            if [ "static" = "${method}" ]; then
                data=$(salt-call --local --retcode-passthrough --no-color \
                    --out=json network.interface "${devname}" | \
                    jq --compact-output '.[][0]')
                address=$(echo ${data} | jq --raw-output '.address // empty')
                netmask=$(echo ${data} | jq --raw-output '.netmask // empty')
                gateway=$(salt-call --local --retcode-passthrough --no-color \
                    --out=json network.routes inet | \
                    jq --raw-output ".[] | map(select((.interface == \"${devname}\") and \
                    (.flags == \"UG\"))) | first | .gateway // empty")
            fi
            jq --null-input --compact-output "{uuid: \"${OMV_CONFIGOBJECT_NEW_UUID}\", \
                devicename: \"${devname}\", type: \"ethernet\", method6: \"manual\", \
                address6: \"\", netmask6: 64, gateway6: \"\", routemetric6: 1, \
                dnsnameservers: \"${dnsnameservers}\", dnssearch: \"\", \
                wol: false, mtu: 0, \
                comment: \"\", method: \"${method}\", \
                address: \"${address}\", netmask: \"${netmask}\", \
                gateway: \"${gateway}\", routemetric: 0}" |
                omv-confdbadm update "conf.system.network.interface" -
        fi
    done

# Create a backup of the original configuration file.
cp "${OMV_INTERFACES_CONFIG}" "${OMV_INTERFACES_CONFIG}.bak"

exit 0
