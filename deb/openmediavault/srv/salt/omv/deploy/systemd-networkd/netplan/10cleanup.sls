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

remove_netplan_systemd_networkd_config_files:
  module.run:
    - file.find:
      - path: "/etc/systemd/network/"
      - iname: "^(*-)?openmediavault-*"
      - delete: "f"

# Remove empty configuration files, otherwise the command
# "udevadm test-builtin net_setup_link /sys/class/net/lo"
# which is executed by netplan.io will fail.
remove_netplan_empty_systemd_networkd_config_files:
  module.run:
    - file.find:
      - path: "/etc/systemd/network/"
      - size: "0"
      - delete: "f"

remove_netplan_config_files:
  module.run:
    - file.find:
      - path: "/etc/netplan/"
      - iname: "^(*-)?openmediavault-*.yaml"
      - delete: "f"

# https://docs.armbian.com/User-Guide_Networking/
remove_netplan_default_armbian_config_file:
  file.absent:
    - name: "/etc/netplan/10-dhcp-all-interfaces.yaml"
