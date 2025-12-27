# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.

# Deploy Monit configuration with alert program for email batching during
# power management operations. This is called explicitly by omv-powermgmt-wrapper
# when batching mode is activated via: omv-salt deploy run monit.alertprogram

{% set email_config = salt['omv_conf.get']('conf.system.notification.email') %}

configure_monit_alertprogram:
  file.managed:
    - name: "/etc/monit/monitrc"
    - source:
      - salt://{{ tpldir }}/files/etc-monit-monitrc-alertprogram.j2
    - template: jinja
    - context:
        email_config: {{ email_config | json }}
    - user: root
    - group: root
    - mode: 700

reload_monit_alertprogram:
  cmd.run:
    - name: "systemctl reload monit"
    - require:
      - file: configure_monit_alertprogram
