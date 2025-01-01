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

{% set dns_config = salt['omv_conf.get']('conf.system.network.dns') %}
{% set email_config = salt['omv_conf.get']('conf.system.notification.email') %}
{% set users = salt['omv_conf.get_by_filter'](
  'conf.system.usermngmnt.user',
  {'operator': 'stringNotEquals', 'arg0': 'email', 'arg1': ''}) %}

# Add a catch-all recipient, thus all emails send to an user/address not
# existing will be redirected to the configured primary recipient address.
configure_postfix_recipient_canonical:
  file.managed:
    - name: "/etc/postfix/recipient_canonical"
    - source:
      - salt://{{ tpldir }}/files/recipient_canonical.j2
    - template: jinja
    - context:
        dns_config: {{ dns_config | json }}
        email_config: {{ email_config | json }}
        users: {{ users | json }}
    - user: root
    - group: root
    - mode: 600

run_postmap_recipient_canonical:
  cmd.run:
    - name: "postmap /etc/postfix/recipient_canonical"
    - onchanges:
      - file: configure_postfix_recipient_canonical
