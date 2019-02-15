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

{% set smb_config = salt['omv_conf.get']('conf.service.smb') %}
{% set zeroconf_config = salt['omv_conf.get_by_filter'](
  'conf.service.zeroconf.service',
  {'operator': 'stringEquals', 'arg0': 'id', 'arg1': 'smb'})[0] %}

{% if not (smb_config.enable | to_bool and zeroconf_config.enable | to_bool) %}

remove_avahi_service_smb:
  file.absent:
    - name: "/etc/avahi/services/smb.service"

{% else %}

configure_avahi_service_smb:
  file.managed:
    - name: "/etc/avahi/services/smb.service"
    - source:
      - salt://{{ slspath }}/files/smb.j2
    - template: jinja
    - context:
        type: "_smb._tcp"
        port: 445
        name: "{{ zeroconf_config.name }}"
        shares: {{ smb_config.shares.share | json }}
    - user: root
    - group: root
    - mode: 644

{% endif %}
