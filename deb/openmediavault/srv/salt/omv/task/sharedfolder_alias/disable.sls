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

# Testing:
# systemctl status sharedfolders-test.mount
# salt-call --local --retcode-passthrough --no-color state.apply omv.task.apply

{% set data = salt['pillar.get']('data', None) %}
{% set sharedfolders_path = salt['pillar.get']('default:OMV_SHAREDFOLDERS_DIR', '/sharedfolders') %}
{% set sharedfolders_dir_enabled = salt['pillar.get']('default:OMV_SHAREDFOLDERS_DIR_ENABLED', 'no') %}

{% if sharedfolders_dir_enabled | to_bool %}

{% if data is not none %}

{% set dir_path = sharedfolders_path | path_join(data.name) %}
{% set unit_name = salt['cmd.run']('systemd-escape --path --suffix=mount ' ~ dir_path) %}

disable_sharedfolder_{{ data.name }}_mount_unit:
  service.dead:
    - name: {{ unit_name }}
    - enable: False

remove_sharedfolder_{{ data.name }}_mount_point:
  file.absent:
    - name: {{ dir_path }}
    - unless: "mountpoint -q '{{ dir_path }}'"

{% endif %}

{% else %}

# Prevent empty rendering.
disable_sharedfolder_alias:
  test.nop

{% endif %}
