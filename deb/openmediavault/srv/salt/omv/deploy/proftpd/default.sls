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

{% set config = salt['omv_conf.get']('conf.service.ftp') %}

include:
  - .modules

{% if config.enable | to_bool %}

test_proftpd_service_config:
  cmd.run:
    - name: "proftpd --configtest"

start_proftpd_service:
  service.running:
    - name: proftpd
    - enable: True
    - require:
      - cmd: test_proftpd_service_config

monitor_proftpd_service:
  module.run:
    - name: monit.monitor
    - m_name: proftpd
    - require:
      - service: start_proftpd_service

{% else %}

start_proftpd_service:
  test.nop

unmonitor_proftpd_service:
  module.run:
    - name: monit.unmonitor
    - m_name: proftpd

stop_proftpd_service:
  service.dead:
    - name: proftpd
    - enable: False

{% endif %}
