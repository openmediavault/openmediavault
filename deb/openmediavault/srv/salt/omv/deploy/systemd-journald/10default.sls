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

{% set system_max_use = salt['pillar.get']('default:OMV_SYSTEMD_JOURNALD_SYSTEMMAXUSE', '1G') %}

systemd_journald_create_conf_dir:
  file.directory:
    - name: "/etc/systemd/journald.conf.d/"
    - makedirs: True

systemd_journald_create_conf_file:
  file.managed:
    - name: "/etc/systemd/journald.conf.d/90-openmediavault.conf"
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        [Journal]
        SystemMaxUse={{ system_max_use }}

restart_systemd_journald_service:
  service.running:
    - name: systemd-journald
    - enable: True
    - watch:
      - file: systemd_journald_create_conf_file
