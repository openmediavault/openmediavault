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

# Get the current timezone and update the database.
# https://unix.stackexchange.com/questions/451709/timedatectl-set-timezone-doesnt-update-etc-timezone/451714#451714
if systemctl is-system-running --quiet; then
	timezone=$(salt-call --local --retcode-passthrough --no-color --out=json timezone.get_zone | \
		jq --raw-output '.[]')
else
	timezone=$(cat /etc/timezone)
fi
data=$(omv-confdbadm read "conf.system.time" | jq ".timezone = \"${timezone}\"")
omv-confdbadm update "conf.system.time" "${data}"

exit 0
