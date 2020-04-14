# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2020 Volker Theile
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
    - name: "/etc/systemd/network/10-openmediavault-{{ interface.devicename }}.netdev"
    - source:
      - salt://{{ tpldir }}/files/vlan_netdev.j2
    - template: jinja
    - context:
        interface: {{ interface | json }}
    - user: root
    - group: root
    - mode: 644

configure_interface_vlan_{{ interface.devicename }}_network:
  file.managed:
    - name: "/etc/systemd/network/10-openmediavault-{{ interface.devicename }}.network"
    - source:
      - salt://{{ tpldir }}/files/vlan_id_network.j2
    - template: jinja
    - context:
        interface: {{ interface | json }}
    - user: root
    - group: root
    - mode: 644

{% endfor %}

{% for devicename in interfaces | map(attribute='vlanrawdevice') | unique %}

# Populate the <DEVICE>.network file. The 'ini.options_present' state
# could not be used here because the 'VLAN' option might appear multiple
# times in the [Network] section.
# 1. Make sure the file exists (if this was not done already).
# 2. Get the options from the [Network] section.
# 3. Purge the [Network] section.
# 4. Write the origin [Network] section options and append the
#    additional 'VLAN' options.
pre_configure_interface_vlan_{{ devicename }}_network:
  file.managed:
    - name: "/etc/systemd/network/10-openmediavault-{{ devicename }}.network"
    - user: root
    - group: root
    - mode: 644
{% set options = salt['ini.get_section'](
  '/etc/systemd/network/10-openmediavault-' ~ devicename ~ '.network', 'Network') %}
configure_interface_vlan_{{ devicename }}_network:
  ini.sections_absent:
    - name: "/etc/systemd/network/10-openmediavault-{{ devicename }}.network"
    - separator: "="
    - sections:
        - Network
  file.append:
    - name: "/etc/systemd/network/10-openmediavault-{{ devicename }}.network"
    - text: |

        [Network]
        {%- for key, value in options.items() %}
        {{ key }}={{ value }}
        {%- endfor %}
        {%- for interface in interfaces %}
        VLAN={{ interface.devicename }}
        {%- endfor %}

{% endfor %}
