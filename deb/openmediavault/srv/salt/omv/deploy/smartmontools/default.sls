# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2018 Volker Theile
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

{% set config = salt['omv.get_config']('conf.service.smartmontools') %}

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
