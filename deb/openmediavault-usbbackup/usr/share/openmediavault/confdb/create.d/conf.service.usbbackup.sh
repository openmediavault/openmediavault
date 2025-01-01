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
#           <reldirpath>xxx</reldirpath>
#           <optionrecursive>1</optionrecursive>
#           <optiontimes>1</optiontimes>
#           <optiongroup>1</optiongroup>
#           <optionowner>1</optionowner>
#           <optioncompress>0</optioncompress>
#           <optionarchive>1</optionarchive>
#           <optiondelete>0</optiondelete>
#           <optionquiet>0</optionquiet>
#           <optionperms>1</optionperms>
#           <optionacls>1</optionacls>
#           <optionxattrs>0</optionxattrs>
#           <optionpartial>0</optionpartial>
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
