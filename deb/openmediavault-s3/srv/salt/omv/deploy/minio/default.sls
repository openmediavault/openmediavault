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
# https://quay.io/repository/minio/minio
# https://docs.min.io/docs/minio-docker-quickstart-guide.html
# https://docs.min.io/minio/baremetal/reference/minio-server/minio-server.html
# https://caddyserver.com/docs/caddyfile

# Testing:
# podman exec -it minio-app /bin/bash
# podman logs -f minio-app
# podman logs -f minio-proxy

{% set config = salt['omv_conf.get']('conf.service.minio') %}
{% set app_image = salt['pillar.get']('default:OMV_S3_APP_CONTAINER_IMAGE', 'quay.io/minio/minio:latest') %}
{% set proxy_image = salt['pillar.get']('default:OMV_S3_PROXY_CONTAINER_IMAGE', 'docker.io/library/caddy:latest') %}
{% set ssl_enabled = config.consolesslcertificateref | length > 0 %}

{% if config.enable | to_bool %}

create_minio_app_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-minio-app.service"
    - source:
      - salt://{{ tpldir }}/files/container-minio-app.service.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

create_minio_pod_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/pod-minio.service"
    - source:
      - salt://{{ tpldir }}/files/pod-minio.service.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

minio_pull_app_image:
  cmd.run:
    - name: podman pull {{ app_image }}
    - unless: podman image exists {{ app_image }}
    - failhard: True

minio_app_image_exists:
  cmd.run:
    - name: podman image exists {{ app_image }}
    - failhard: True

{% if ssl_enabled %}

create_minio_proxy_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-minio-proxy.service"
    - source:
      - salt://{{ tpldir }}/files/container-minio-proxy.service.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

create_minio_proxy_container_caddyfile:
  file.managed:
    - name: "/var/lib/minio/Caddyfile"
    - source:
      - salt://{{ tpldir }}/files/Caddyfile.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

minio_pull_proxy_image:
  cmd.run:
    - name: podman pull {{ proxy_image }}
    - unless: podman image exists {{ proxy_image }}
    - failhard: True

minio_proxy_image_exists:
  cmd.run:
    - name: podman image exists {{ proxy_image }}
    - failhard: True

{% else %}

stop_minio_proxy_service:
  service.dead:
    - name: container-minio-proxy
    - enable: False

purge_minio_proxy_container_caddyfile:
  file.absent:
    - name: "/var/lib/minio/Caddyfile"

purge_minio_proxy_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-minio-proxy.service"

{% endif %}

minio_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

start_minio_service:
  service.running:
    - name: pod-minio
    - enable: True
    - watch:
      - file: create_minio_pod_systemd_unit_file
      - file: create_minio_app_container_systemd_unit_file

{% else %}

stop_minio_service:
  service.dead:
    - name: pod-minio
    - enable: False

remove_minio_proxy_container_caddyfile:
  file.absent:
    - name: "/var/lib/minio/Caddyfile"

remove_minio_app_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-minio-app.service"

remove_minio_proxy_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-minio-proxy.service"

remove_minio_pod_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/pod-minio.service"

minio_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% endif %}
