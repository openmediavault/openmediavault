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

{% set cron_jobs = salt['omv_conf.get_by_filter'](
  'conf.system.cron.job',
  {'operator': 'stringEquals', 'arg0': 'type', 'arg1': 'userdefined'}) %}
{% set scripts_dir = salt['pillar.get']('default:OMV_CRONSCRIPTS_DIR', '/var/lib/openmediavault/cron.d') %}
{% set script_prefix = salt['pillar.get']('default:OMV_CRONTAB_USERDEFINED_PREFIX', 'userdefined-') %}

cron_create_cron_scripts_dir:
  file.directory:
    - name: "{{ scripts_dir }}"
    - makedirs: True

remove_cron_userdefined_scripts:
  module.run:
    - file.find:
      - path: "{{ scripts_dir }}"
      - iname: "{{ script_prefix }}*"
      - delete: "f"

{% for cron_job in cron_jobs %}
create_cron_userdefined_{{ cron_job.uuid }}_script:
  file.managed:
    - name: "{{ scripts_dir | path_join(script_prefix ~ cron_job.uuid) }}"
    - contents: |
        #!/bin/sh -l
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        {{ cron_job.command }}
    - user: {{ cron_job.username }}
    - group: root
    - mode: 744
{% endfor %}

create_cron_userdefined:
  file.managed:
    - name: "/etc/cron.d/openmediavault-userdefined"
    - source:
      - salt://{{ tpldir }}/files/userdefined.j2
    - template: jinja
    - context:
        jobs: {{ cron_jobs | json }}
    - user: root
    - group: root
    - mode: 644
