# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2019 Volker Theile
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

{% set dirpath = '/srv/salt' | path_join(slspath) %}

append_fstab_entries:
  file.blockreplace:
    - name: "/etc/fstab"
    - marker_start: "# >>> [openmediavault]"
    - marker_end: "# <<< [openmediavault]"
    - content: ""
    - append_if_not_found: True
    - show_changes: True

include:
{% for file in salt['file.readdir'](dirpath) | sort %}
{% if file | regex_match('^(\d+.+).sls$', ignorecase=True) %}
  - .{{ file | replace('.sls', '') }}
{% endif %}
{% endfor %}

# Mount all filesystems without the 'bind' mount option.
{% set no_bind_mountpoints = salt['omv_conf.get_by_filter'](
  'conf.system.filesystem.mountpoint',
  {'operator': 'not', 'arg0': {'operator': 'stringContains', 'arg0': 'opts', 'arg1': 'bind'}}) %}

{% for mountpoint in no_bind_mountpoints %}
mount_no_bind_mountpoint_{{ mountpoint.uuid }}:
  mount.mounted:
    - name: {{ mountpoint.dir | escape_blank(True) | replace('\\', '\\\\') }}
    - device: {{ mountpoint.fsname | replace('\\', '\\\\') }}
    - fstype: {{ mountpoint.type }}
    - mkmnt: True
    - persist: False
    - mount: True
{% endfor %}

# Mount all filesystems with containing the 'bind' mount option.
{% set bind_mountpoints = salt['omv_conf.get_by_filter'](
  'conf.system.filesystem.mountpoint',
  {'operator': 'stringContains', 'arg0': 'opts', 'arg1': 'bind'}) %}

{% for mountpoint in bind_mountpoints %}
mount_bind_mountpoint_{{ mountpoint.uuid }}:
  mount.mounted:
    - name: {{ mountpoint.dir | escape_blank(True) | replace('\\', '\\\\') }}
    - device: {{ mountpoint.fsname | replace('\\', '\\\\') }}
    - fstype: none
    - opts: bind
    - mkmnt: True
    - persist: False
    - mount: True
{% endfor %}
