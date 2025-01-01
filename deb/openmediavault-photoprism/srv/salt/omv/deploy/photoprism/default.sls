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
# https://docs.photoprism.app/getting-started/
# https://github.com/photoprism/photoprism/blob/develop/docker-compose.latest.yml
# https://caddyserver.com/docs/caddyfile

# Testing:
# podman exec -it photoprism-app /bin/bash
# cat /config/log/nginx/error.log
# podman logs -f photoprism-app
# podman logs -f photoprism-db
# podman logs -f photoprism-proxy

{% set config = salt['omv_conf.get']('conf.service.photoprism') %}
{% set app_image = salt['pillar.get']('default:OMV_PHOTOPRISM_APP_CONTAINER_IMAGE', 'docker.io/photoprism/photoprism:latest') %}
{% set db_image = salt['pillar.get']('default:OMV_PHOTOPRISM_DB_CONTAINER_IMAGE', 'docker.io/mariadb:latest') %}
{% set proxy_image = salt['pillar.get']('default:OMV_PHOTOPRISM_PROXY_CONTAINER_IMAGE', 'docker.io/library/caddy:latest') %}
{% set ssl_enabled = config.sslcertificateref | length > 0 %}

{% if config.enable | to_bool %}

{% set appdata_sf_path = salt['omv_conf.get_sharedfolder_path'](config.appdata_sharedfolderref) %}

create_photoprism_appdata_storage_dir:
  file.directory:
    - name: "{{ appdata_sf_path }}/storage/"

create_photoprism_appdata_db_dir:
  file.directory:
    - name: "{{ appdata_sf_path }}/db/"

create_photoprism_app_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-photoprism-app.service"
    - source:
      - salt://{{ tpldir }}/files/container-photoprism-app.service.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

create_photoprism_db_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-photoprism-db.service"
    - source:
      - salt://{{ tpldir }}/files/container-photoprism-db.service.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

create_photoprism_pod_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/pod-photoprism.service"
    - source:
      - salt://{{ tpldir }}/files/pod-photoprism.service.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

photoprism_pull_app_image:
  cmd.run:
    - name: podman pull {{ app_image }}
    - unless: podman image exists {{ app_image }}
    - failhard: True

photoprism_app_image_exists:
  cmd.run:
    - name: podman image exists {{ app_image }}
    - failhard: True

photoprism_pull_db_image:
  cmd.run:
    - name: podman pull {{ db_image }}
    - unless: podman image exists {{ db_image }}
    - failhard: True

photoprism_db_image_exists:
  cmd.run:
    - name: podman image exists {{ db_image }}
    - failhard: True

{% if ssl_enabled %}

create_photoprism_proxy_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-photoprism-proxy.service"
    - source:
      - salt://{{ tpldir }}/files/container-photoprism-proxy.service.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

create_photoprism_proxy_container_caddyfile:
  file.managed:
    - name: "/var/lib/photoprism/Caddyfile"
    - source:
      - salt://{{ tpldir }}/files/Caddyfile.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

photoprism_pull_proxy_image:
  cmd.run:
    - name: podman pull {{ proxy_image }}
    - unless: podman image exists {{ proxy_image }}
    - failhard: True

photoprism_proxy_image_exists:
  cmd.run:
    - name: podman image exists {{ proxy_image }}
    - failhard: True

{% else %}

stop_photoprism_proxy_service:
  service.dead:
    - name: container-photoprism-proxy
    - enable: False

purge_photoprism_proxy_container_caddyfile:
  file.absent:
    - name: "/var/lib/photoprism/Caddyfile"

purge_photoprism_proxy_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-photoprism-proxy.service"

{% endif %}

photoprism_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

start_photoprism_service:
  service.running:
    - name: pod-photoprism
    - enable: True
    - watch:
      - file: create_photoprism_pod_systemd_unit_file
      - file: create_photoprism_app_container_systemd_unit_file
      - file: create_photoprism_db_container_systemd_unit_file

{% else %}

stop_photoprism_service:
  service.dead:
    - name: pod-photoprism
    - enable: False

remove_photoprism_proxy_container_caddyfile:
  file.absent:
    - name: "/var/lib/photoprism/Caddyfile"

remove_photoprism_app_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-photoprism-app.service"

remove_photoprism_db_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-photoprism-db.service"

remove_photoprism_proxy_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-photoprism-proxy.service"

remove_photoprism_pod_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/pod-photoprism.service"

photoprism_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% endif %}
