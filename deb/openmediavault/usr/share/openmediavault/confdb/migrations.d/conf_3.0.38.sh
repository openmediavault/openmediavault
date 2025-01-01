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

# Make sure that this fields do not exist.
omv_config_delete "/config/system/usermanagement/users/user/comment"
omv_config_delete "/config/system/usermanagement/users/user/groups"
omv_config_delete "/config/system/usermanagement/users/user/password"
omv_config_delete "/config/system/usermanagement/users/user/sharedfolderref"
omv_config_delete "/config/system/usermanagement/users/user/shell"
omv_config_delete "/config/system/usermanagement/users/user/sshpubkey"
omv_config_delete "/config/system/usermanagement/users/user/uid"

exit 0
