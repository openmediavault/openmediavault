# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2023 Volker Theile
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

{% set config = salt['omv_conf.get']('conf.service.onedrive') %}

{% if config.enable | to_bool %}

setup_onedrive_config_dir:
  module.run:
    - file.chown:
      - path: "/var/cache/onedrive/"
      - user: onedrive
      - group: users

create_onedrive_config_file:
  file.managed:
    - name: "/var/cache/onedrive/config"
    - source:
      - salt://{{ tpldir }}/files/config.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: onedrive
    - group: users
    - mode: 644

start_onedrive_service:
  service.running:
    - name: onedrive
    - enable: True
    - watch:
      - file: create_onedrive_config_file

{% else %}

stop_onedrive_service:
  service.dead:
    - name: onedrive
    - enable: False

{% endif %}
