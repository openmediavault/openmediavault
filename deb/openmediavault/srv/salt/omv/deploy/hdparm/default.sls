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
# http://wiki.debianforum.de/Hdparm

{% set config = salt['omv.get_config']('conf.system.hdparm') %}

configure_hdparm_conf:
  file.managed:
    - name: "/etc/hdparm.conf"
    - source:
      - salt://{{ slspath }}/files/etc-hdparm.conf.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

configure_pm-utils_hook:
  file.managed:
    - name: "/usr/lib/pm-utils/sleep.d/99openmediavault-hdparm"
    - source:
      - salt://{{ slspath }}/files/usr-lib-pm-utils-sleep.d-99openmediavault-hdparm.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 755
