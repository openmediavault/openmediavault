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

# Add monitoring configuration.
omv_config_add_node "/config/system" "monitoring"
omv_config_add_node "/config/system/monitoring" "perfstats"
omv_config_add_key "/config/system/monitoring/perfstats" "enable" "1"

# Convert public SSH key.
xmlstarlet sel -t -m "/config/system/usermanagement/users/user[string-length(sshpubkey) > 0]" \
	-v "uuid" \
	-i "position() != last()" -n -b \
	${OMV_CONFIG_FILE} | xmlstarlet unesc |
	while read uuid; do
		tmpfile=$(mktemp)
		omv_config_get "/config/system/usermanagement/users/user[uuid='${uuid}']/sshpubkey" >"${tmpfile}"
		sshpubkey=$(ssh-keygen -e -f "${tmpfile}")
		rm -f "${tmpfile}"
		omv_config_update "/config/system/usermanagement/users/user[uuid='${uuid}']/sshpubkey" "${sshpubkey}"
	done

# Mark modules as dirty.
omv_module_set_dirty ssh

exit 0
