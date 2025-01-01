#!/usr/bin/env bash
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

. /etc/default/openmediavault
. /usr/share/openmediavault/scripts/helper-functions

OMV_NOTIFICATION_SINK_DIR=${OMV_NOTIFICATION_SINK_DIR:-"/usr/share/openmediavault/notification/sink.d"}

OMV_NOTIFICATION_FROM=$1
OMV_NOTIFICATION_RECIPIENT=
OMV_NOTIFICATION_SUBJECT=
OMV_NOTIFICATION_DATE=
OMV_NOTIFICATION_MESSAGE_FILE=$(mktemp)

# Clean up when done or when aborting.
trap "rm -f ${OMV_NOTIFICATION_MESSAGE_FILE}" 0 1 2 3 15

# Read the email from stdin.
cat >${OMV_NOTIFICATION_MESSAGE_FILE}

# Extract various header fields from the email.
OMV_NOTIFICATION_RECIPIENT=$(sed -n -e 's/^To: \(.*\)/\1/p' ${OMV_NOTIFICATION_MESSAGE_FILE})
OMV_NOTIFICATION_SUBJECT=$(sed -n -e 's/^Subject: \(.*\)/\1/p' ${OMV_NOTIFICATION_MESSAGE_FILE})
OMV_NOTIFICATION_DATE=$(sed -n -e 's/^Date: \(.*\)/\1/p' ${OMV_NOTIFICATION_MESSAGE_FILE})

# Remove all header fields from the email to get the plain message part.
sed -i -e '1,/^$/d' ${OMV_NOTIFICATION_MESSAGE_FILE}

# Export environment variables that can be used by the notification sinks.
export OMV_NOTIFICATION_FROM
export OMV_NOTIFICATION_RECIPIENT
export OMV_NOTIFICATION_SUBJECT
export OMV_NOTIFICATION_DATE
export OMV_NOTIFICATION_MESSAGE_FILE

# Execute the notification scripts.
run-parts --report --lsbsysinit --arg="${OMV_NOTIFICATION_MESSAGE_FILE}" \
  --arg="${OMV_NOTIFICATION_FROM}" --arg="${OMV_NOTIFICATION_RECIPIENT}" -- \
  ${OMV_NOTIFICATION_SINK_DIR}

exit 0
