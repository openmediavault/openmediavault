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

{% set notification_config = salt['omv_conf.get_by_filter'](
  'conf.system.notification.notification',
  {'operator': 'stringEquals', 'arg0': 'id', 'arg1': 'monitfilesystems'})[0] %}

{% if notification_config.enable | to_bool %}

{% set mountpoints = salt['omv_conf.get_by_filter'](
  'conf.system.filesystem.mountpoint',
  {'operator': 'and', 'arg0': {'operator': 'equals', 'arg0': 'hidden', 'arg1': '0'}, 'arg1': {'operator': 'not', 'arg0': {'operator': 'or', 'arg0': {'operator': 'stringContains', 'arg0': 'opts', 'arg1': 'bind'}, 'arg1': {'operator': 'stringContains', 'arg0': 'opts', 'arg1': 'loop'}}}}) %}

configure_monit_filesystem_service:
  file.managed:
    - name: "/etc/monit/conf.d/openmediavault-filesystem.conf"
    - source:
      - salt://{{ tpldir }}/files/filesystem.j2
    - template: jinja
    - context:
        mountpoints: {{ mountpoints | json }}
    - user: root
    - group: root
    - mode: 644

{% endif %}
