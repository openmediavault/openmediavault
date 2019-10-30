# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2019 Volker Theile
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

{% for file in salt['file.find']('/etc/systemd/network/', iname='^(*-)?openmediavault-*.network$', print='name') | sort %}

{% set ifname = file | regex_search('^\d+-openmediavault-(.+)\.network$') | first %}

flush_interface_{{ ifname }}:
  cmd.run:
    - name: "ip addr flush dev {{ ifname }}"
    - onlyif: "test -e /sys/class/net/{{ ifname }}"

{% endfor %}

remove_systemd_networkd_config_files:
  module.run:
    - file.find:
      - path: "/etc/systemd/network/"
      - iname: "^(*-)?openmediavault-*"
      - delete: "f"
