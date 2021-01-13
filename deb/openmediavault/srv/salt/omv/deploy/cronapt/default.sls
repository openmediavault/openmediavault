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

{% set notification_config = salt['omv_conf.get_by_filter'](
  'conf.system.notification.notification',
  {'operator': 'stringEquals', 'arg0': 'id', 'arg1': 'cronapt'},
  min_result=1, max_result=1)[0] %}
{% set email_config = salt['omv_conf.get']('conf.system.notification.email') %}
{% set refrain_file = salt['pillar.get']('default:OMV_CRONAPT_REFRAINFILE', '/etc/cron-apt/refrain') %}

# If this file exist cron-apt will silently exit, so make sure
# it does not exist.
remove_cron-apt_refrain_file:
  file.absent:
    - name: "{{ refrain_file }}"

create_cron-apt_config:
  file.managed:
    - name: "/etc/cron-apt/config"
    - source:
      - salt://{{ tpldir }}/files/etc_cron-apt_config.j2
    - template: jinja
    - context:
        email_config: {{ email_config | json }}
        notification_config: {{ notification_config | json }}
    - user: root
    - group: root
    - mode: 644

create_cron-apt_download_msg:
  file.managed:
    - name: "/etc/cron-apt/mailmsg.d/3-download"
    - source:
      - salt://{{ tpldir }}/files/etc_cron-apt_mailmsgd_3-download.j2
    - user: root
    - group: root
    - mode: 644
