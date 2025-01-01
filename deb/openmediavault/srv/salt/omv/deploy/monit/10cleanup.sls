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

cleanup_monit_conf_dir:
  module.run:
    - file.find:
      - path: "/etc/monit/conf.d/"
      - iname: "openmediavault-*.conf"
      - delete: "f"

remove_monit_delay_file:
  file.absent:
    - name: "/etc/monit/monit_delay"

divert_monit_delay_file:
  omv_dpkg.divert_add:
    - name: "/etc/monit/monit_delay"
