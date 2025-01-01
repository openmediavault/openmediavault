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

{% set config = salt['omv_conf.get']('conf.service.shairport') %}

{% if config.enable | to_bool %}

configure_shairport_sync:
  file.managed:
    - name: "/etc/shairport-sync.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-shairport-sync_conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

divert_shairport_sync:
  omv_dpkg.divert_add:
    - name: "/etc/shairport-sync.conf"

start_shairport_sync_service:
  service.running:
    - name: shairport-sync
    - enable: True
    - watch:
      - file: configure_shairport_sync

monitor_shairport_sync_service:
  module.run:
    - monit.monitor:
      - name: shairport-sync
    - require:
      - service: start_shairport_sync_service

{% else %}

unmonitor_shairport_sync_service:
  cmd.run:
    - name: monit unmonitor shairport-sync || true

stop_shairport_sync_service:
  service.dead:
    - name: shairport-sync
    - enable: False

{% endif %}
