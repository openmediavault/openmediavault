#!/bin/bash
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2016 Volker Theile
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

OMV_NOTIFICATION_SENDER=${OMV_NOTIFICATION_SENDER:-"$1"}
OMV_NOTIFICATION_USER=${OMV_NOTIFICATION_USER:-"$2"}
OMV_NOTIFICATION_RECEIVER=${OMV_NOTIFICATION_RECEIVER:-"$3"}
OMV_NOTIFICATION_SUBJECT=${OMV_NOTIFICATION_SUBJECT:-""}
OMV_NOTIFICATION_MESSAGE=${OMV_NOTIFICATION_MESSAGE:-""}
OMV_NOTIFICATION_MESSAGE_FILE=${OMV_NOTIFICATION_MESSAGE_FILE:-"$(tempfile)"}

# Read the email from stdin.
read OMV_NOTIFICATION_MESSAGE
cat >${OMV_NOTIFICATION_MESSAGE_FILE}

# Extract the email subject.
OMV_NOTIFICATION_SUBJECT="${OMV_NOTIFICATION_MESSAGE/Subject: /}"

# Export environment variables that can be used by the notification scripts.
export OMV_NOTIFICATION_SENDER
export OMV_NOTIFICATION_USER
export OMV_NOTIFICATION_RECEIVER
export OMV_NOTIFICATION_SUBJECT
export OMV_NOTIFICATION_MESSAGE
export OMV_NOTIFICATION_MESSAGE_FILE

# Execute the notification scripts.
run-parts --report --lsbsysinit --arg=${OMV_NOTIFICATION_MESSAGE_FILE} \
  --arg=${OMV_NOTIFICATION_SENDER} --arg=${OMV_NOTIFICATION_RECEIVER} \
  ${OMV_NOTIFICATION_SINK_DIR}

rm -f ${OMV_NOTIFICATION_MESSAGE_FILE}
