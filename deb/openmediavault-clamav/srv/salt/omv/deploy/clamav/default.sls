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
# http://wiki.dreamhost.com/index.php/Crontab/
# https://www.systutorials.com/docs/linux/man/5-clamd.conf/
# https://www.systutorials.com/docs/linux/man/5-freshclam.conf/
# https://blog.clamav.net/2019/09/understanding-and-transitioning-to.html

{% set cron_scripts_dir = salt['pillar.get']('default:OMV_CRONSCRIPTS_DIR', '/var/lib/openmediavault/cron.d') %}
{% set cron_script_prefix = salt['pillar.get']('default:OMV_CLAMAV_CLAMDSCAN_CRONSCRIPT_PREFIX', 'clamdscan-') %}
{% set clamav_clamd_logfile = salt['pillar.get']('default:OMV_CLAMAV_CLAMD_LOGFILE', '/var/log/clamav/clamav.log') %}
{% set clamav_clamd_user = salt['pillar.get']('default:OMV_CLAMAV_CLAMD_USER', 'clamav') %}
{% set clamav_freshclam_logfile = salt['pillar.get']('default:OMV_CLAMAV_FRESHCLAM_UPDATELOGFILE', '/var/log/clamav/freshclam.log') %}
{% set clamav_freshclam_user = salt['pillar.get']('default:OMV_CLAMAV_FRESHCLAM_DATABASEOWNER', 'clamav') %}
{% set clamav_config = salt['omv_conf.get']('conf.service.clamav') %}
{% set proxy_config = salt['omv_conf.get']('conf.system.network.proxy') %}

remove_clamav_clamdscan_cron:
  file.absent:
    - name: "/etc/cron.d/openmediavault-clamdscan"

remove_clamav_clamdscan_cron_scripts:
  module.run:
    - file.find:
      - path: "{{ cron_scripts_dir }}"
      - iname: "{{ cron_script_prefix }}*"
      - delete: "f"

remove_clamav_daemon_logrotate:
  file.absent:
    - name: "/etc/logrotate.d/clamav-daemon"

remove_clamav_freshclam_logrotate:
  file.absent:
    - name: "/etc/logrotate.d/clamav-freshclam"

# If Apparmor is installed and enabled, we need to make
# sure that the shared folders are accessible by clamd.
configure_clamd_apparmor_profile:
  file.replace:
    - name: "/etc/apparmor.d/usr.sbin.clamd"
    - pattern: "#(include <local/usr\\.sbin\\.clamd>)"
    - repl: "\\1"
    - ignore_if_missing: True
    - backup: False

