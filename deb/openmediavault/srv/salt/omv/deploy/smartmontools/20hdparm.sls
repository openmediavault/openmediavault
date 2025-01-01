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

# Documentation/Howto:
# https://www.smartmontools.org/browser/trunk/smartmontools/smartctl.8.in
# https://wiki.archlinux.org/title/hdparm
# https://lukas.zapletalovi.com/posts/2020/configuring-hdd-to-spin-down-in-linux-via-smart/

{% set smartmontools_config = salt['omv_conf.get']('conf.service.smartmontools') %}
{% set hdparm_config = salt['omv_conf.get']('conf.system.hdparm') %}

cleanup_smartmontools_hdparm_dir:
  module.run:
    - file.find:
      - path: "/etc/smartmontools/hdparm.d/"
      - iname: "openmediavault-*"
      - delete: "f"

{% if smartmontools_config.enable | to_bool %}

# Do not use a distinct filter on the `devicefile`, because this will hide
# duplicate database entries. In the worst case, the correct configuration
# for the device is filtered out.
{% set monitored_devices = salt['omv_conf.get_by_filter'](
  'conf.service.smartmontools.device',
  {'operator': 'equals', 'arg0': 'enable', 'arg1': '1'}) %}

{% for device in monitored_devices %}

# Note, this is not necessary anymore, because SMART is enabled by default
# on modern disks; but since we also support old hardware, it still makes
# sense.
smartmontools_hdparm_enable_smart_{{ device.uuid }}:
  file.managed:
    - name: "/etc/smartmontools/hdparm.d/openmediavault-enable-smart-{{ device.uuid }}"
    - contents: |
        #!/usr/bin/env dash
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        smartctl -s on {{ device.devicefile }}
    - user: root
    - group: root
    - mode: 744
    - onlyif: "export LC_ALL=C.UTF-8; smartctl -i '{{ device.devicefile }}' | grep -q 'SMART support is: Disabled'"

{% endfor %}

{% endif %}

{% for device in hdparm_config %}

smartmontools_hdparm_non_smart_settings_{{ device.uuid }}:
  file.managed:
    - name: "/etc/smartmontools/hdparm.d/openmediavault-{{ device.uuid }}"
    - contents: |
        #!/usr/bin/env dash
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        if [ -b '{{ device.devicefile }}' ]; then
          smartctl{% if device.aam > 0 %} --set=aam,{{ device.aam }}{% endif %}{% if device.apm > 0 %} --set=apm,{{ device.apm }}{% endif %}{% if device.spindowntime > 0 %} --set=standby,{{ device.spindowntime }}{% endif %}{% if device.writecache | to_bool %} --set=wcache,on{% else %} --set=wcache,off{% endif %} {{ device.devicefile }}
        fi
    - user: root
    - group: root
    - mode: 744

{% endfor %}

restart_smartmontools_smartctl_hdparm_service:
  module.run:
    - service.restart:
      - name: smartctl-hdparm
