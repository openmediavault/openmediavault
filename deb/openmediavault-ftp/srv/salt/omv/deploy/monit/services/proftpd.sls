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

{% set proftpd_config = salt['omv_conf.get']('conf.service.ftp') %}

{% if proftpd_config.enable | to_bool %}

{% set email_config = salt['omv_conf.get']('conf.system.notification.email') %}
{% set notification_config = salt['omv_conf.get_by_filter'](
  'conf.system.notification.notification',
  {'operator': 'stringEquals', 'arg0': 'id', 'arg1': 'monitprocevents'})[0] %}

configure_monit_proftpd_service:
  file.managed:
    - name: "/etc/monit/conf.d/openmediavault-proftpd.conf"
    - source:
      - salt://{{ tpldir }}/files/proftpd.j2
    - template: jinja
    - context:
        proftpd_config: {{ proftpd_config | json }}
        email_config: {{ email_config | json }}
        notification_config: {{ notification_config | json }}
    - user: root
    - group: root
    - mode: 644

{% endif %}
