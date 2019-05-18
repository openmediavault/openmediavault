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

# Documentation/Howto:
# https://help.ubuntu.com/community/Smartmontools
# http://en.gentoo-wiki.com/wiki/Smartmontools
# http://www.linux-user.de/ausgabe/2004/10/056-smartmontools

{% set config = salt['omv_conf.get']('conf.service.smartmontools') %}

configure_default_smartmontools:
  file.managed:
    - name: "/etc/default/smartmontools"
    - source:
      - salt://{{ slspath }}/files/etc-default-smartmontools.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

configure_smartd_conf:
  file.managed:
    - name: "/etc/smartd.conf"
    - source:
      - salt://{{ slspath }}/files/etc-smartd_conf.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

{% if config.enable | to_bool %}

{% set smart_devices = salt['omv_conf.get_by_filter'](
  'conf.service.smartmontools.device',
  {'operator': 'equals', 'arg0': 'enable', 'arg1': '1'}) %}

{% for device in smart_devices %}

enable_smart_on_{{ device.devicefile }}:
  cmd.run:
    - name: "export LANG=C; smartctl -s on {{ device.devicefile }}"
    - onlyif: "export LANG=C; smartctl -i {{ device.devicefile }} | grep -q 'SMART support is: Disabled'"

{% endfor %}

start_smartd_service:
  service.running:
    - name: smartd
    - enable: True
    - watch:
      - file: configure_smartd_conf

{% else %}

stop_smartd_service:
  service.dead:
    - name: smartd
    - enable: False

{% endif %}
