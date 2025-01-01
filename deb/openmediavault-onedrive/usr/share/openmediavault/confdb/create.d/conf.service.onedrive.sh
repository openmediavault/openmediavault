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
#     <onedrive>
#       <enable>0|1</enable>
#       <sharedfolderref></sharedfolderref>
#       <replicationmode>down|up|updown</replicationmode>
#       <localfirst>0|1</localfirst>
#       <noremotedelete>0|1</noremotedelete>
#       <skipdotfiles>0|1</skipdotfiles>
#       <monitorinterval>300|600|900|1800|3600</monitorinterval>
#       <ratelimit>0|131072|262144|524288|1048576|10485760|104857600</ratelimit>
#       <username>onedrive</username>
#     </onedrive>
#   </services>
# </config>
########################################################################
if ! omv_config_exists "/config/services/onedrive"; then
	omv_config_add_node "/config/services" "onedrive"
	omv_config_add_key "/config/services/onedrive" "enable" "0"
	omv_config_add_key "/config/services/onedrive" "sharedfolderref" ""
	omv_config_add_key "/config/services/onedrive" "replicationmode" "updown"
	omv_config_add_key "/config/services/onedrive" "localfirst" "0"
	omv_config_add_key "/config/services/onedrive" "noremotedelete" "0"
	omv_config_add_key "/config/services/onedrive" "skipdotfiles" "0"
	omv_config_add_key "/config/services/onedrive" "monitorinterval" "300"
	omv_config_add_key "/config/services/onedrive" "ratelimit" "0"
	omv_config_add_key "/config/services/onedrive" "username" "onedrive"
fi

exit 0
