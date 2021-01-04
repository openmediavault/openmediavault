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

{% set config = salt['omv_conf.get']('conf.service.nut') %}
{% set port = salt['pillar.get']('default:OMV_NUT_UPSD_PORT', '3493') %}

{% if config.enable | to_bool %}

configure_collectd_conf_nut_plugin:
  file.managed:
    - name: "/etc/collectd/collectd.conf.d/nut.conf"
    - contents: |
        LoadPlugin nut
        <Plugin nut>
            {% if config.mode == 'netclient' -%}
            UPS "{{ config.upsname }}@{{ config.netclienthostname }}"
            {%- else -%}
            UPS "{{ config.upsname }}@localhost:{{ port }}"
            {%- endif %}
        </Plugin>

{% else %}

remove_collectd_conf_nut_plugin:
  file.absent:
    - name: "/etc/collectd/collectd.conf.d/nut.conf"

{% endif %}
