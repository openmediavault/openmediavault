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

versions=""
if [ -e "/proc/fs/nfsd/versions" ]; then
	# Convert the enabled NFS versions into a comma separated list.
	# Example: +2 -3 +4 -4.1 -4.2 => 2,4
	versions=$(cat /proc/fs/nfsd/versions | tr ' ' '\n' | grep '^+' | tr -d '+' | xargs | sed 's/ /,/g')
fi
if [ -n "${versions}" ]; then
    data=$(omv-confdbadm read "conf.service.nfs" | jq ".versions = \"${versions}\"")
	omv-confdbadm update "conf.service.nfs" "${data}"
fi

exit 0
