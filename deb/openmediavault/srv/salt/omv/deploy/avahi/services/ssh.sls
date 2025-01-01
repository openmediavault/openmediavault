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

{% set ssh_config = salt['omv_conf.get']('conf.service.ssh') %}
{% set ssh_zeroconf_enabled = salt['pillar.get']('default:OMV_SSHD_ZEROCONF_ENABLED', 1) %}
{% set ssh_zeroconf_name = salt['pillar.get']('default:OMV_SSHD_ZEROCONF_NAME', '%h - SSH') %}

{% if (ssh_config.enable | to_bool) and (ssh_zeroconf_enabled | to_bool) %}

configure_avahi_service_ssh:
  file.managed:
    - name: "/etc/avahi/services/ssh.service"
    - source:
      - salt://{{ tpldir }}/files/ssh.j2
    - template: jinja
    - context:
        port: {{ ssh_config.port }}
        name: "{{ ssh_zeroconf_name }}"
    - user: root
    - group: root
    - mode: 644

{% endif %}
