#!/bin/sh
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2020 Volker Theile
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
#     <afp>
#       <enable>0|1</enable>
#       <allowclrtxt>0</allowclrtxt>
#       <homesenable>0</homesenable>
#       <extraoptions></extraoptions>
#       <shares>
#       	<!--
#       	<share>
#       		<uuid>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</uuid>
#       		<sharedfolderref>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</sharedfolderref>
#       		<name>xxx</name>
#       		<comment>xxx</comment>
#       		<password></password>
#       		<casefold>none|tolower|toupper|xlatelower|xlateupper</casefold>
#       		<volsizelimit>0</volsizelimit>
#       		<allowguest>0|1</allowguest>
#       		<guestrw>0|1</guestrw>
#       		<options>
#       			<ro>0|1</ro>
#       			<upriv>0|1</upriv>
#       			<invisibledots>0|1</invisibledots>
#       			<tm>0|1</tm>
#       		</options>
#       		<extraoptions></extraoptions>
#       	</share>
#       	-->
#       </shares>
#     </afp>
#   </services>
# </config>
########################################################################
if ! omv_config_exists "/config/services/afp"; then
	omv_config_add_node "/config/services" "afp"
	omv_config_add_key "/config/services/afp" "enable" "0"
	omv_config_add_key "/config/services/afp" "allowclrtxt" "0"
	omv_config_add_key "/config/services/afp" "homesenable" "0"
	omv_config_add_key "/config/services/afp" "extraoptions" ""
	omv_config_add_node "/config/services/afp" "shares"
fi
exit 0