# https://wiki.ubuntuusers.de/AppArmor/#Berechtigungen
configure_clamd_apparmor_local_profile:
  file.append:
    - name: "/etc/apparmor.d/local/usr.sbin.clamd"
    - text: |
        # Allow mount dirs to be scanned. Need write access to be able to
        # move/delete malicious files.
        /srv/** krw,
        /media/** krw,

        # Allow an action to perform when clamav detects a malicious file.
        # The scripts located in /etc/clamav/virusevent.d/ may require
        # complex privileges, because of that we use the unconfined
        # execute mode.
        /usr/bin/dash muxr,

        # Allow user clamav to access log files at /var/log/clamav
        capability chown,

# https://help.ubuntu.com/community/AppArmor#Reload_one_profile
reload_clamd_apparmor_profile:
  cmd.run:
    - name: "apparmor_parser -r /etc/apparmor.d/usr.sbin.clamd"
    - onlyif: "which apparmor_parser"
    - onchanges:
      - file: configure_clamd_apparmor_profile
      - file: configure_clamd_apparmor_local_profile

{% if clamav_config.enable | to_bool %}

{% if clamav_config.freshclam.enable | to_bool %}

configure_clamav_freshclam:
  file.managed:
    - name: "/etc/clamav/freshclam.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-clamav-freshclam_conf.j2
    - template: jinja
    - context:
        clamav_config: {{ clamav_config | json }}
        proxy_config: {{ proxy_config | json }}
    - user: root
    - group: root
    - mode: 644

# Ensure the signature databases exists, otherwise starting
# clamav-daemon will fail. If they do not exist, then stop
# clamav-freshclam to automatically download them when it is
# started again. Wait for 5 minutes (consider slow internet
# connections) before aborting.
stop_clamav_freshclam_service_to_force_db_download:
  service.dead:
    - name: clamav-freshclam
    - onlyif: "test ! -e /var/lib/clamav/main.c*d"

start_clamav_freshclam_service:
  service.running:
    - name: clamav-freshclam
    - enable: True
    - watch:
      - file: configure_clamav_freshclam

wait_for_clamav_freshclam_db_download:
  file.exists:
    - name: "/var/lib/clamav/main.cvd"
    - retry:
        attempts: 60
        interval: 5
    - onlyif: "test ! -e /var/lib/clamav/main.cld"

configure_clamav_freshclam_logrotate:
  file.managed:
    - name: "/etc/logrotate.d/clamav-freshclam"
    - contents: |
        {{ clamav_freshclam_logfile }} {
            rotate 12
            weekly
            compress
            delaycompress
            create 640 {{ clamav_freshclam_user }} adm
            postrotate
            systemctl -q is-active clamav-freshclam && systemctl kill --signal=SIGHUP clamav-freshclam || true
            endscript
        }
    - user: {{ clamav_freshclam_user }}
    - group: root
    - mode: 644

{% else %}

stop_clamav_freshclam_service:
  service.dead:
    - name: clamav-freshclam
    - enable: False

{% endif %}

configure_clamav_daemon:
  file.managed:
    - name: "/etc/clamav/clamd.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-clamav-clamd_conf.j2
    - template: jinja
    - context:
        config: {{ clamav_config | json }}
    - user: root
    - group: root
    - mode: 644

start_clamav_daemon_service:
  service.running:
    - name: clamav-daemon
    - enable: True
    - watch:
      - file: configure_clamav_daemon

{% if clamav_config.onaccesspaths.onaccesspath | selectattr('enable') | list | length > 0 %}

start_clamav_onaccess_service:
  service.running:
    - name: clamav-onaccess
    - enable: True

{% else %}

stop_clamav_onaccess_service:
  service.dead:
    - name: clamav-onaccess
    - enable: False

{% endif %}

configure_clamav_clamdscan_cron:
  file.managed:
    - name: "/etc/cron.d/openmediavault-clamdscan"
    - source:
      - salt://{{ tpldir }}/files/cron-clamdscan.j2
    - context:
        jobs: {{ clamav_config.jobs.job | json }}
    - template: jinja
    - user: root
    - group: root
    - mode: 644

{% for job in clamav_config.jobs.job %}

configure_clamav_clamdscan_cron_script_{{ job.uuid }}:
  file.managed:
    - name: "{{ cron_scripts_dir | path_join(cron_script_prefix ~ job.uuid) }}"
    - source:
      - salt://{{ tpldir }}/files/cron-clamdscan-script.j2
    - context:
        clamav_config: {{ clamav_config | json }}
        job: {{ job | json }}
    - template: jinja
    - user: root
    - group: root
    - mode: 750

{% endfor %}

configure_clamav_daemon_logrotate:
  file.managed:
    - name: "/etc/logrotate.d/clamav-daemon"
    - contents: |
        {{ clamav_clamd_logfile }} {
            rotate 12
            weekly
            compress
            delaycompress
            create 640 {{ clamav_clamd_user }} adm
            postrotate
            systemctl -q is-active clamav-daemon && systemctl kill --signal=SIGHUP clamav-daemon || true
            endscript
        }
    - user: {{ clamav_clamd_user }}
    - group: root
    - mode: 644

{% else %}

stop_clamav_daemon_service:
  service.dead:
    - name: clamav-daemon
    - enable: False

stop_clamav_freshclam_service:
  service.dead:
    - name: clamav-freshclam
    - enable: False

stop_clamav_onaccess_service:
  service.dead:
    - name: clamav-onaccess
    - enable: False

{% endif %}
