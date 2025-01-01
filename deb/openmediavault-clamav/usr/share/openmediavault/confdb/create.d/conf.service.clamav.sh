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
#     <clamav>
#       <enable>0</enable>
#       <quarantine>
#         <sharedfolderref>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</sharedfolderref>
#       </quarantine>
#       <clamd>
#         <logclean>0</logclean>
#         <scanpe>1</scanpe>
#         <scanole2>1</scanole2>
#         <scanhtml>1</scanhtml>
#         <scanpdf>1</scanpdf>
#         <scanelf>1</scanelf>
#         <scanarchive>1</scanarchive>
#         <detectbrokenexecutables>0</detectbrokenexecutables>
#         <alertbrokenmedia>0</alertbrokenmedia>
#         <algorithmicdetection>1</algorithmicdetection>
#         <followdirectorysymlinks>0</followdirectorysymlinks>
#         <followfilesymlinks>0</followfilesymlinks>
#         <detectpua>0</detectpua>
#         <extraoptions></extraoptions>
#       </clamd>
#       <freshclam>
#         <!--
#         <enable>0|1</enable>
#         <checks>0 = disable, 1..50</checks>
#         -->
#         <enable>1</enable>
#         <checks>24</checks>
#       </freshclam>
#       <jobs>
#         <!--
#         <job>
#           <uuid>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</uuid>
#           <enable>0|1</enable>
#           <sharedfolderref>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</sharedfolderref>
#           <minute>[00-59|*]</minute>
#           <everynminute>0|1</everynminute>
#           <hour>[00-23|*]</hour>
#           <everynhour>0|1</everynhour>
#           <dayofmonth>[01-31|*]</dayofmonth>
#           <everyndayofmonth>0|1</everyndayofmonth>
#           <month>[01-12|*]</month>
#           <dayofweek>[1-7|*]</dayofweek>
#           <sendemail>0|1</sendemail>
#           <onaccess>0|1</onaccess>
#           <virusaction>nothing|quarantine|delete</virusaction>
#           <verbose>0|1</verbose>
#           <multiscan>0|1</multiscan>
#           <comment></comment>
#         </job>
#         -->
#       </jobs>
#       <onaccesspaths>
#         <!--
#         <onaccesspath>
#           <uuid>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</uuid>
#           <enable>0|1</enable>
#           <sharedfolderref>xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx</sharedfolderref>
#         </onaccesspath>
#         -->
#       </onaccesspaths>
#     </clamav>
#   </services>
# </config>
########################################################################
if ! omv_config_exists "/config/services/clamav"; then
	omv_config_add_node "/config/services" "clamav"
	omv_config_add_key "/config/services/clamav" "enable" "0"
	omv_config_add_node "/config/services/clamav" "quarantine"
	omv_config_add_key "/config/services/clamav/quarantine" "sharedfolderref" ""
	omv_config_add_node "/config/services/clamav" "clamd"
	omv_config_add_key "/config/services/clamav/clamd" "logclean" "0"
	omv_config_add_key "/config/services/clamav/clamd" "scanpe" "1"
	omv_config_add_key "/config/services/clamav/clamd" "scanole2" "1"
	omv_config_add_key "/config/services/clamav/clamd" "scanhtml" "1"
	omv_config_add_key "/config/services/clamav/clamd" "scanpdf" "1"
	omv_config_add_key "/config/services/clamav/clamd" "scanelf" "1"
	omv_config_add_key "/config/services/clamav/clamd" "scanarchive" "1"
	omv_config_add_key "/config/services/clamav/clamd" "detectbrokenexecutables" "0"
	omv_config_add_key "/config/services/clamav/clamd" "algorithmicdetection" "1"
	omv_config_add_key "/config/services/clamav/clamd" "followdirectorysymlinks" "0"
	omv_config_add_key "/config/services/clamav/clamd" "followfilesymlinks" "0"
	omv_config_add_key "/config/services/clamav/clamd" "detectpua" "0"
	omv_config_add_key "/config/services/clamav/clamd" "extraoptions" ""
	omv_config_add_node "/config/services/clamav" "freshclam"
	omv_config_add_key "/config/services/clamav/freshclam" "enable" "1"
	omv_config_add_key "/config/services/clamav/freshclam" "checks" "24"
	omv_config_add_node "/config/services/clamav" "jobs"
	omv_config_add_node "/config/services/clamav" "onaccesspaths"
fi

exit 0
