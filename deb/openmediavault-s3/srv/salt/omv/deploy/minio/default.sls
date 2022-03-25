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
# https://hub.docker.com/r/minio/minio/

# Testing:
# podman exec -it minio /bin/bash
# podman logs -f minio

{% set config = salt['omv_conf.get']('conf.service.minio') %}

create_minio_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-minio.service"
    - source:
      - salt://{{ tpldir }}/files/container-minio.service.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

minio_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% if config.enable | to_bool %}

{% set image = salt['pillar.get']('default:OMV_S3_APP_CONTAINER_IMAGE', 'docker.io/minio/minio:latest') %}

minio_pull_app_image:
  cmd.run:
    - name: podman pull {{ image }}
    - unless: podman image exists {{ image }}

start_minio_service:
  service.running:
    - name: container-minio
    - enable: True
    - watch:
      - file: create_minio_container_systemd_unit_file

{% else %}

stop_minio_service:
  service.dead:
    - name: container-minio
    - enable: False

{% endif %}
