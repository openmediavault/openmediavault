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

{% set mkrrdgraph = salt['pillar.get']('default:OMV_MKRRDGRAPH', '/usr/sbin/omv-mkrrdgraph') %}
{% set mkrrdgraph_interval = salt['pillar.get']('default:OMV_MKRRDGRAPH_INTERVAL', '15') %}
{% set config = salt['omv_conf.get']('conf.system.monitoring.perfstats') %}

prereq_collectd_service_monit:
  salt.state:
    - tgt: '*'
    - sls: omv.deploy.monit

configure_collectd_conf:
  file.managed:
    - name: "/etc/collectd/collectd.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-collectd_collectd.conf.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

include:
  - .plugins

{% if config.enable | to_bool %}

start_collectd_service:
  service.running:
    - name: collectd
    - enable: True
    - watch:
      - file: "/etc/collectd/collectd.conf"
      - file: "/etc/collectd/collectd.conf.d/*.conf"

monitor_collectd_service:
  module.run:
    - name: monit.monitor
    - m_name: collectd
    - require:
      - service: start_collectd_service

install_mkrrdgraph_cron_job:
  file.managed:
    - name: "/etc/cron.d/openmediavault-mkrrdgraph"
    - contents:
      - "# Create graphs every {{ mkrrdgraph_interval }} minutes"
      - "# m h dom mon dow user    command"
      - "*/{{ mkrrdgraph_interval }} * * * * root {{ mkrrdgraph }} >/dev/null 2>&1"
    - user: root
    - group: root
    - mode: 644

generate_rrd_graphs:
  cmd.run:
    - name: "{{ mkrrdgraph }}"

{% else %}

remove_mkrrdgraph_cron_job:
  file.absent:
    - name: "/etc/cron.d/openmediavault-mkrrdgraph"

unmonitor_collectd_service:
  module.run:
    - name: monit.unmonitor
    - m_name: collectd

stop_collectd_service:
  service.dead:
    - name: collectd
    - enable: False

{% endif %}
