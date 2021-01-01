# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2021 Volker Theile
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
# http://wiki.debianforum.de/Hdparm

# hdparm is now triggered via UDEV.
# See /lib/udev/rules.d/85-hdparm.rules and /lib/udev/hdparm scripts.

{% set config = salt['omv_conf.get']('conf.system.hdparm') %}

configure_hdparm_conf:
  file.managed:
    - name: "/etc/hdparm.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-hdparm.conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

# Usually '/lib/udev/hdparm' is executed by UDEV when a device is added
# to apply the hdparm.conf settings. At runtime it is not possible to
# force UDEV to do the same thing again to reload the settings, e.g.
# by running 'udevadm trigger'. For this reason, we simply run the script
# ourselves.
{% for device in config %}
{% if salt['omv_utils.is_block_device'](device.devicefile) %}
reload_hdparm_{{ device.devicefile }}:
  cmd.run:
    - name: "/lib/udev/hdparm"
    - env:
      - DEVNAME: "{{ device.devicefile | path_realpath }}"
    - onchanges:
      - file: configure_hdparm_conf
{% endif %}
{% endfor %}
