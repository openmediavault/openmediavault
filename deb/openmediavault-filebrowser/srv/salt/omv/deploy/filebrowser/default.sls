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
# https://filebrowser.org/installation
# https://github.com/filebrowser/filebrowser/blob/master/Dockerfile

# Testing:
# podman exec -it filebrowser /bin/bash
# podman logs -f filebrowser

{% set config = salt['omv_conf.get']('conf.service.filebrowser') %}

{% if config.enable | to_bool %}

{% set image = salt['pillar.get']('default:OMV_FILEBROWSER_APP_CONTAINER_IMAGE', 'docker.io/filebrowser/filebrowser:latest') %}
{% set uname = salt['pillar.get']('default:OMV_FILEBROWSER_APP_CONTAINER_UNAME', 'filebrowser') %}
{% set gname = salt['pillar.get']('default:OMV_FILEBROWSER_APP_CONTAINER_GNAME', 'users') %}

create_filebrowser_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-filebrowser.service"
    - source:
      - salt://{{ tpldir }}/files/container-filebrowser.service.j2
    - template: jinja
    - context:
        config: {{ config }}
    - user: root
    - group: root
    - mode: 644

filebrowser_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

filebrowser_pull_app_image:
  cmd.run:
    - name: podman pull {{ image }}
    - unless: podman image exists {{ image }}
    - failhard: True

filebrowser_app_image_exists:
  cmd.run:
    - name: podman image exists {{ image }}
    - failhard: True

# Make sure the file exists. Do not use file.touch state.
create_filebrowser_database:
  file.managed:
    - name: "/var/cache/filebrowser/database.db"
    - user: {{ uname }}
    - group: {{ gname }}
    - mode: 644
    - replace: False

configure_filebrowser:
  file.managed:
    - name: "/var/cache/filebrowser/filebrowser.json"
    - source:
      - salt://{{ tpldir }}/files/filebrowser.json.j2
    - template: jinja
    - context:
        config: {{ config }}
    - user: {{ uname }}
    - group: {{ gname }}
    - mode: 644

start_filebrowser_service:
  service.running:
    - name: container-filebrowser
    - enable: True
    - watch:
      - file: configure_filebrowser
      - file: create_filebrowser_container_systemd_unit_file

{% else %}

stop_filebrowser_service:
  service.dead:
    - name: container-filebrowser
    - enable: False

remove_filebrowser_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-filebrowser.service"

filebrowser_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% endif %}
