# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2018 Volker Theile
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
# https://wiki.archlinux.org/index.php/Shutting_system_down_by_pressing_the_power_button

{% set config = salt['omv.get_config']('conf.system.powermngmnt') %}

configure_default_acpid:
  file.managed:
    - name: "/etc/default/acpid"
    - source:
      - salt://{{ slspath }}/files/etc-default-acpid.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

configure_acpid_events:
  file.managed:
    - name: "/etc/acpi/events/powerbtn"
    - source:
      - salt://{{ slspath }}/files/etc-acpi-events-powerbtn.j2
    - template: jinja
    - context:
        powerbtn: {{ config.powerbtn }}
    - user: root
    - group: root
    - mode: 644

restart_acpid_service:
  service.running:
    - name: acpid
    - watch:
      - file: configure_acpid_events
