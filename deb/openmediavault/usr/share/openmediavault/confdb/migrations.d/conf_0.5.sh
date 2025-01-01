#!/bin/sh
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

# Add avahi configurations.
omv_config_add_node "/config/services" "zeroconf"
omv_config_add_node "/config/services/zeroconf" "services"
omv_config_add_node_data "/config/services/zeroconf/services" "service" "<id>webadmin</id><enable>1</enable><name>%h - Web administration</name>"
omv_config_add_node_data "/config/services/zeroconf/services" "service" "<id>ssh</id><enable>1</enable><name>%h - SSH</name>"
omv_config_add_node_data "/config/services/zeroconf/services" "service" "<id>nfs</id><enable>1</enable><name>%h - NFS</name>"
omv_config_add_node_data "/config/services/zeroconf/services" "service" "<id>ftp</id><enable>1</enable><name>%h - FTP</name>"
omv_config_add_node_data "/config/services/zeroconf/services" "service" "<id>tftp</id><enable>1</enable><name>%h - TFTP</name>"
omv_config_add_node_data "/config/services/zeroconf/services" "service" "<id>smb</id><enable>1</enable><name>%h - SMB/CIFS</name>"
omv_config_add_node_data "/config/services/zeroconf/services" "service" "<id>rsync</id><enable>1</enable><name>%h - Rsync</name>"
omv_config_delete "//dnssd"

# Add notification configurations.
omv_config_add_node "/config/system" "notification"
omv_config_add_node "/config/system/notification" "notifications"
omv_config_add_key "/config/system/notification/notifications" "monitprocevents" "1"
omv_config_add_key "/config/system/notification/notifications" "monitloadavg" "1"
omv_config_add_key "/config/system/notification/notifications" "monitmemoryusage" "1"
omv_config_add_key "/config/system/notification/notifications" "monitcpuusage" "1"
omv_config_add_key "/config/system/notification/notifications" "monitfilesystems" "1"
omv_config_add_key "/config/system/notification/notifications" "mdadm" "1"
omv_config_add_key "/config/system/notification/notifications" "smartmontools" "1"

# Add new Rsync module option.
omv_config_add_key "/config/services/rsync/server/modules/module" "usechroot" "1"

# Add comment to network interface configurations.
omv_config_add_key "/config/system/network/interfaces/iface" "comment" ""
omv_config_add_key "/config/system/network/interfaces/bondiface" "comment" ""

# Add 'aio' option to SMB/CIFS server. Configuration will be
# reloaded automatically.
omv_config_add_key "/config/services/smb" "aio" "1"

# Add new S.M.A.R.T. scheduled test option.
omv_config_add_key "/config/services/smart/scheduledtests/job" "enable" "1"

exit 0
