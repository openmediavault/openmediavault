# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.

{% set config = salt['omv_conf.get']('conf.system.notification.email') %}

{% if config.enable | to_bool %}

remove_postfix_sendmail_wrapper:
  file.absent:
    - name: "/usr/sbin/sendmail"
    - onlyif: dpkg-divert --list /usr/sbin/sendmail | grep -q .

remove_postfix_divert_sendmail:
  cmd.run:
    - name: dpkg-divert --remove --rename /usr/sbin/sendmail

{% else %}

# If email notifications are disabled, we need another sendmail wrapper,
# otherwise the Postfix sendmail wrapper (/usr/sbin/sendmail) tries to
# connect to the queue manager through a local socket. When Postfix isn't
# running, the connection attempt times out, causing the caller to hang.

postfix_divert_sendmail:
  cmd.run:
    - name: dpkg-divert --add --rename --divert /usr/sbin/sendmail.real /usr/sbin/sendmail

create_postfix_sendmail_wrapper:
  file.managed:
    - name: "/usr/sbin/sendmail"
    - contents: |
        #!/bin/sh
        exit 0
    - user: root
    - group: root
    - mode: 755

{% endif %}
