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

# Note, it is necessary to stop the service, otherwise the configuration
# is auto-reloaded every time a configuration file is modified.
# Mask the service to prevent restarting via DBUS (.e.g. shairport-sync or
# forked-daapd).
mask_avahi_service:
  service.masked:
    - name: avahi-daemon
    - runtime: True

stop_avahi_service:
  service.dead:
    - name: avahi-daemon.service
    - enable: False

stop_avahi_socket:
  service.dead:
    - name: avahi-daemon.socket
    - enable: False
