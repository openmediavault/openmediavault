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
# http://mmonit.com/monit/documentation/monit.html
# http://mmonit.com/wiki/Monit/ConfigurationExamples
# http://www.cyberciti.biz/tips/howto-monitor-and-restart-linux-unix-service.html
# https://www.adminlife.net/allgemein/howto-monit-unter-debian-etch
# http://www.howtoforge.com/server_monitoring_monit_munin
# http://www.howtoforge.de/howto/server-uberwachung-mit-munin-und-monit
# http://www.howtoforge.de/howto/wie-man-sich-mit-monit-per-sms-warnen-lasst-bei-einem-serverabsturz
# http://www.musicinfo.org/node/81
# http://www.cyberciti.biz/faq/tag/etcinitdmonit
# http://www.tim-bormann.de/linux-dienste-berwachen-mit-monit
# http://en.gentoo-wiki.com/wiki/Monit
# http://www.debianadmin.com/monitoring-debian-servers-using-monit.html
# http://www.uibk.ac.at/zid/systeme/linux/monit.html
# http://wiki.ubuntuusers.de/Monit
# http://viktorpetersson.com/2010/07/09/setting-up-monit-to-monitor-apache-and-postgresql-on-ubuntu

{% set email_config = salt['omv_conf.get']('conf.system.notification.email') %}

configure_default_monit:
  file.managed:
    - name: "/etc/default/monit"
    - contents:
      - "{{ pillar['headers']['auto_generated'] }}"
      - "{{ pillar['headers']['warning'] }}"
      - ""
      - "# You must set this variable to yes for monit to start"
      - "START=yes"
    - user: root
    - group: root
    - mode: 644

divert_default_monit:
  omv_dpkg.divert_add:
    - name: "/etc/default/monit"

configure_monit_monitrc:
  file.managed:
    - name: "/etc/monit/monitrc"
    - source:
      - salt://{{ tpldir }}/files/etc-monit-monitrc.j2
    - template: jinja
    - context:
        email_config: {{ email_config | json }}
    - user: root
    - group: root
    - mode: 700

divert_monit_monitrc:
  omv_dpkg.divert_add:
    - name: "/etc/monit/monitrc"
