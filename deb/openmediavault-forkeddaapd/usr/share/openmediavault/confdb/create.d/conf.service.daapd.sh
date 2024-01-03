#!/usr/bin/env dash
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2024 Volker Theile
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
#     <daap>
#       <!--
#       <sharedfolderref>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</sharedfolderref>
#       -->
#       <enable>0|1</enable>
#       <libraryname>%h - iTunes/DAAP</libraryname>
#       <port>3689</port>
#       <sharedfolderref></sharedfolderref>
#       <passwordrequired>0|1</passwordrequired>
#       <password></password>
#       <adminpassword>unused</adminpassword>
#     </daap>
#   </services>
# </config>
########################################################################
if ! omv_config_exists "/config/services/daap"; then
	omv_config_add_node "/config/services" "daap"
	omv_config_add_key "/config/services/daap" "enable" "0"
	omv_config_add_key "/config/services/daap" "libraryname" "%h - iTunes/DAAP"
	omv_config_add_key "/config/services/daap" "port" "3689"
	omv_config_add_key "/config/services/daap" "sharedfolderref" ""
	omv_config_add_key "/config/services/daap" "passwordrequired" "0"
	omv_config_add_key "/config/services/daap" "password" ""
	omv_config_add_key "/config/services/daap" "adminpassword" "unused"
fi

exit 0
