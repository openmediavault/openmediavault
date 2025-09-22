#!/usr/bin/env dash
#
# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.

set -e

. /usr/share/openmediavault/scripts/helper-functions

if ! omv-confdbadm exists --filter '{"operator":"stringEquals","arg0":"id","arg1":"authentication"}' \
		"conf.system.notification.notification"; then
	omv_config_add_node_data "/config/system/notification/notifications" "notification" \
		"<uuid>ce287478-5429-46c7-9e06-438a2aba8499</uuid><id>authentication</id><enable>1</enable>"
fi

if ! omv-confdbadm exists --filter '{"operator":"stringEquals","arg0":"id","arg1":"misc"}' \
		"conf.system.notification.notification"; then
	omv_config_add_node_data "/config/system/notification/notifications" "notification" \
		"<uuid>6a0ca6b0-81a5-11f0-a77e-e72b7329fa76</uuid><id>misc</id><enable>1</enable>"
fi

exit 0
