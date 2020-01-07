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
#     <ldap>
#       <enable>0</enable>
#       <host></host>
#       <port>389</port>
#       <enablessl>0</enablessl>
#       <base></base>
#       <rootbinddn></rootbinddn>
#       <rootbindpw></rootbindpw>
#       <usersuffix></usersuffix>
#       <groupsuffix></groupsuffix>
#       <enablepam>0|1</enablepam>
#       <extraoptions></extraoptions>
#       <extraclientoptions></extraclientoptions>
#     </ldap>
#   </services>
# </config>
########################################################################
if ! omv_config_exists "/config/services/ldap"; then
	omv_config_add_node "/config/services" "ldap"
	omv_config_add_key "/config/services/ldap" "enable" "0"
	omv_config_add_key "/config/services/ldap" "host" ""
	omv_config_add_key "/config/services/ldap" "port" "389"
	omv_config_add_key "/config/services/ldap" "enablessl" "0"
	omv_config_add_key "/config/services/ldap" "base" ""
	omv_config_add_key "/config/services/ldap" "rootbinddn" ""
	omv_config_add_key "/config/services/ldap" "rootbindpw" ""
	omv_config_add_key "/config/services/ldap" "usersuffix" "ou=Users"
	omv_config_add_key "/config/services/ldap" "groupsuffix" "ou=Groups"
	omv_config_add_key "/config/services/ldap" "enablepam" "1"
	omv_config_add_key "/config/services/ldap" "extraoptions" ""
	omv_config_add_key "/config/services/ldap" "extraclientoptions" ""
fi

exit 0
