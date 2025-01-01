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

########################################################################
# Update the configuration.
# <config>
#   <services>
#     <snmp>
#       <!--
#       <version>2c|3</version>
#       <securitylevel>noauth|auth|priv</securitylevel>
#       <authtype>MD5|SHA</authtype>
#       <privtype>DES|AES</privtype>
#       -->
#       <enable>0</enable>
#       <community>public</community>
#       <syslocation></syslocation>
#       <syscontact></syscontact>
#       <version>2c</version>
#       <username></username>
#       <securitylevel>noauth</securitylevel>
#       <authtype>MD5</authtype>
#       <authpassphrase></authpassphrase>
#       <privtype>DES</privtype>
#       <privpassphrase></privpassphrase>
#       <extraoptions></extraoptions>
#       <trap>
#         <enable>0</enable>
#         <community>public</community>
#         <host>localhost</host>
#         <port>162</port>
#       </trap>
#     </snmp>
#   </services>
# </config>
########################################################################
if ! omv-confdbadm exists "conf.service.snmp"; then
	omv-confdbadm read --defaults "conf.service.snmp" | omv-confdbadm update "conf.service.snmp" -
fi

exit 0
