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
# http://wiki.dreamhost.com/index.php/Crontab

{% set scripts_dir = salt['pillar.get']('default:OMV_CRONSCRIPTS_DIR', '/var/lib/openmediavault/cron.d') %}
{% set script_prefix = salt['pillar.get']('default:OMV_RSYNC_CRONSCRIPT_PREFIX', 'rsync-') %}
{% set jobs = salt['omv_conf.get']('conf.service.rsync.job') %}

configure_rsync_cron:
  file.managed:
    - name: "/etc/cron.d/openmediavault-rsync"
    - source:
      - salt://{{ tpldir }}/files/cron-rsync.j2
    - context:
        jobs: {{ jobs | json }}
    - template: jinja
    - user: root
    - group: root
    - mode: 644

rsync_create_cron_scripts_dir:
  file.directory:
    - name: "{{ scripts_dir }}"
    - makedirs: True

remove_rsync_cron_scripts:
  module.run:
    - file.find:
      - path: "{{ scripts_dir }}"
      - iname: "{{ script_prefix }}*"
      - delete: "f"

{% for job in jobs %}

configure_rsync_cron_script_{{ job.uuid }}:
  file.managed:
    - name: "{{ scripts_dir | path_join(script_prefix ~ job.uuid) }}"
    - source:
      - salt://{{ tpldir }}/files/cron-rsync-script.j2
    - context:
        job: {{ job | json }}
    - template: jinja
    - user: root
    - group: root
    - mode: 750

{% endfor %}
