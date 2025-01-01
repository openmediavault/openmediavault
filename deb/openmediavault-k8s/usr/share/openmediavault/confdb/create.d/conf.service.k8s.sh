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

########################################################################
# Update the configuration.
# <config>
#   <services>
#     <k8s>
#     	<enable>0</enable>
#     	<datastore>etcd|sqlite</datastore>
#     	<webport>8080</webport>
#     	<websecureport>8443</websecureport>
#     	<dashboardport>4443</dashboardport>
#       <sslcertificateref></sslcertificateref>
#       <snapshots_sharedfolderref></snapshots_sharedfolderref>
#     </k8s>
#   </services>
# </config>
########################################################################
if ! omv_config_exists "/config/services/k8s"; then
    omv-confdbadm read --defaults "conf.service.k8s" | omv-confdbadm update "conf.service.k8s" -
fi

exit 0
