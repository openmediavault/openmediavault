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

. /etc/default/openmediavault
. /usr/share/openmediavault/scripts/helper-functions

OMV_CONFIG_FILE=${OMV_CONFIG_FILE:-"/etc/openmediavault/config.xml"}
OMV_CONFIG_TEMPLATE_FILE=${OMV_CONFIG_TEMPLATE_FILE:-"/usr/share/openmediavault/templates/config.xml"}

# Install the configuration database.
if [ ! -e "${OMV_CONFIG_FILE}" ]; then
	cp ${OMV_CONFIG_TEMPLATE_FILE} ${OMV_CONFIG_FILE}
fi
# Always update the file permissions.
chmod 660 ${OMV_CONFIG_FILE}
chown :openmediavault-config ${OMV_CONFIG_FILE}

exit 0
