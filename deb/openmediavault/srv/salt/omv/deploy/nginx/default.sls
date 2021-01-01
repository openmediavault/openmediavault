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

{% set dirpath = '/srv/salt' | path_join(tpldir) %}

prereq_nginx_service_monit:
  salt.state:
    - tgt: '*'
    - sls: omv.deploy.monit

include:
{% for file in salt['file.find'](dirpath, iname='*.sls', print='name') | difference(['init.sls', 'default.sls']) %}
  - .{{ file | replace('.sls', '') }}
{% endfor %}

test_nginx_service_config:
  cmd.run:
    - name: "nginx -t"

restart_nginx_service:
  service.running:
    - name: nginx
    - enable: True
    - require:
      - cmd: test_nginx_service_config

monitor_nginx_service:
  module.run:
    - name: monit.monitor
    - m_name: nginx
    - require:
      - service: restart_nginx_service
