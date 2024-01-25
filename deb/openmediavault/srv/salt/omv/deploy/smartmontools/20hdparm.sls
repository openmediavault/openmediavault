# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2024 Volker Theile
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
# https://wiki.archlinux.org/title/hdparm
# https://lukas.zapletalovi.com/posts/2020/configuring-hdd-to-spin-down-in-linux-via-smart/

{% set config = salt['omv_conf.get']('conf.system.hdparm') %}

{% for device in config %}

{% set filename = salt['cmd.run']('systemd-escape --path ' ~ device.devicefile) %}
smartmontools_hdparm_{{ filename }}:
  file.managed:
    - name: "/etc/smartmontools/hdparm.d/openmediavault-{{ filename }}"
    - contents: |
        #!/bin/sh
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        smartctl{% if device.aam > 0 %} --set=aam,{{ device.aam }}{% endif %}{% if device.apm > 0 %} --set=apm,{{ device.apm }}{% endif %}{% if device.spindowntime > 0 %} --set=standby,{{ device.spindowntime }}{% endif %}{% if device.writecache | to_bool %} --set=wcache,on{% else %} --set=wcache,off{% endif %} {{ device.devicefile }}
    - user: root
    - group: root
    - mode: 744

{% endfor %}

restart_smartmontools_smartctl_hdparm_service:
  module.run:
    - service.restart:
      - name: smartctl-hdparm
