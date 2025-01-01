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

omv_config_add_key "/config/services/usbbackup/jobs/job" "reldirpath" "/"
omv_config_exec_query -m "/config/services/usbbackup/jobs/job[usesubdir='1']" \
	-v \"uuid\" \
	-i \"position\(\) != last\(\)\" -n -b |
	while read uuid; do
		sfref=$(omv_config_get "/config/services/usbbackup/jobs/job[uuid='${uuid}']/sharedfolderref")
		sfname=$(omv_get_sharedfolder_name "${sfref}")
		omv_config_update "/config/services/usbbackup/jobs/job[uuid='${uuid}']/reldirpath" "${sfname}/"
	done
omv_config_delete "/config/services/usbbackup/jobs/job/usesubdir"

exit 0
