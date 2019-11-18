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

# Flush all network interfaces that are not configured anymore. To identify
# devices that are not used/configured anymore, simply iterate through the
# existing configuration files and extract the device names. Then check if
# those devices exists in the database.
# Note, systemd-networkd does not like that devices are flushed before they
# are configured by the daemon. It will time out in such a case (bug???).

# Get the network devices that are used after the configuration has
# been applied.
{% set interfaces_config = salt['omv_conf.get']('conf.system.network.interface') %}
{% set used_devices_in_db = interfaces_config | map(attribute='devicename') | list %}
{% for interface in interfaces_config %}
{% for slave in interface.slaves.split(',') %}
{% if slave %}
{% set _ = used_devices_in_db.append(slave) %}
{% endif %}
{% endfor %}
{% endfor %}

# Get the network devices that are used at the moment BEFORE the new
# configuration is applied.
{% set used_devices = [] %}
{% for file in salt['file.find']('/etc/systemd/network/', iname='^(*-)?openmediavault-*.network$', print='name') | sort %}
{% set devicename = file | regex_search('^\d+-openmediavault-(.+)\.network$') | first %}
{% set _ = used_devices.append(devicename) %}
{% endfor %}

{% for devicename in used_devices | difference(used_devices_in_db | unique) %}

flush_interface_{{ devicename }}:
  cmd.run:
    - name: "ip addr flush dev {{ devicename }}"
    - onlyif: "test -e /sys/class/net/{{ devicename }}"

{% endfor %}

remove_systemd_networkd_config_files:
  module.run:
    - file.find:
      - path: "/etc/systemd/network/"
      - iname: "^(*-)?openmediavault-*"
      - delete: "f"
