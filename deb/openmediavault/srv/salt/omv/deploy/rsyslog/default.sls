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

{% set config = salt['omv_conf.get']('conf.system.syslog.remote') %}

configure_rsyslog_main:
  file.managed:
    - name: "/etc/rsyslog.conf"
    - source:
      - salt://{{ tpldir }}/files/etc_rsyslog.conf.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

divert_rsyslog_main:
  omv_dpkg.divert_add:
    - name: "/etc/rsyslog.conf"

configure_rsyslog_remote:
  file.managed:
    - name: "/etc/rsyslog.d/openmediavault-remote.conf"
    - source:
      - salt://{{ tpldir }}/files/etc_rsyslog.d_openmediavault-remote.conf.j2
    - context:
        config: {{ config | json }}
    - template: jinja
    - user: root
    - group: root
    - mode: 644

restart_rsyslog_service:
  service.running:
    - name: rsyslog
    - enable: True
    - watch:
      - file: configure_rsyslog_main
      - file: configure_rsyslog_remote
