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

{% set socket_file = salt['pillar.get']('default:OMV_COLLECTD_UNIXSOCK_SOCKETFILE', '/run/collectd.socket') %}
{% set socket_group = salt['pillar.get']('default:OMV_COLLECTD_UNIXSOCK_SOCKETGROUP', 'root') %}
{% set socket_perms = salt['pillar.get']('default:OMV_COLLECTD_UNIXSOCK_SOCKETPERMS', '0660') %}

configure_collectd_conf_unixsock_plugin:
  file.managed:
    - name: "/etc/collectd/collectd.conf.d/unixsock.conf"
    - contents: |
        LoadPlugin unixsock
        <Plugin unixsock>
            SocketFile "{{ socket_file }}"
            SocketGroup "{{ socket_group }}"
            SocketPerms "{{ socket_perms }}"
        </Plugin>
