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

configure_tmp_mount_unit_file:
  file.managed:
    - name: '/etc/systemd/system/tmp.mount'
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        [Unit]
        Description=Mount /tmp as tmpfs to reduce write access to the root device
        Before=local-fs.target

        [Mount]
        What=tmpfs
        Where=/tmp
        Type=tmpfs
    - user: root
    - group: root
    - mode: 644

tmp_mount_unit_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:
    - onchanges:
      - file: configure_tmp_mount_unit_file
