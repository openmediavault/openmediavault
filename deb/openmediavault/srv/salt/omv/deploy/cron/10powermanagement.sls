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
# http://www.cyberciti.biz/faq/disable-the-mail-alert-by-crontab-command
# http://www.cyberciti.biz/faq/how-do-i-add-jobs-to-cron-under-linux-or-unix-oses
# http://wiki.dreamhost.com/index.php/Crontab

{% set config = salt['omv_conf.get']('conf.system.powermngmnt') %}
{% set cron_jobs = salt['omv_conf.get_by_filter'](
  'conf.system.cron.job',
  {'operator': 'and', 'arg0': {'operator': 'stringEnum', 'arg0': 'type', 'arg1': ['reboot', 'shutdown', 'standby']}, 'arg1': {'operator': 'equals', 'arg0': 'enable', 'arg1': '1'}}) %}

create_cron_powermanagement:
  file.managed:
    - name: "/etc/cron.d/openmediavault-powermngmt"
    - source:
      - salt://{{ tpldir }}/files/powermanagement.j2
    - template: jinja
    - context:
        config: {{ config | json }}
        jobs: {{ cron_jobs | json }}
    - user: root
    - group: root
    - mode: 644
