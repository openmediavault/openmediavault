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
# https://help.ubuntu.com/community/HowToZeroconf
# http://wiki.ubuntuusers.de/Avahi
# http://www.kde4.de/?page_id=389
# http://wiki.archlinux.org/index.php/Avahi
# http://en.gentoo-wiki.com/wiki/Avahi
# http://www.zaphu.com/2008/04/29/ubuntu-guide-configure-avahi-to-broadcast-services-via-bonjour-to-mac-os-x/
# http://www.dns-sd.org/ServiceTypes.html

configure_default_avahi_daemon:
  file.managed:
    - name: "/etc/default/avahi-daemon"
    - source:
      - salt://{{ tpldir }}/files/etc-default-avahi-daemon.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

divert_default_avahi_daemon:
  omv_dpkg.divert_add:
    - name: "/etc/default/avahi-daemon"

configure_avahi_daemon_conf:
  file.managed:
    - name: "/etc/avahi/avahi-daemon.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-avahi-avahi-daemon_conf.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

divert_avahi_daemon_conf:
  omv_dpkg.divert_add:
    - name: "/etc/avahi/avahi-daemon.conf"
