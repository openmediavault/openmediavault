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

omv_config_add_node "/config/services/smart" "monitor"
xmlstarlet sel -t -m "/config/system/storage/hdparm" \
	-v "uuid" \
	-i "position() != last()" -n -b \
	${OMV_CONFIG_FILE} | xmlstarlet unesc |
	while read uuid; do
		devicefile=$(omv_config_get "/config/system/storage/hdparm[uuid='${uuid}']/devicefile")
		enable=$(omv_config_get "/config/system/storage/hdparm[uuid='${uuid}']/smart/enable")
		object="<uuid>$(omv_uuid)</uuid>"
		object="${object}<devicefile>${devicefile}</devicefile>"
		object="${object}<enable>${enable}</enable>"
		object="${object}<type></type>"
		omv_config_add_node_data "/config/services/smart/monitor" "device" "${object}"
		omv_config_delete "/config/system/storage/hdparm[uuid='${uuid}']/smart"
	done

exit 0
