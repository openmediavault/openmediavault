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
#     <usbbackup>
#       <jobs>
#         <job>
#           <uuid>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</uuid>
#           <enable>0|1</enable>
#           <mode>push|pull</mode>
#           <sendemail>0|1<sendemail>
#           <comment>xxx</comment>
#           <devicefile>xxx</devicefile>
#           <sharedfolderref>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</sharedfolderref>
#           <usesubdir>0|1</usesubdir>
#           <recursive>1</recursive>
#           <times>1</times>
#           <compress>0</compress>
#           <archive>1</archive>
#           <delete>0</delete>
#           <quiet>0</quiet>
#           <perms>1</perms>
#           <acls>1</acls>
#           <xattrs>0</xattrs>
#           <partial>0</partial>
#           <extraoptions></extraoptions>
#         </job>
#       </jobs>
#     <usbbackup>
#   </services>
# </config>
########################################################################
if ! omv_config_exists "/config/services/usbbackup"; then
	omv_config_add_node "/config/services" "usbbackup"
	omv_config_add_node "/config/services/usbbackup" "jobs"
fi

exit 0
