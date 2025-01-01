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
# rrdtool info /var/lib/rrdcached/db/localhost/<xxx>/<xxx>.rrd

{% set config = salt['omv_conf.get']('conf.system.monitoring.perfstats') %}
{% set daemon = salt['pillar.get']('default:OMV_RRDCACHED_DAEMON', '/usr/bin/rrdcached') %}
{% set write_timeout = salt['pillar.get']('default:OMV_RRDCACHED_WRITETIMEOUT', '900') %}
{% set base_path = salt['pillar.get']('default:OMV_RRDCACHED_BASEDIR', '/var/lib/rrdcached/db/') %}
{% set journal_path = salt['pillar.get']('default:OMV_RRDCACHED_JOURNALDIR', '/var/lib/rrdcached/journal/') %}
{% set pidfile = salt['pillar.get']('default:OMV_RRDCACHED_PIDFILE', '/run/rrdcached.pid') %}
{% set sockfile = salt['pillar.get']('default:OMV_RRDCACHED_SOCKETFILE', '/run/rrdcached.sock') %}
{% set flush_interval = salt['pillar.get']('default:OMV_RRDCACHED_FLUSHINTERVAL', '3600') %}
{% set base_options = salt['pillar.get']('default:OMV_RRDCACHED_BASEOPTIONS', '-B -F -f ' ~ flush_interval) %}

include:
  - omv.deploy.monit

configure_default_rrdcached:
  file.managed:
    - name: "/etc/default/rrdcached"
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        # /etc/default file for RRD cache daemon

        # Full path to daemon
        DAEMON={{ daemon }}

        # Optional override flush interval, in seconds.
        WRITE_TIMEOUT={{ write_timeout }}

        # Where database files are placed.  If left unset, the default /tmp will
        # be used.  NB: The daemon will reject a directory that has symlinks as
        # components.  NB: You may want to have -B in BASE_OPTS.
        BASE_PATH={{ base_path }}

        # Where journal files are placed.  If left unset, journaling will
        # be disabled.
        JOURNAL_PATH={{ journal_path }}

        # FHS standard placement for process ID file.
        PIDFILE={{ pidfile }}

        # FHS standard placement for local control socket.
        SOCKFILE={{ sockfile }}

        # Any other options not specifically supported by the script (-P, -f,
        # -F, -B).
        BASE_OPTIONS="{{ base_options }}"
    - user: root
    - group: root
    - mode: 644

divert_default_rrdcached:
  omv_dpkg.divert_add:
    - name: "/etc/default/rrdcached"

{% if config.enable | to_bool %}

start_rrdcached_service:
  service.running:
    - name: rrdcached
    - enable: True
    - watch:
      - file: configure_default_rrdcached

monitor_rrdcached_service:
  module.run:
    - monit.monitor:
      - name: rrdcached
    - require:
      - service: start_rrdcached_service
      - service: reload_monit_service

{% else %}

unmonitor_rrdcached_service:
  cmd.run:
    - name: monit unmonitor rrdcached || true
    - require:
      - service: reload_monit_service

stop_rrdcached_service:
  service.dead:
    - name: rrdcached
    - enable: False

{% endif %}
