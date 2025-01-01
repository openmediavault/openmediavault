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

{% set interfaces = [] %}
{% for interface in salt['omv_conf.get_by_filter'](
  'conf.system.network.interface',
  {'operator': 'or', 'arg0': {'operator': 'stringEquals', 'arg0': 'type', 'arg1': 'ethernet'}, 'arg1': {'operator': 'stringEquals', 'arg0': 'type', 'arg1': 'wifi'}}) %}
{% set used_by = salt['omv_conf.get_by_filter'](
  'conf.system.network.interface',
  {'operator': 'stringContains', 'arg0': 'slaves', 'arg1': interface.devicename}) %}
{% if used_by | length == 0 %}
{% set _ = interfaces.append(interface) %}
{% endif %}
{% endfor %}
{% set interfaces = interfaces + salt['omv_conf.get_by_filter'](
  'conf.system.network.interface',
  {'operator': 'stringEnum', 'arg0': 'type', 'arg1': ['bond', 'bridge', 'vlan']}) %}

configure_collectd_conf_interface_plugin:
  file.managed:
    - name: "/etc/collectd/collectd.conf.d/interface.conf"
    - source:
      - salt://{{ tpldir }}/files/collectd-interface.j2
    - template: jinja
    - context:
        interfaces: {{ interfaces | json }}
