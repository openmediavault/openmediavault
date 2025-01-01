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

# Modify the model of the rsync jobs.
xmlstarlet sel -t -m "/config/services/rsync/jobs/job" \
	-v "uuid" -n \
	${OMV_CONFIG_FILE} | xmlstarlet unesc |
	while read uuid; do
		src=$(omv_config_get "/config/services/rsync/jobs/job[uuid='${uuid}']/src")
		omv_config_delete "/config/services/rsync/jobs/job[uuid='${uuid}']/src"
		omv_config_add_key "/config/services/rsync/jobs/job[uuid='${uuid}']" "src" ""
		if omv_isuuid "${src}"; then
			omv_config_add_key "/config/services/rsync/jobs/job[uuid='${uuid}']/src" "sharedfolderref" "${src}"
			omv_config_add_key "/config/services/rsync/jobs/job[uuid='${uuid}']/src" "uri" ""
		else
			omv_config_add_key "/config/services/rsync/jobs/job[uuid='${uuid}']/src" "sharedfolderref" ""
			omv_config_add_key "/config/services/rsync/jobs/job[uuid='${uuid}']/src" "uri" "${src}"
		fi

		dest=$(omv_config_get "/config/services/rsync/jobs/job[uuid='${uuid}']/dest")
		omv_config_delete "/config/services/rsync/jobs/job[uuid='${uuid}']/dest"
		omv_config_add_key "/config/services/rsync/jobs/job[uuid='${uuid}']" "dest" ""
		if omv_isuuid "${dest}"; then
			omv_config_add_key "/config/services/rsync/jobs/job[uuid='${uuid}']/dest" "sharedfolderref" "${dest}"
			omv_config_add_key "/config/services/rsync/jobs/job[uuid='${uuid}']/dest" "uri" ""
		else
			omv_config_add_key "/config/services/rsync/jobs/job[uuid='${uuid}']/dest" "sharedfolderref" ""
			omv_config_add_key "/config/services/rsync/jobs/job[uuid='${uuid}']/dest" "uri" "${dest}"
		fi
	done

# Remove the TFTP configuration.
omv_config_delete "/config/services/tftp"
omv_config_delete "/config/services/zeroconf/services/service[uuid='1e70f7f8-f4db-11e5-9e90-000c2972b176']"

exit 0
