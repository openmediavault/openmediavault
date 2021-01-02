# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2021 Volker Theile
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

{% set tftp_config = salt['omv_conf.get']('conf.service.tftp') %}
{% set tftp_zeroconf_enabled = salt['pillar.get']('default:OMV_TFTPDHPA_ZEROCONF_ENABLED', 1) %}
{% set tftp_zeroconf_name = salt['pillar.get']('default:OMV_TFTPDHPA_ZEROCONF_NAME', '%h - TFTP') %}

{% if not (tftp_config.enable | to_bool and tftp_zeroconf_enabled | to_bool) %}

remove_avahi_service_tftpd-hpa:
  file.absent:
    - name: "/etc/avahi/services/tftp.service"

{% else %}

configure_avahi_service_tftpd-hpa:
  file.managed:
    - name: "/etc/avahi/services/tftp.service"
    - source:
      - salt://{{ tpldir }}/files/template.j2
    - template: jinja
    - context:
        type: "_tftp._udp"
        port: {{ tftp_config.port }}
        name: "{{ tftp_zeroconf_name }}"
    - user: root
    - group: root
    - mode: 644

{% endif %}
