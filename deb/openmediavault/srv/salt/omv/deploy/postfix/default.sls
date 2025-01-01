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

# Documentation/Howto:
# http://www.postfix.org/postconf.5.html
# http://www.postfix.org/pipe.8.html
# http://www.gtkdb.de/index_7_727.html
# http://irbs.net/internet/postfix/0503/2148.html
# http://gate.io/blogpost34
# http://www.cyberciti.biz/tips/howto-postfix-flush-mail-queue.html
# http://www.tuxfutter.de/wiki/Einrichten_eines_Mailservers_mit_Postfix
# http://dokuwiki.tachtler.net/doku.php?id=tachtler:postfix_centos_6
# http://serverfault.com/questions/536648/postfix-pass-a-copy-of-an-email-to-a-script-but-deliver-the-original-one-to-mai

# Testing:
# echo "Test" | mail -s "Test subject" -a "From: xxx@yyy.zzz" root
# hostname | mailx -s "root `hostname` `date`" root
# mailq
# postcat -q A705238B4C

# Working with CA certificates:
# - Add new CA certificate to /etc/ssl/certs
# - Modify /etc/ca-certificates.conf
# - Execute update-ca-certificates

{% set config = salt['omv_conf.get']('conf.system.notification.email') %}
{% set dirpath = '/srv/salt' | path_join(tpldir) %}

include:
{% for file in salt['file.readdir'](dirpath) | sort %}
{% if file | regex_match('^(\d+.+).sls$', ignorecase=True) %}
  - .{{ file | replace('.sls', '') }}
{% endif %}
{% endfor %}

postfix_set_permissions:
  cmd.run:
    - name: postfix set-permissions

{% if config.enable | to_bool %}

start_postfix_service:
  service.running:
    - name: postfix
    - enable: True
    - watch:
      - file: configure_postfix_main
      - file: configure_postfix_master
      - file: configure_postfix_recipient_bcc
      - file: configure_postfix_recipient_canonical
      - file: configure_postfix_sasl_passwd
      - file: configure_postfix_sender_bcc
      - file: configure_postfix_sender_canonical
      - file: configure_postfix_smtp_header_checks
      - file: configure_postfix_transport

{% else %}

stop_postfix_service:
  service.dead:
    - name: postfix
    - enable: False

{% endif %}
