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
# https://wiki.debian.org/Bonding
# https://forum.manjaro.org/t/how-to-create-lacp-802-3ad-bond-using-systemd-networkd/14655
# https://kerlilow.me/blog/setting-up-systemd-networkd-with-bonding/

{% set interfaces = salt['omv_conf.get_by_filter'](
  'conf.system.network.interface',
  {'operator': 'stringEquals', 'arg0': 'type', 'arg1': 'bond'}) %}

{% for interface in interfaces %}

configure_interface_bond_{{ interface.devicename }}_netdev:
  file.managed:
    - name: "/etc/systemd/network/10-openmediavault-{{ interface.devicename }}.netdev"
    - source:
      - salt://{{ tpldir }}/files/bond_netdev.j2
    - template: jinja
    - context:
        interface: {{ interface | json }}
    - user: root
    - group: root
    - mode: 644

configure_interface_bond_{{ interface.devicename }}_network:
  file.managed:
    - name: "/etc/systemd/network/10-openmediavault-{{ interface.devicename }}.network"
    - source:
      - salt://{{ tpldir }}/files/bond_network.j2
    - template: jinja
    - context:
        interface: {{ interface | json }}
    - user: root
    - group: root
    - mode: 644

configure_interface_bond_{{ interface.bondprimary }}_link:
  file.managed:
    - name: "/etc/systemd/network/10-openmediavault-{{ interface.bondprimary }}.link"
    - source:
      - salt://{{ tpldir }}/files/link.j2
    - template: jinja
    - context:
        interface: {{ interface | json }}
    - user: root
    - group: root
    - mode: 644

{% for slave in interface.slaves.split(',') %}

# Populate the <DEVICE>.network files of the slave devices.
# 1. Make sure the file exists (if this was not done already).
# 2. Set additional options.
configure_interface_bond_{{ slave }}_network:
  file.managed:
    - name: "/etc/systemd/network/10-openmediavault-{{ slave }}.network"
    - user: root
    - group: root
    - mode: 644
  ini.options_present:
    - name: "/etc/systemd/network/10-openmediavault-{{ slave }}.network"
    - separator: "="
    - sections:
        Match:
          Name: {{ slave }}
        Network:
          Bond: {{ interface.devicename }}
          PrimarySlave: {{ (slave == interface.bondprimary) | to_bool }}

{% endfor %}

{% endfor %}
