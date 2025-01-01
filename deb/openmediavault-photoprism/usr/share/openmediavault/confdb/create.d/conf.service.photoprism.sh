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
#     <photoprism>
#       <enable>0|1</enable>
#       <port>2342</port>
#       <sslcertificateref></sslcertificateref>
#       <readonly>0|1</readonly>
#       <public>0|1</public>
#       <originalslimit>1000</originalslimit>
#       <originals_sharedfolderref></originals_sharedfolderref>
#       <appdata_sharedfolderref></appdata_sharedfolderref>
#       <import_sharedfolderref></import_sharedfolderref>
#     </photoprism>
#   </services>
# </config>
########################################################################
if ! omv_config_exists "/config/services/photoprism"; then
	omv_config_add_node "/config/services" "photoprism"
	omv_config_add_key "/config/services/photoprism" "enable" "0"
	omv_config_add_key "/config/services/photoprism" "port" "2342"
	omv_config_add_key "/config/services/photoprism" "sslcertificateref" ""
	omv_config_add_key "/config/services/photoprism" "readonly" "0"
	omv_config_add_key "/config/services/photoprism" "public" "0"
	omv_config_add_key "/config/services/photoprism" "originalslimit" "1000"
	omv_config_add_key "/config/services/photoprism" "originals_sharedfolderref" ""
	omv_config_add_key "/config/services/photoprism" "appdata_sharedfolderref" ""
	omv_config_add_key "/config/services/photoprism" "import_sharedfolderref" ""
fi

exit 0
