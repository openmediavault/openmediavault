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

{% set config = salt['omv_conf.get']('conf.system.notification.email') %}

# BCC (blind carbon-copy) all emails send to the primary email address to
# the given secondary email address. Do not use 'sender_bcc_maps' for that,
# otherwise all emails, including local users, will be sent, too.
configure_postfix_recipient_bcc:
  file.managed:
    - name: "/etc/postfix/recipient_bcc"
    - source:
      - salt://{{ tpldir }}/files/recipient_bcc.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 600

run_postmap_recipient_bcc:
  cmd.run:
    - name: "postmap /etc/postfix/recipient_bcc"
    - onchanges:
      - file: configure_postfix_recipient_bcc
