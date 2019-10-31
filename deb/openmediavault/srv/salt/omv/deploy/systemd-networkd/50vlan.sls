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

# Documentation/Howto:
# https://wiki.archlinux.org/index.php/Systemd-networkd
# http://enricorossi.org/blog/2017/systemd_network_vlan_interface_up/

{% set interfaces = salt['omv_conf.get_by_filter'](
  'conf.system.network.interface',
  {'operator': 'stringEquals', 'arg0': 'type', 'arg1': 'vlan'}) %}

{% for interface in interfaces %}

configure_interface_vlan_{{ interface.devicename }}_netdev:
  file.managed:
    - name: "/etc/systemd/network/90-openmediavault-{{ interface.devicename }}.netdev"
    - source:
      - salt://{{ slspath }}/files/vlan_netdev.j2
    - template: jinja
    - context:
        interface: {{ interface | json }}
    - user: root
    - group: root
    - mode: 644

configure_interface_vlan_{{ interface.devicename }}_network:
  file.managed:
    - name: "/etc/systemd/network/90-openmediavault-{{ interface.devicename }}.network"
    - source:
      - salt://{{ slspath }}/files/vlan_id_network.j2
    - template: jinja
    - context:
        interface: {{ interface | json }}
    - user: root
    - group: root
    - mode: 644

{% endfor %}

{% for devicename in interfaces | map(attribute='vlanrawdevice') | unique %}

configure_interface_vlan_{{ devicename }}_network:
  file.managed:
    - name: "/etc/systemd/network/90-openmediavault-{{ devicename }}.network"
    - source:
      - salt://{{ slspath }}/files/vlan_network.j2
    - template: jinja
    - context:
        interfaces: {{ interfaces | selectattr('vlanrawdevice', 'equalto', devicename) | list | json }}
    - user: root
    - group: root
    - mode: 644

{% endfor %}
