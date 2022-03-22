# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2022 Volker Theile
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

{% set config = salt['omv_conf.get']('conf.service.filebrowser') %}
{% set zeroconf_enabled = salt['pillar.get']('default:OMV_FILEBROWSER_ZEROCONF_ENABLED', 1) %}
{% set zeroconf_name = salt['pillar.get']('default:OMV_FILEBROWSER_ZEROCONF_NAME', '%h - File Browser') %}

{% if not (config.enable | to_bool and zeroconf_enabled | to_bool) %}

remove_avahi_service_filebrowser:
  file.absent:
    - name: "/etc/avahi/services/filebrowser.service"

{% else %}

configure_avahi_service_filebrowser:
  file.managed:
    - name: "/etc/avahi/services/filebrowser.service"
    - source:
      - salt://{{ tpldir }}/files/template.j2
    - template: jinja
    - context:
        type: "_http._tcp"
        port: {{ config.port }}
        name: "{{ zeroconf_name }}"
    - user: root
    - group: root
    - mode: 644

{% endif %}
