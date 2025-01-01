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

{% set dirpath = '/srv/salt' | path_join(tpldir) %}

include:
  - omv.deploy.monit
{% for file in salt['file.find'](dirpath, iname='*.sls', print='name') | difference(['init.sls', 'default.sls']) | sort %}
  - .{{ file | replace('.sls', '') }}
{% endfor %}

test_phpfpm_service_config:
  cmd.run:
    - name: "php-fpm8.2 --test"

restart_phpfpm_service:
  service.running:
    - name: php8.2-fpm
    - enable: True
    - require:
      - cmd: test_phpfpm_service_config
    - watch:
      - file: configure_phpfpm_webgui

monitor_phpfpm_service:
  module.run:
    - monit.monitor:
      - name: php-fpm
    - require:
      - service: restart_phpfpm_service
      - service: reload_monit_service
