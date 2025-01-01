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

{% set snmpd_config = salt['omv_conf.get']('conf.service.snmp') %}
{% set network_dns_config = salt['omv_conf.get']('conf.system.network.dns') %}

{% if snmpd_config.enable | to_bool %}

configure_snmpd:
  file.managed:
    - name: "/etc/snmp/snmpd.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-snmp-snmpd_conf.j2
    - template: jinja
    - context:
        snmpd_config: {{ snmpd_config | json }}
        network_dns_config: {{ network_dns_config | json }}
    - user: root
    - group: root
    - mode: 600

divert_snmpd:
  omv_dpkg.divert_add:
    - name: "/etc/snmp/snmpd.conf"

start_snmpd_service:
  service.running:
    - name: snmpd
    - enable: True
    - watch:
      - file: configure_snmpd

{% else %}

stop_snmpd_service:
  service.dead:
    - name: snmpd
    - enable: False

{% endif %}
