#!/bin/sh
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

omv_config_add_node "/config/services/k8s" "lbports"

if ! omv-confdbadm exists --filter '{"operator":"stringEquals","arg0":"name","arg1":"web"}' \
		"conf.service.k8s.lbport"; then
	port=$(omv_config_get "/config/services/k8s/webport")
	omv-confdbadm read --defaults "conf.service.k8s.lbport" | \
		jq ".name = \"web\" | .port = ${port} | .exposedport = ${port} | .protocol = \"tcp\" | .expose = true | .comment = \"HTTP\" | .extravalues=\"transport:\\n  respondingTimeouts:\\n    readTimeout: 60\"" | \
		omv-confdbadm update "conf.service.k8s.lbport" -
fi
if ! omv-confdbadm exists --filter '{"operator":"stringEquals","arg0":"name","arg1":"websecure"}' \
		"conf.service.k8s.lbport"; then
	port=$(omv_config_get "/config/services/k8s/websecureport")
	omv-confdbadm read --defaults "conf.service.k8s.lbport" | \
		jq ".name = \"websecure\" | .port = ${port} | .exposedport = ${port} | .protocol = \"tcp\" | .expose = true | .comment = \"HTTPS\" | .extravalues=\"transport:\\n  respondingTimeouts:\\n    readTimeout: 60\"" | \
		omv-confdbadm update "conf.service.k8s.lbport" -
fi
if ! omv-confdbadm exists --filter '{"operator":"stringEquals","arg0":"name","arg1":"k8s-dashboard"}' \
		"conf.service.k8s.lbport"; then
	port=$(omv_config_get "/config/services/k8s/dashboardport")
	omv-confdbadm read --defaults "conf.service.k8s.lbport" | \
		jq ".name = \"k8s-dashboard\" | .port = ${port} | .exposedport = ${port} | .protocol = \"tcp\" | .expose = true | .comment = \"Kubernetes,Dashboard\" | .extravalues= \"tls:\\n  enabled: true\"" | \
		omv-confdbadm update "conf.service.k8s.lbport" -
fi

omv_config_delete "/config/services/k8s/webport"
omv_config_delete "/config/services/k8s/websecureport"
omv_config_delete "/config/services/k8s/dashboardport"

exit 0
