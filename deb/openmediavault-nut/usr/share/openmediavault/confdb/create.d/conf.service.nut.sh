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

. /usr/share/openmediavault/scripts/helper-functions

########################################################################
# Update the configuration.
# <config>
#   <services>
#     <nut>
#       <enable>0|1</enable>
#       <upsname>xxx</upsname>
#       <comment>xxx</comment>
#       <driverconf>xxx</driverconf>
#       <shutdownmode>fsd|onbatt<shutdownmode>
#       <shutdowntimer>xxx</shutdowntimer>
#       <remotemonitor>0|1</remotemonitor>
#       <remoteuser></remoteuser>
#       <remotepassword></remotepassword>
#     </nut>
#   </services>
# </config>
########################################################################
if ! omv_config_exists "/config/services/nut"; then
	# Set driver configuration for an USB connected device as default.
	driverconf=$(cat <<-EOF
driver = usbhid-ups
port = auto
EOF
)
	omv_config_add_node "/config/services" "nut"
	omv_config_add_key "/config/services/nut" "enable" "0"
	omv_config_add_key "/config/services/nut" "mode" "standalone"
	omv_config_add_key "/config/services/nut" "upsname" "ups"
	omv_config_add_key "/config/services/nut" "comment" ""
	omv_config_add_key "/config/services/nut" "netclienthostname" ""
	omv_config_add_key "/config/services/nut" "netclientusername" ""
	omv_config_add_key "/config/services/nut" "netclientpassword" ""
	omv_config_add_key "/config/services/nut" "powervalue" "1"
	omv_config_add_key "/config/services/nut" "driverconf" "${driverconf}"
	omv_config_add_key "/config/services/nut" "shutdownmode" "onbatt"
	omv_config_add_key "/config/services/nut" "shutdowntimer" "30"
	omv_config_add_key "/config/services/nut" "remotemonitor" "0"
	omv_config_add_key "/config/services/nut" "remoteuser" ""
	omv_config_add_key "/config/services/nut" "remotepassword" ""
fi

exit 0
