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

# Testing:
# ls -la /sharedfolders
# salt-call --local --retcode-passthrough --no-color state.apply omv.task.apply

{% set data = salt['pillar.get']('data', None) %}
{% set sharedfolders_path = salt['pillar.get']('default:OMV_SHAREDFOLDERS_DIR', '/sharedfolders') %}
{% set sharedfolders_dir_enabled = salt['pillar.get']('default:OMV_SHAREDFOLDERS_DIR_ENABLED', 'no') %}

{% if sharedfolders_dir_enabled | to_bool and data is not none %}

{% set link_path = sharedfolders_path | path_join(data.name) %}

# Only remove the entry if it is a symlink, never delete real data.
remove_sharedfolder_{{ data.name }}_symlink:
  file.absent:
    - name: "{{ link_path }}"
    - onlyif: test -L {{ link_path | quote }}

{% else %}

# Prevent empty rendering.
disable_sharedfolder_alias:
  test.nop

{% endif %}
