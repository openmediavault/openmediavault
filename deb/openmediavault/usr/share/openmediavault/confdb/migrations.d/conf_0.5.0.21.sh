#!/bin/sh
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

. /usr/share/openmediavault/scripts/helper-functions

# Set IPv6 interface configurations.
omv_config_add_key "/config/system/network/interfaces/*[name()='iface' or name()='bondiface']" "method6" "manual"
omv_config_add_key "/config/system/network/interfaces/*[name()='iface' or name()='bondiface']" "address6" ""
omv_config_add_key "/config/system/network/interfaces/*[name()='iface' or name()='bondiface']" "netmask6" "64"
omv_config_add_key "/config/system/network/interfaces/*[name()='iface' or name()='bondiface']" "gateway6" ""

# Modify firewall rules.
omv_config_add_key "/config/system/network/iptables/rule" "family" "inet"

exit 0
