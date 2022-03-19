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

{% set config = salt['omv_conf.get']('conf.service.photoprism') %}
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
        port: {{ config.port }}
    - user: root
    - group: root
    - mode: 644

photoprism_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% if config.enable | to_bool %}

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

{% endif %}
