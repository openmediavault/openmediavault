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

# Testing:
# systemctl --all --full -t device

{% set usbbackup_scripts_dir = salt['pillar.get']('default:OMV_USBBACKUP_SCRIPTS_DIR', '/var/lib/openmediavault/usbbackup.d') %}
{% set usbbackup_systemd_unitfile_dir = salt['pillar.get']('default:OMV_USBBACKUP_SYSTEMD_UNITFILE_DIR', '/lib/systemd/system') %}
{% set usbbackup_rsync_script_prefix = salt['pillar.get']('default:OMV_USBBACKUP_RSYNC_SCRIPT_PREFIX', 'rsync-') %}
{% set usbbackup_systemd_script_prefix = salt['pillar.get']('default:OMV_USBBACKUP_SYSTEMD_SCRIPT_PREFIX', 'systemd-') %}
{% set usbbackup_systemd_unitfile_prefix = salt['pillar.get']('default:OMV_USBBACKUP_SYSTEMD_UNITFILE_PREFIX', 'openmediavault-usbbackup-') %}

create_usbbackup_scripts_dir:
  file.directory:
    - name: {{ usbbackup_scripts_dir }}
    - clean: True

remove_usbbackup_rsync_scripts:
  module.run:
    - file.find:
      - path: {{ usbbackup_scripts_dir }}
      - iname: "{{ usbbackup_rsync_script_prefix }}*"
      - delete: "f"

remove_usbbackup_systemd_scripts:
  module.run:
    - file.find:
      - path: {{ usbbackup_scripts_dir }}
      - iname: "{{ usbbackup_systemd_script_prefix }}*"
      - delete: "f"

{% for file in salt['file.readdir'](usbbackup_systemd_unitfile_dir) | sort %}
{% if file | regex_match('^' ~ usbbackup_systemd_unitfile_prefix ~ '(.+).service$', ignorecase=True) %}
disable_usbbackup_systemd_unitfile_{{ file }}:
  service.disabled:
    - name: {{ file }}

remove_usbbackup_systemd_unitfile_{{ file }}:
  file.absent:
    - name: {{ usbbackup_systemd_unitfile_dir | path_join(file) }}
{% endif %}
{% endfor %}

{% set jobs = salt['omv_conf.get']('conf.service.usbbackup.job') %}
{% for job in jobs %}
# Create the rsync scripts regardless if enabled or disabled. This is
# necessary to be able to run the job manually from within the UI.
configure_usbbackup_rsync_script_{{ job.uuid }}:
  file.managed:
    - name: {{ usbbackup_scripts_dir | path_join(usbbackup_rsync_script_prefix ~ job.uuid) }}
    - source:
      - salt://{{ tpldir }}/files/rsync-script.j2
    - context:
        job: {{ job | json }}
    - template: jinja
    - user: root
    - group: root
    - mode: 750
{% endfor %}

# Create the scripts that are executed by systemd when the USB storage device
# is connected. Process duplicate jobs per filesystem only once.
{% set jobs = salt['omv_conf.get_by_filter'](
  'conf.service.usbbackup.job',
  {'operator': 'and', 'arg0': {'operator': 'distinct', 'arg0': 'devicefile'}, 'arg1': {'operator': 'equals', 'arg0': 'enable', 'arg1': '1'}}) %}
{% for job in jobs %}
configure_usbbackup_systemd_script_{{ job.devicefile | md5 }}:
  file.managed:
    - name: {{ usbbackup_scripts_dir | path_join(usbbackup_systemd_script_prefix ~ job.devicefile | md5) }}
    - source:
      - salt://{{ tpldir }}/files/systemd-unitfile-script.j2
    - context:
        devicefile: {{ job.devicefile }}
    - template: jinja
    - user: root
    - group: root
    - mode: 750

configure_usbbackup_systemd_unitfile_{{ job.devicefile | md5 }}:
  file.managed:
    - name: {{ usbbackup_systemd_unitfile_dir | path_join(usbbackup_systemd_unitfile_prefix ~ job.devicefile | md5 ~ '.service') }}
    - source:
      - salt://{{ tpldir }}/files/systemd-unitfile.j2
    - context:
        devicefile: {{ job.devicefile }}
    - template: jinja
    - user: root
    - group: root
    - mode: 644
{% endfor %}

usbbackup_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% for job in jobs %}
enable_usbbackup_systemd_unitfile_{{ job.devicefile | md5 }}:
  service.enabled:
    - name: {{ usbbackup_systemd_unitfile_prefix ~ job.devicefile | md5 }}
    - enable: True
{% endfor %}
