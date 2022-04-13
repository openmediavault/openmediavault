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
# https://github.com/abraunegg/onedrive
# https://github.com/abraunegg/onedrive/blob/master/docs/Docker.md

# Testing:
# podman logs -f onedrive

{% set config = salt['omv_conf.get']('conf.service.onedrive') %}

{% if config.enable | to_bool %}

{% set image = salt['pillar.get']('default:OMV_ONEDRIVE_APP_CONTAINER_IMAGE', 'docker.io/driveone/onedrive:latest') %}
{% set uname = salt['pillar.get']('default:OMV_ONEDRIVE_APP_CONTAINER_UNAME', 'onedrive') %}
{% set gname = salt['pillar.get']('default:OMV_ONEDRIVE_APP_CONTAINER_GNAME', 'users') %}

create_onedrive_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-onedrive.service"
    - source:
      - salt://{{ tpldir }}/files/container-onedrive.service.j2
    - template: jinja
    - context:
        config: {{ config }}
    - user: root
    - group: root
    - mode: 644

setup_onedrive_conf_dir:
  module.run:
    - file.chown:
      - path: "/var/cache/onedrive/"
      - user: {{ uname }}
      - group: {{ gname }}

onedrive_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

onedrive_pull_app_image:
  cmd.run:
    - name: podman pull {{ image }}
    - unless: podman image exists {{ image }}
    - failhard: True

onedrive_app_image_exists:
  cmd.run:
    - name: podman image exists {{ image }}
    - failhard: True

start_onedrive_service:
  service.running:
    - name: container-onedrive
    - enable: True
    - watch:
      - file: create_onedrive_container_systemd_unit_file

{% else %}

stop_onedrive_service:
  service.dead:
    - name: container-onedrive
    - enable: False

remove_onedrive_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-onedrive.service"

onedrive_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% endif %}
