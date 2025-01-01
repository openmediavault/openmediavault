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

{% set notification_config = salt['omv_conf.get_by_filter'](
  'conf.system.notification.notification',
  {'operator': 'stringEquals', 'arg0': 'id', 'arg1': 'apt'})[0] %}

remove_apticron_default_daily_cron:
  file.absent:
    - name: "/etc/cron.d/apticron"

divert_apticron_default_daily_cron:
  omv_dpkg.divert_add:
    - name: "/etc/cron.d/apticron"

{% if notification_config.enable %}

create_apticron_cron_daily:
  file.managed:
    - name: "/etc/cron.daily/openmediavault-apticron"
    - contents: |
        #!/usr/bin/env dash
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        if test -x /usr/sbin/apticron; then /usr/sbin/apticron --cron; fi
    - user: root
    - group: root
    - mode: 755

{% else %}

remove_apticron_cron_daily:
  file.absent:
    - name: "/etc/cron.daily/openmediavault-apticron"

{% endif %}
