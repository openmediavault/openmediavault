#!/usr/bin/env dash
#
# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2026 Volker Theile
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.

set -e

. /usr/share/openmediavault/scripts/helper-functions

########################################################################
# Update the configuration.
# <config>
#   <services>
#     <k8s>
#     	<enable>0</enable>
#     	<datastore>etcd|sqlite</datastore>
#     	<webport>8080</webport>
#     	<websecureport>8443</websecureport>
#       <lbports>
#         <lbport>
#           <name>xxx</name>
#           <port>1-65535</port>
#           <exposedport>1-65535</exposedport>
#           <protocol>udp|tcp</protocol>
#           <expose>0|1</expose>
#           <comment>xxx</comment>
#           <extravalues>xxx</extravalues>
#         </lbport>
#       </ports>
#       <snapshots_sharedfolderref></snapshots_sharedfolderref>
#       <sslcertificateref></sslcertificateref>
#     </k8s>
#   </services>
# </config>
########################################################################
if ! omv_config_exists "/config/services/k8s"; then
    omv-confdbadm read --defaults "conf.service.k8s" | omv-confdbadm update "conf.service.k8s" -
	if ! omv-confdbadm exists --filter '{"operator":"stringEquals","arg0":"name","arg1":"web"}' \
			"conf.service.k8s.lbport"; then
		omv-confdbadm read --defaults "conf.service.k8s.lbport" | \
			jq '.name = "web" | .port = 8080 | .exposedport = 8080 | .protocol = "tcp" | .expose = true | .comment = "HTTP" | .extravalues="transport:\n  respondingTimeouts:\n    readTimeout: 60"' | \
			omv-confdbadm update "conf.service.k8s.lbport" -
	fi
	if ! omv-confdbadm exists --filter '{"operator":"stringEquals","arg0":"name","arg1":"websecure"}' \
			"conf.service.k8s.lbport"; then
		omv-confdbadm read --defaults "conf.service.k8s.lbport" | \
			jq '.name = "websecure" | .port = 8443 | .exposedport = 8443 | .protocol = "tcp" | .expose = true | .comment = "HTTPS" | .extravalues="transport:\n  respondingTimeouts:\n    readTimeout: 60"' | \
			omv-confdbadm update "conf.service.k8s.lbport" -
	fi
	if ! omv-confdbadm exists --filter '{"operator":"stringEquals","arg0":"name","arg1":"dashboard"}' \
			"conf.service.k8s.lbport"; then
		omv-confdbadm read --defaults "conf.service.k8s.lbport" | \
			jq '.name = "dashboard" | .port = 4443 | .exposedport = 4443 | .protocol = "tcp" | .expose = true | .comment = "Dashboard" | .extravalues= "tls:\n  enabled: true"' | \
			omv-confdbadm update "conf.service.k8s.lbport" -
	fi
fi

exit 0
