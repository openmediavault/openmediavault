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
# https://www.freedesktop.org/software/systemd/man/systemd.network.html
# https://wiki.archlinux.org/index.php/Systemd-networkd
# https://manpages.debian.org/systemd/systemd.link.5.en.html

{% set interfaces = salt['omv_conf.get_by_filter'](
  'conf.system.network.interface',
  {'operator': 'stringEquals', 'arg0': 'type', 'arg1': 'ethernet'}) %}

{% for interface in interfaces %}

configure_interface_wired_{{ interface.devicename }}_network:
  file.managed:
    - name: "/etc/systemd/network/10-openmediavault-{{ interface.devicename }}.network"
    - source:
      - salt://{{ slspath }}/files/wired_network.j2
    - template: jinja
    - context:
        interface: {{ interface | json }}
    - user: root
    - group: root
    - mode: 644

configure_interface_wired_{{ interface.devicename }}_link:
  file.managed:
    - name: "/etc/systemd/network/10-openmediavault-{{ interface.devicename }}.link"
    - source:
      - salt://{{ slspath }}/files/link.j2
    - template: jinja
    - context:
        interface: {{ interface | json }}
    - user: root
    - group: root
    - mode: 644

{% endfor %}
