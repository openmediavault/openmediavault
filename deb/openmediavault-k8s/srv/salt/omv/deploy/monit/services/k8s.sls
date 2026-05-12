# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2026 Volker Theile
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.

{% set k8s_config = salt['omv_conf.get']('conf.service.k8s') %}
{% set notification_config = salt['omv_conf.get_by_filter'](
  'conf.system.notification.notification',
  {'operator': 'stringEquals', 'arg0': 'id', 'arg1': 'monitk8s'})[0] %}

{% if k8s_config.enable | to_bool and notification_config.enable | to_bool %}

configure_monit_k8s_service:
  file.managed:
    - name: "/etc/monit/conf.d/openmediavault-k8s.conf"
    - contents: |
        check program k8s-health-check with path "/usr/sbin/omv-k8s-health-check"
            if status != 0 for 2 cycles then alert
    - user: root
    - group: root
    - mode: 644

{% endif %}
