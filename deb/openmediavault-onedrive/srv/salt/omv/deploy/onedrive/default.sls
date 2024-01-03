# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2024 Volker Theile
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
# https://github.com/abraunegg/onedrive/blob/master/docs/USAGE.md

# Testing
# onedrive --confdir /var/cache/onedrive --display-config
# systemctl show onedrive.service

{% set config = salt['omv_conf.get']('conf.service.onedrive') %}

{% if config.enable | to_bool %}

{% set sf_mnt_path = salt['omv_conf.get_sharedfolder_mount_path'](config.sharedfolderref) %}

setup_onedrive_config_dir:
  file.directory:
    - name: "/var/cache/onedrive/"
    - user: "{{ config.username }}"
    - recurse:
      - user

setup_onedrive_log_dir:
  file.directory:
    - name: "/var/log/onedrive/"
    - user: "{{ config.username }}"
    - recurse:
      - user

create_onedrive_config_file:
  file.managed:
    - name: "/var/cache/onedrive/config"
    - source:
      - salt://{{ tpldir }}/files/config.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: "{{ config.username }}"
    - mode: 644

create_onedrive_systemd_conf:
  file.managed:
    - name: "/etc/systemd/system/onedrive.service.d/openmediavault.conf"
    - contents: |
        [Unit]
        RequiresMountsFor="{{ sf_mnt_path }}"

        [Service]
        User={{ config.username }}
    - makedirs: True
    - mode: 644

onedrive_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:
    - onchanges:
      - file: create_onedrive_systemd_conf

start_onedrive_service:
  service.running:
    - name: onedrive
    - enable: True
    - watch:
      - file: create_onedrive_config_file
      - file: create_onedrive_systemd_conf

{% else %}

stop_onedrive_service:
  service.dead:
    - name: onedrive
    - enable: False

{% endif %}
