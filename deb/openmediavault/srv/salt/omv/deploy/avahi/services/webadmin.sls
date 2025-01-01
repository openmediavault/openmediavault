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

{% set webadmin_config = salt['omv_conf.get']('conf.webadmin') %}
{% set webadmin_zeroconf_enabled = salt['pillar.get']('default:OMV_WEBGUI_ZEROCONF_ENABLED', 1) %}
{% set webadmin_zeroconf_name = salt['pillar.get']('default:OMV_WEBGUI_ZEROCONF_NAME', '%h - Web control panel') %}

{% if (webadmin_zeroconf_enabled | to_bool) %}

configure_avahi_service_webadmin:
  file.managed:
    - name: "/etc/avahi/services/website.service"
    - source:
      - salt://{{ tpldir }}/files/webadmin.j2
    - template: jinja
    - context:
        name: "{{ webadmin_zeroconf_name }}"
        enablessl: {{ webadmin_config.enablessl }}
        forcesslonly: {{ webadmin_config.forcesslonly }}
        port: {{ webadmin_config.port }}
        sslport: {{ webadmin_config.sslport }}
    - user: root
    - group: root
    - mode: 644

{% endif %}
