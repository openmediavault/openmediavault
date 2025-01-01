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
#     <filebrowser>
#       <enable>0|1</enable>
#       <port>3670</port>
#       <sslcertificateref></sslcertificateref>
#       <sharedfolderref></sharedfolderref>
#     </filebrowser>
#   </services>
# </config>
########################################################################
if ! omv_config_exists "/config/services/filebrowser"; then
	omv_config_add_node "/config/services" "filebrowser"
	omv_config_add_key "/config/services/filebrowser" "enable" "0"
	omv_config_add_key "/config/services/filebrowser" "port" "3670"
	omv_config_add_key "/config/services/filebrowser" "sslcertificateref" ""
	omv_config_add_key "/config/services/filebrowser" "sharedfolderref" ""
fi

exit 0
