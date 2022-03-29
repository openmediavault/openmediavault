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
# https://docs.photoprism.app/getting-started/
# https://github.com/photoprism/photoprism/blob/develop/docker-compose.latest.yml

# Testing:
# podman exec -it photoprism-app /bin/bash
# cat /config/log/nginx/error.log
# podman logs -f photoprism-app
# podman logs -f photoprism-db
# podman logs -f photoprism-proxy

{% set config = salt['omv_conf.get']('conf.service.photoprism') %}
{% set appdata_sf_path = salt['omv_conf.get_sharedfolder_path'](config.appdata_sharedfolderref) %}

create_photoprism_appdata_storage_dir:
  file.directory:
    - name: "{{ appdata_sf_path }}/storage/"

create_photoprism_appdata_db_dir:
  file.directory:
    - name: "{{ appdata_sf_path }}/db/"

create_photoprism_appdata_proxy_data_dir:
  file.directory:
    - name: "{{ appdata_sf_path }}/proxy/data/"

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

photoprism_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% if config.enable | to_bool %}

{% set app_image = salt['pillar.get']('default:OMV_PHOTOPRISM_APP_CONTAINER_IMAGE', 'docker.io/photoprism/photoprism:latest') %}
{% set db_image = salt['pillar.get']('default:OMV_PHOTOPRISM_DB_CONTAINER_IMAGE', 'docker.io/mariadb:latest') %}
{% set proxy_image = salt['pillar.get']('default:OMV_PHOTOPRISM_PROXY_CONTAINER_IMAGE', 'docker.io/library/caddy:latest') %}

photoprism_pull_app_image:
  cmd.run:
    - name: podman pull {{ app_image }}
    - unless: podman image exists {{ app_image }}
    - failhard: True

photoprism_pull_db_image:
  cmd.run:
    - name: podman pull {{ db_image }}
    - unless: podman image exists {{ db_image }}
    - failhard: True

photoprism_pull_proxy_image:
  cmd.run:
    - name: podman pull {{ proxy_image }}
    - unless: podman image exists {{ proxy_image }}
    - failhard: True

configure_caddyfile:
  file.managed:
    - name: "{{ appdata_sf_path }}/proxy/Caddyfile"
    - source:
      - salt://{{ tpldir }}/files/Caddyfile.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

start_photoprism_service:
  service.running:
    - name: pod-photoprism
    - enable: True
    - watch:
      - file: create_photoprism_pod_systemd_unit_file
      - file: create_photoprism_app_container_systemd_unit_file
      - file: create_photoprism_db_container_systemd_unit_file
      - file: create_photoprism_proxy_container_systemd_unit_file
      - file: configure_caddyfile

{% else %}

stop_photoprism_service:
  service.dead:
    - name: pod-photoprism
    - enable: False

{% endif %}
