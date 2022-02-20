# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2022 Volker Theile
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

# Make sure the directory '/run/proftpd' exists to prevent a config
# test failure. Note, the config test is executed BEFORE proftpd
# is started, so the directory isn't auto-created by the daemon at
# this time.
# mod_ban/0.8: unable to open BanTable '/run/proftpd/ban.tab': No such file or directory
create_run_proftpd_dir:
  file.directory:
    - name: "/run/proftpd"

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
    - monit.monitor:
      - name: proftpd
    - require:
      - service: start_proftpd_service

{% else %}

start_proftpd_service:
  test.nop

unmonitor_proftpd_service:
  cmd.run:
    - name: monit unmonitor proftpd || true

stop_proftpd_service:
  service.dead:
    - name: proftpd
    - enable: False

{% endif %}
