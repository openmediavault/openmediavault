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

{% set mountpoints = salt['omv_conf.get_by_filter'](
  'conf.system.filesystem.mountpoint',
  {'operator': 'and', 'arg0': {'operator': 'equals', 'arg0': 'hidden', 'arg1': '0'}, 'arg1': {'operator': 'stringContains', 'arg0': 'opts', 'arg1': 'bind'}}) %}

{% for mountpoint in mountpoints %}
create_bind_mountpoint_{{ mountpoint.uuid }}:
  file.accumulated:
    - filename: "/etc/fstab"
    - text: "{{ mountpoint.fsname | escape_blank(True) | replace('\\', '\\\\') }}\t\t{{ mountpoint.dir | escape_blank(True) | replace('\\', '\\\\') }}\t{{ mountpoint.type }}\t{{ mountpoint.opts }}\t{{ mountpoint.freq }} {{ mountpoint.passno }}"
    - require_in:
      - file: append_fstab_entries

mount_bind_mountpoint_{{ mountpoint.uuid }}:
  mount.mounted:
    - name: {{ mountpoint.dir | escape_blank(True) }}
    - device: {{ mountpoint.fsname | replace('\\', '\\\\') }}
    - fstype: none
    - opts: bind
    - mkmnt: True
    - persist: False
    - mount: True
{% endfor %}
