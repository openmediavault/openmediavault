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
# https://owntone.github.io/owntone-server/configuration/
# https://github.com/owntone/owntone-container

# Testing:
# podman exec -it owntone /bin/bash
# podman logs -f owntone

{% set config = salt['omv_conf.get']('conf.service.owntone') %}

setup_owntone_config_dir:
  file.directory:
    - name: "/var/cache/owntone/config"
    - makedirs: True
    - user: "owntone"
    - recurse:
      - user

setup_owntone_db_dir:
  file.directory:
    - name: "/var/cache/owntone/db"
    - makedirs: True
    - user: "owntone"
    - recurse:
      - user

{% if config.enable | to_bool %}

{% set image = salt['pillar.get']('default:OMV_OWNTONE_APP_CONTAINER_IMAGE', 'docker.io/owntone/owntone:latest') %}

create_owntone_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-owntone.service"
    - source:
      - salt://{{ tpldir }}/files/container-owntone.service.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

owntone_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

owntone_pull_app_image:
  cmd.run:
    - name: podman pull {{ image }}
    - unless: podman image exists {{ image }}
    - failhard: True

owntone_app_image_exists:
  cmd.run:
    - name: podman image exists {{ image }}
    - failhard: True

configure_owntone:
  file.managed:
    - name: "/var/cache/owntone/config/owntone.conf"
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

remove_owntone_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-owntone.service"

owntone_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% endif %}
