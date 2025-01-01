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

{% set config = salt['omv_conf.get']('conf.system.powermngmnt') %}

configure_power_button:
  ini.options_present:
    - name: "/etc/systemd/logind.conf"
    - separator: "="
    - sections:
        Login:
          HandlePowerKey: {{ config.powerbtn | replace('nothing', 'ignore') | replace('shutdown', 'poweroff') | replace('standby', config.standbymode) | replace('suspendhybrid', 'hybrid-sleep') }}

divert_power_button:
  omv_dpkg.divert_add:
    - name: "/etc/systemd/logind.conf"

restart_systemd_logind:
  service.running:
    - name: systemd-logind
    - watch:
      - ini: configure_power_button
