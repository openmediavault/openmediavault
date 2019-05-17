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
# https://wiki.archlinux.org/index.php/Systemd-networkd#Wireless_adapter
# https://www.linkedin.com/pulse/using-systemd-networkd-manage-your-network-ajibola-okubanjo
# https://remy.grunblatt.org/using-systemd-networkd-with-wpa_supplicant-to-manage-wireless-network-configuration.html

{% set interfaces = salt['omv_conf.get_by_filter'](
  'conf.system.network.interface',
  {'operator': 'stringEquals', 'arg0': 'type', 'arg1': 'wireless'}) %}

{% for interface in interfaces %}

configure_interface_wireless_{{ interface.devicename }}_network:
  file.managed:
    - name: "/etc/systemd/network/openmediavault-{{ interface.devicename }}.network"
    - source:
      - salt://{{ slspath }}/files/wireless_network.j2
    - template: jinja
    - context:
        interface: {{ interface | tojson }}
    - user: root
    - group: root
    - mode: 644

configure_wpa_supplicant_{{ interface.devicename }}:
  file.managed:
    - name: "/etc/wpa_supplicant/wpa_supplicant-{{ interface.devicename }}.conf"
    - contents: |
        ctrl_interface=/var/run/wpa_supplicant
        ctrl_interface_group=0
        update_config=1
        eapol_version=1
        ap_scan=1
        fast_reauth=1
        network={
          ssid="{{ interface.wpassid }}"
          psk="{{ interface.wpapsk }}"
        }
    - user: root
    - group: root
    - mode: 640

restart_wpa_supplicant_{{ interface.devicename }}:
  service.running:
    - name: wpa_supplicant@{{ interface.devicename }}
    - enable: True

{% endfor %}
