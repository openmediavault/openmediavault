# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2026 Volker Theile
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.

{% set webadmin_config = salt['omv_conf.get']('conf.webadmin') %}
{% set webadmin_zeroconf_enabled = salt['pillar.get']('default:OMV_WEBGUI_ZEROCONF_ENABLED', 1) %}
{% set webadmin_zeroconf_name = salt['pillar.get']('default:OMV_WEBGUI_ZEROCONF_NAME', '%h') %}

{% if (webadmin_zeroconf_enabled | to_bool) %}

configure_avahi_service_webadmin:
  file.managed:
    - name: "/etc/avahi/services/website.service"
    - source:
      - salt://{{ tpldir }}/files/template.j2
    - template: jinja
    - context:
        name: "{{ webadmin_zeroconf_name }}"
        services:
{% if (webadmin_config.enablessl | to_bool) %}
          - type: "_https._tcp"
            port: {{ webadmin_config.sslport }}
            txt_records:
              - "path=/index.html"
              - "ui=admin"
{% endif %}
{% if not ((webadmin_config.enablessl | to_bool) and (webadmin_config.forcesslonly | to_bool)) %}
          - type: "_http._tcp"
            port: {{ webadmin_config.port }}
            txt_records:
              - "path=/index.html"
              - "ui=admin"
{% endif %}
    - user: root
    - group: root
    - mode: 644

{% endif %}
