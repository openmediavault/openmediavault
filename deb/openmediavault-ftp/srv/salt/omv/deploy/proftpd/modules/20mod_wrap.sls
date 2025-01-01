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

{% set tcp_access_files = salt['pillar.get']('default:OMV_PROFTPD_MODWRAP_TCPACCESSFILES', '/etc/hosts.allow /etc/hosts.deny') %}
{% set tcp_access_syslog_levels = salt['pillar.get']('default:OMV_PROFTPD_MODWRAP_TCPACCESSSYSLOGLEVELS', 'info warn') %}
{% set tcp_service_name = salt['pillar.get']('default:OMV_PROFTPD_MODWRAP_TCPSERVICENAME', 'ftpd') %}

configure_proftpd_mod_wrap:
  file.append:
    - name: "/etc/proftpd/proftpd.conf"
    - text: |
        <IfModule mod_wrap.c>
          TCPAccessFiles {{ tcp_access_files }}
          TCPAccessSyslogLevels {{ tcp_access_syslog_levels }}
          TCPServiceName {{ tcp_service_name }}
        </IfModule>

{% for host_file in tcp_access_files.split() %}

# Make sure the file exists. Do not use file.touch state.
create_proftpd_hosts_file_{{ host_file | replace('/', '-') }}:
  file.managed:
    - name: {{ host_file }}
    - user: root
    - group: root
    - mode: 644
    - replace: False

{% endfor %}
