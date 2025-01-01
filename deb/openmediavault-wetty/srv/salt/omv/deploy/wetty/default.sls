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

# Documentation/Howto:
# https://github.com/butlerx/wetty
# https://hub.docker.com/r/wettyoss/wetty

# Testing:
# podman exec -it wetty /bin/bash
# podman logs -f wetty

{% set wetty_config = salt['omv_conf.get']('conf.service.wetty') %}

{% if wetty_config.enable | to_bool %}

{% set image = salt['pillar.get']('default:OMV_WETTY_APP_CONTAINER_IMAGE', 'docker.io/wettyoss/wetty:latest') %}
{% set dns_config = salt['omv_conf.get']('conf.system.network.dns') %}
{% set ssh_config = salt['omv_conf.get']('conf.service.ssh') %}

create_wetty_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-wetty.service"
    - source:
      - salt://{{ tpldir }}/files/container-wetty.service.j2
    - template: jinja
    - context:
        wetty_config: {{ wetty_config | json }}
        dns_config: {{ dns_config | json }}
        ssh_config: {{ ssh_config | json }}
    - user: root
    - group: root
    - mode: 644

wetty_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

wetty_pull_app_image:
  cmd.run:
    - name: podman pull {{ image }}
    - unless: podman image exists {{ image }}
    - failhard: True

wetty_app_image_exists:
  cmd.run:
    - name: podman image exists {{ image }}
    - failhard: True

start_wetty_service:
  service.running:
    - name: container-wetty
    - enable: True
    - watch:
      - file: create_wetty_container_systemd_unit_file

{% else %}

stop_wetty_service:
  service.dead:
    - name: container-wetty
    - enable: False

remove_wetty_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-wetty.service"

wetty_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% endif %}
