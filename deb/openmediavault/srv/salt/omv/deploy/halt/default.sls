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
# http://www.vdr-wiki.de/wiki/index.php/Debian_-_WAKE_ON_LAN
# http://www.brueck-computer.de/index2.php?modul=1404&link=1

configure_default_halt:
  file.managed:
    - name: "/etc/default/halt"
    - source:
      - salt://{{ tpldir }}/files/etc-default-halt.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

divert_default_halt:
  omv_dpkg.divert_add:
    - name: "/etc/default/halt"
