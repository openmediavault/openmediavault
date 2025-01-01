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

# Modify the notification settings.
# Note, the query must ensure that new appended child nodes are not processed.
xmlstarlet sel -t -m "/config/system/notification/notifications/*[not(name(.)='notification')]" \
	-v "name(.)" -n \
	${OMV_CONFIG_FILE} | xmlstarlet unesc |
	while read id; do
		# Generate the new notification object.
		enable=$(omv_config_get "/config/system/notification/notifications/${id}")
		object="<uuid>$(omv_uuid)</uuid>"
		object="${object}<id>${id}</id>"
		object="${object}<enable>${enable}</enable>"
		omv_config_add_node_data "/config/system/notification/notifications" "notification" "${object}"
		omv_config_delete "/config/system/notification/notifications/${id}"
	done

# Add the 'enable' attribute to FTP shares.
omv_config_add_key "/config/services/ftp/shares/share" "enable" "1"

# Relocate hostname/domainname settings.
hostname=$(omv_config_get "/config/system/network/hostname")
domainname=$(omv_config_get "/config/system/network/domainname")
omv_config_add_node "/config/system/network" "dns"
omv_config_add_key "/config/system/network/dns" "hostname" "${hostname}"
omv_config_add_key "/config/system/network/dns" "domainname" "${domainname}"
omv_config_delete "/config/system/network/hostname"
omv_config_delete "/config/system/network/domainname"

# Rename S.M.A.R.T. device attribute 'type'.
omv_config_rename "//services/smart/monitor/device/type" "devicetype"

exit 0
