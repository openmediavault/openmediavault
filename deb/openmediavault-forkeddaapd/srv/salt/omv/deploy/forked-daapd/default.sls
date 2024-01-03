# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2024 Volker Theile
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

{% set config = salt['omv_conf.get']('conf.service.daapd') %}

{% if config.enable | to_bool %}

configure_forked_daapd:
  file.managed:
    - name: "/etc/forked-daapd.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-forked-daapd_conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

divert_forked_daapd:
  omv_dpkg.divert_add:
    - name: "/etc/forked-daapd.conf"

start_forked_daapd_service:
  service.running:
    - name: forked-daapd
    - enable: True
    - watch:
      - file: configure_forked_daapd

monitor_forked_daapd_service:
  module.run:
    - monit.monitor:
      - name: forked-daapd
    - require:
      - service: start_forked_daapd_service

{% else %}

unmonitor_forked_daapd_service:
  cmd.run:
    - name: monit unmonitor forked-daapd || true

stop_forked_daapd_service:
  service.dead:
    - name: forked-daapd
    - enable: False

{% endif %}
