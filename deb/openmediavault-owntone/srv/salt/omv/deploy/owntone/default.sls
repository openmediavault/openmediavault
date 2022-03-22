# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2022 Volker Theile
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

# Documentation/Howto:
# https://fleet.linuxserver.io/image?name=linuxserver/daapd

# Testing:
# podman exec -it daapd /bin/bash
# podman logs -f daapd

{% set config = salt['omv_conf.get']('conf.service.owntone') %}
{% set time_config = salt['omv_conf.get']('conf.system.time') %}
{% set sf_path = salt['omv_conf.get_sharedfolder_path'](config.sharedfolderref) %}

create_owntone_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-owntone.service"
    - source:
      - salt://{{ tpldir }}/files/container-owntone.service.j2
    - template: jinja
    - context:
        sf_path: {{ sf_path }}
        timezone: {{ time_config.timezone }}
    - user: root
    - group: root
    - mode: 644

owntone_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% if config.enable | to_bool %}

configure_owntone:
  file.managed:
    - name: "/var/cache/owntone/owntone.conf"
    - source:
      - salt://{{ tpldir }}/files/owntone.conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

start_owntone_service:
  service.running:
    - name: container-owntone
    - enable: True
    - watch:
      - file: configure_owntone
      - file: create_owntone_container_systemd_unit_file

{% else %}

stop_owntone_service:
  service.dead:
    - name: container-owntone
    - enable: False

{% endif %}
