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

{% set config = salt['omv_conf.get']('conf.system.notification.email') %}

{% if config.enable | to_bool %}

remove_postfix_cron:
  file.absent:
    - name: "/etc/cron.hourly/openmediavault-flushmailq"

{% else %}

configure_postfix_cron:
  file.managed:
    - name: "/etc/cron.hourly/openmediavault-flushmailq"
    - contents: |
        #!/bin/sh
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        # Flush the mail queue every hour if email notification is disabled.
        postsuper -d ALL
    - user: root
    - group: root
    - mode: 750
    - watch_in:
      - service: start_postfix_service

{% endif %}
