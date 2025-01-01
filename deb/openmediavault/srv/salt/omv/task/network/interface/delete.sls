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

# Testing:
# systemctl status sharedfolders-test.mount
# salt-call --local --retcode-passthrough --no-color state.apply omv.task.apply

{% set data = salt['pillar.get']('data', None) %}

{% if data is not none %}

ip_link_set_down_{{ data.name }}:
  cmd.run:
    - name: "ip link set down {{ data.name }}"
    - onlyif: "test -e /sys/class/net/{{ data.name }}"

ip_addr_flush_dev_{{ data.name }}:
  cmd.run:
    - name: "ip addr flush dev {{ data.name }}"
    - onlyif: "test -e /sys/class/net/{{ data.name }}"

{% if data.type not in ('ethernet', 'wifi') %}

ip_link_del_dev_{{ data.name }}:
  cmd.run:
    - name: "ip link del dev {{ data.name }}"
    - onlyif: "test -e /sys/class/net/{{ data.name }}"

{% endif %}

{% if data.type == 'wifi' %}

stop_wpa_supplicant_{{ data.name }}:
  service.dead:
    - name: wpa_supplicant@{{ data.name }}
    - enable: False

{% endif %}

{% endif %}
