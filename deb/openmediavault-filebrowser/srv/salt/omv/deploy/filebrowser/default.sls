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
# https://filebrowser.org/installation
# https://github.com/filebrowser/filebrowser/blob/master/Dockerfile
# https://github.com/filebrowser/filebrowser/blob/master/docker/root/defaults/settings.json
# https://caddyserver.com/docs/caddyfile

# Testing:
# podman exec -it filebrowser-app /bin/bash
# podman logs -f filebrowser-app
# podman logs -f filebrowser-proxy

{% set config = salt['omv_conf.get']('conf.service.filebrowser') %}
{% set app_image = salt['pillar.get']('default:OMV_FILEBROWSER_APP_CONTAINER_IMAGE', 'docker.io/filebrowser/filebrowser:latest') %}
{% set proxy_image = salt['pillar.get']('default:OMV_FILEBROWSER_PROXY_CONTAINER_IMAGE', 'docker.io/library/caddy:latest') %}
{% set uname = salt['pillar.get']('default:OMV_FILEBROWSER_APP_CONTAINER_UNAME', 'filebrowser') %}
{% set gname = salt['pillar.get']('default:OMV_FILEBROWSER_APP_CONTAINER_GNAME', 'users') %}
{% set ssl_enabled = config.sslcertificateref | length > 0 %}

{% if config.enable | to_bool %}

create_filebrowser_app_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-filebrowser-app.service"
    - source:
      - salt://{{ tpldir }}/files/container-filebrowser-app.service.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

create_filebrowser_pod_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/pod-filebrowser.service"
    - source:
      - salt://{{ tpldir }}/files/pod-filebrowser.service.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

filebrowser_pull_app_image:
  cmd.run:
    - name: podman pull {{ app_image }}
    - unless: podman image exists {{ app_image }}
    - failhard: True

filebrowser_app_image_exists:
  cmd.run:
    - name: podman image exists {{ app_image }}
    - failhard: True

# Make sure the file exists. Do not use "file.touch" state.
create_filebrowser_database:
  file.managed:
    - name: "/var/lib/filebrowser/database.db"
    - user: {{ uname }}
    - group: {{ gname }}
    - mode: 644
    - replace: False

configure_filebrowser:
  file.managed:
    - name: "/var/lib/filebrowser/filebrowser.json"
    - source:
      - salt://{{ tpldir }}/files/filebrowser.json.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: {{ uname }}
    - group: {{ gname }}
    - mode: 644

{% if ssl_enabled %}

create_filebrowser_proxy_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-filebrowser-proxy.service"
    - source:
      - salt://{{ tpldir }}/files/container-filebrowser-proxy.service.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

create_filebrowser_proxy_container_caddyfile:
  file.managed:
    - name: "/var/lib/filebrowser/Caddyfile"
    - source:
      - salt://{{ tpldir }}/files/Caddyfile.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

filebrowser_pull_proxy_image:
  cmd.run:
    - name: podman pull {{ proxy_image }}
    - unless: podman image exists {{ proxy_image }}
    - failhard: True

filebrowser_proxy_image_exists:
  cmd.run:
    - name: podman image exists {{ proxy_image }}
    - failhard: True

{% else %}

stop_filebrowser_proxy_service:
  service.dead:
    - name: container-filebrowser-proxy
    - enable: False

purge_filebrowser_proxy_container_caddyfile:
  file.absent:
    - name: "/var/lib/filebrowser/Caddyfile"

purge_filebrowser_proxy_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-filebrowser-proxy.service"

{% endif %}

filebrowser_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

start_filebrowser_service:
  service.running:
    - name: pod-filebrowser
    - enable: True
    - watch:
      - file: configure_filebrowser
      - file: create_filebrowser_pod_systemd_unit_file
      - file: create_filebrowser_app_container_systemd_unit_file

{% else %}

stop_filebrowser_service:
  service.dead:
    - name: pod-filebrowser
    - enable: False

remove_filebrowser_proxy_container_caddyfile:
  file.absent:
    - name: "/var/lib/filebrowser/Caddyfile"

remove_filebrowser_app_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-filebrowser-app.service"

remove_filebrowser_proxy_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-filebrowser-proxy.service"

remove_filebrowser_pod_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/pod-filebrowser.service"

filebrowser_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% endif %}
