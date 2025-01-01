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

# Documentation/Howto:
# https://www.freedesktop.org/software/systemd/man/systemd.link.html

{% set set_mac_addr_policy = salt['omv_conf.get_by_filter'](
  'conf.system.network.interface',
  {'operator': 'stringNotEquals', 'arg0': 'altmacaddress', 'arg1': ''}) | length > 0 %}

unmask_systemd_networkd:
  service.unmasked:
    - name: systemd-networkd

enable_systemd_networkd:
  service.enabled:
    - name: systemd-networkd

{% if set_mac_addr_policy %}

create_systemd_networkd_defaults:
  file.managed:
    - name: "/etc/systemd/network/05-openmediavault-default.link"
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        [Match]
        OriginalName=*

        [Link]
        # This setting is necessary otherwise the alternative MAC addresses
        # are not applied.
        MACAddressPolicy=none
    - user: root
    - group: root
    - mode: 644

{% else %}

remove_systemd_networkd_defaults:
  file.absent:
    - name: "/etc/systemd/network/05-openmediavault-default.link"

{% endif %}
