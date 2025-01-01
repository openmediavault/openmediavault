#!/bin/sh
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

omv_config_delete "/config/system/network/interfaces/interface/options"
omv_config_delete "/config/services/zeroconf"
omv_config_delete "/config/services/smb/localmaster"

omv_config_rename "/config/services/rsync/jobs/job/recursive" "optionrecursive"
omv_config_rename "/config/services/rsync/jobs/job/times" "optiontimes"
omv_config_rename "/config/services/rsync/jobs/job/compress" "optioncompress"
omv_config_rename "/config/services/rsync/jobs/job/archive" "optionarchive"
omv_config_rename "/config/services/rsync/jobs/job/delete" "optiondelete"
omv_config_rename "/config/services/rsync/jobs/job/perms" "optionperms"
omv_config_rename "/config/services/rsync/jobs/job/acls" "optionacls"
omv_config_rename "/config/services/rsync/jobs/job/xattrs" "optionxattrs"
omv_config_rename "/config/services/rsync/jobs/job/dryrun" "optiondryrun"
omv_config_rename "/config/services/rsync/jobs/job/quiet" "optionquiet"
omv_config_rename "/config/services/rsync/jobs/job/partial" "optionpartial"
omv_config_add_key "/config/services/rsync/jobs/job" "optiongroup" "1"
omv_config_add_key "/config/services/rsync/jobs/job" "optionowner" "1"

omv_config_add_key "/config/services/smb/shares/share" "timemachine" "0"

# It seems there are migrated OMV3 versions with invalid databases out there.
# The datamodel has changed with version 3.0.16.
omv_config_update "/config/system/network/interfaces/interface/mtu[.='']" "0"

exit 0
