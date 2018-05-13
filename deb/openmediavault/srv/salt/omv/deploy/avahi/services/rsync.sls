# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2018 Volker Theile
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

{% set rsync_config = salt['omv.get_config']('conf.service.rsyncd') %}
{% set zeroconf_config = salt['omv.get_config_by_filter'](
  'conf.service.zeroconf.service',
  '{"operator": "stringEquals", "arg0": "id", "arg1": "rsync"}')[0] %}

{% if not (rsync_config.enable | to_bool and zeroconf_config.enable | to_bool) %}

remove_avahi_service_rsync:
  file.absent:
    - name: "/etc/avahi/services/rsync.service"

{% else %}

configure_avahi_service_rsync:
  file.managed:
    - name: "/etc/avahi/services/rsync.service"
    - source:
      - salt://{{ slspath }}/files/template.j2
    - template: jinja
    - context:
        type: "_rsync._tcp"
        port: {{ rsync_config.port }}
        name: "{{ zeroconf_config.name }}"
    - user: root
    - group: root
    - mode: 644

{% endif %}
