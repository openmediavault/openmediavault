# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2026 Volker Theile
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.

# Documentation/Howto:
# https://github.com/butlerx/wetty
# https://hub.docker.com/r/wettyoss/wetty
# https://caddyserver.com/docs/caddyfile

# Testing:
# podman exec -it wetty-app /bin/bash
# podman logs -f wetty-app
# podman logs -f wetty-proxy

{% set wetty_config = salt['omv_conf.get']('conf.service.wetty') %}
{% set app_image = salt['pillar.get']('default:OMV_WETTY_APP_CONTAINER_IMAGE', 'docker.io/wettyoss/wetty:latest') %}
{% set proxy_image = salt['pillar.get']('default:OMV_WETTY_PROXY_CONTAINER_IMAGE', 'docker.io/library/caddy:latest') %}

{% if wetty_config.enable | to_bool %}

{% set ssh_config = salt['omv_conf.get']('conf.service.ssh') %}

create_wetty_app_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-wetty-app.service"
    - source:
      - salt://{{ tpldir }}/files/container-wetty-app.service.j2
    - template: jinja
    - context:
        wetty_config: {{ wetty_config | json }}
        ssh_config: {{ ssh_config | json }}
    - user: root
    - group: root
    - mode: '0644'

create_wetty_proxy_container_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/container-wetty-proxy.service"
    - source:
      - salt://{{ tpldir }}/files/container-wetty-proxy.service.j2
    - template: jinja
    - context:
        wetty_config: {{ wetty_config | json }}
    - user: root
    - group: root
    - mode: '0644'

create_wetty_pod_systemd_unit_file:
  file.managed:
    - name: "/etc/systemd/system/pod-wetty.service"
    - source:
      - salt://{{ tpldir }}/files/pod-wetty.service.j2
    - template: jinja
    - context:
        wetty_config: {{ wetty_config | json }}
    - user: root
    - group: root
    - mode: '0644'

setup_wetty_dir:
  file.directory:
    - name: "/var/lib/wetty/"
    - user: root
    - group: root
    - mode: '0755'

create_wetty_proxy_container_caddyfile:
  file.managed:
    - name: "/var/lib/wetty/Caddyfile"
    - source:
      - salt://{{ tpldir }}/files/Caddyfile.j2
    - template: jinja
    - context:
        config: {{ wetty_config | json }}
    - user: root
    - group: root
    - mode: '0644'
    - require:
      - file: setup_wetty_dir

wetty_pull_app_image:
  cmd.run:
    - name: podman pull {{ app_image }}
    - unless: podman image exists {{ app_image }}
    - failhard: True

wetty_app_image_exists:
  cmd.run:
    - name: podman image exists {{ app_image }}
    - failhard: True

wetty_pull_proxy_image:
  cmd.run:
    - name: podman pull {{ proxy_image }}
    - unless: podman image exists {{ proxy_image }}
    - failhard: True

wetty_proxy_image_exists:
  cmd.run:
    - name: podman image exists {{ proxy_image }}
    - failhard: True

wetty_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

start_wetty_service:
  service.running:
    - name: pod-wetty
    - enable: True
    - watch:
      - file: create_wetty_proxy_container_caddyfile
      - file: create_wetty_pod_systemd_unit_file
      - file: create_wetty_app_container_systemd_unit_file
      - file: create_wetty_proxy_container_systemd_unit_file

{% else %}

stop_wetty_service:
  service.dead:
    - name: pod-wetty
    - enable: False

remove_wetty_proxy_container_caddyfile:
  file.absent:
    - name: "/var/lib/wetty/Caddyfile"

remove_wetty_app_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-wetty-app.service"

remove_wetty_proxy_container_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/container-wetty-proxy.service"

remove_wetty_pod_systemd_unit_file:
  file.absent:
    - name: "/etc/systemd/system/pod-wetty.service"

wetty_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% endif %}
