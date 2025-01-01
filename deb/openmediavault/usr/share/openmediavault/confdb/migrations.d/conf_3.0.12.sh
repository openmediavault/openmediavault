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

omv_config_add_node "/config/system/network" "proxy"
omv_config_add_node "/config/system/network/proxy" "http"
omv_config_add_key "/config/system/network/proxy/http" "enable" "0"
omv_config_add_key "/config/system/network/proxy/http" "host" ""
omv_config_add_key "/config/system/network/proxy/http" "port" "8080"
omv_config_add_key "/config/system/network/proxy/http" "username" ""
omv_config_add_key "/config/system/network/proxy/http" "password" ""
omv_config_add_node "/config/system/network/proxy" "https"
omv_config_add_key "/config/system/network/proxy/https" "enable" "0"
omv_config_add_key "/config/system/network/proxy/https" "host" ""
omv_config_add_key "/config/system/network/proxy/https" "port" "4443"
omv_config_add_key "/config/system/network/proxy/https" "username" ""
omv_config_add_key "/config/system/network/proxy/https" "password" ""
omv_config_add_node "/config/system/network/proxy" "ftp"
omv_config_add_key "/config/system/network/proxy/ftp" "enable" "0"
omv_config_add_key "/config/system/network/proxy/ftp" "host" ""
omv_config_add_key "/config/system/network/proxy/ftp" "port" "2121"
omv_config_add_key "/config/system/network/proxy/ftp" "username" ""
omv_config_add_key "/config/system/network/proxy/ftp" "password" ""

exit 0
