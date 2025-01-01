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
# https://www.freedesktop.org/software/systemd/man/systemd-resolved.service.html#/etc/resolv.conf
# https://unix.stackexchange.com/questions/548830/whats-the-difference-between-run-systemd-resolve-stub-resolv-conf-and-run-sys
# https://superuser.com/questions/1490670/does-systemd-networkd-systemd-resolved-add-search-domains-specified-in-dhcp#comment2249615_1490670
# https://wiki.archlinux.org/title/systemd-resolved

{% set resolvconf_target = salt['pillar.get']('default:OMV_SYSTEMD_NETWORKD_RESOLVCONF_TARGET', '/run/systemd/resolve/stub-resolv.conf') %}

symlink_systemd_resolvconf:
  file.symlink:
    - name: /etc/resolv.conf
    - target: {{ resolvconf_target }}
    - force: True
    - onlyif: "test -e {{ resolvconf_target }}"
