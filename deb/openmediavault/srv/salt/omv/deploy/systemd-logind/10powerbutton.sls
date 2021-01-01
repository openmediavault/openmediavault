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

{% set config = salt['omv_conf.get']('conf.system.powermngmnt') %}
{% set action = config.powerbtn | replace('nothing', 'ignore') | replace('shutdown', 'poweroff') %}
{% if action == 'standby' %}
    # Put the machine in a sleep state. If suspend to disk (STD) or
    # RAM (STR) is not supported the system will be shut down. The
    # system will be put into sleep in the following order depending
    # on which state is supported:
    # * Suspend to disk (STD)
    # * Suspend to RAM (STR)
    # * Shut down and turn of system
    # See https://cgit.freedesktop.org/pm-utils/tree/pm/pm-functions.in#n294
    {% if 'disk' in salt['sysfs.read']('power/state') and
            salt['sysfs.read']('power/disk') not in ['[disabled]', False] and
            salt['mount.swaps']() | length > 1 %}
        {% set action = 'hibernate' %}
    {% elif 'mem' in salt['sysfs.read']('power/state') %}
        {% set action = 'suspend' %}
    {% else %}
        {% set action = 'poweroff' %}
    {% endif %}
{% endif %}

configure_power_button:
  ini.options_present:
    - name: "/etc/systemd/logind.conf"
    - separator: "="
    - sections:
        Login:
          HandlePowerKey: {{ action }}

restart_systemd_logind:
  service.running:
    - name: systemd-logind
    - watch:
      - ini: configure_power_button
