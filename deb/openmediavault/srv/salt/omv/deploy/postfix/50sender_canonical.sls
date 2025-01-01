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

# Rewrite all outgoing emails with the configured sender address, otherwise
# the SMTP relay will bounce them, e.g.
# postfix/smtp[xxxxx]: XXXXXXXXXX: to=<abc@xyz.localdomain>,
#   orig_to=<test>, relay=mail.gmx.net[x.x.x.x]:25, delay=1,
#   delays=0.02/0.02/0.93/0.06, dsn=5.7.0, status=bounced (host
#   mail.gmx.net[x.x.x.x] said: 550 5.7.0 Sender address does not belong to
#   logged in user {mp030} (in reply to MAIL FROM command))
configure_postfix_sender_canonical:
  file.managed:
    - name: "/etc/postfix/sender_canonical"
    - source:
      - salt://{{ tpldir }}/files/sender_canonical.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 600

run_postmap_sender_canonical:
  cmd.run:
    - name: "postmap /etc/postfix/sender_canonical"
    - onchanges:
      - file: configure_postfix_sender_canonical
