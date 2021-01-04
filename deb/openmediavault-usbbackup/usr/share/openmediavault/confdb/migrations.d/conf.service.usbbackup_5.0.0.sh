#!/bin/sh
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2021 Volker Theile
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

omv_config_rename "/config/services/usbbackup/jobs/job/recursive" "optionrecursive"
omv_config_rename "/config/services/usbbackup/jobs/job/times" "optiontimes"
omv_config_rename "/config/services/usbbackup/jobs/job/compress" "optioncompress"
omv_config_rename "/config/services/usbbackup/jobs/job/archive" "optionarchive"
omv_config_rename "/config/services/usbbackup/jobs/job/delete" "optiondelete"
omv_config_rename "/config/services/usbbackup/jobs/job/perms" "optionperms"
omv_config_rename "/config/services/usbbackup/jobs/job/acls" "optionacls"
omv_config_rename "/config/services/usbbackup/jobs/job/xattrs" "optionxattrs"
omv_config_rename "/config/services/usbbackup/jobs/job/quiet" "optionquiet"
omv_config_rename "/config/services/usbbackup/jobs/job/partial" "optionpartial"
omv_config_add_key "/config/services/usbbackup/jobs/job" "optiongroup" "1"
omv_config_add_key "/config/services/usbbackup/jobs/job" "optionowner" "1"

exit 0
