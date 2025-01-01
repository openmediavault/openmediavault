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

# Prevent the TTY error "mesg: ttyname failed: Inappropriate ioctl for device".
# Note, we only need to modify /root/.profile, the skeleton file at
# /etc/skel/.profile does not contain the problematic line.

# See post in the forum:
# https://forum.openmediavault.org/index.php/Thread/20966-mesg-ttyname-failed-Inappropriate-ioctl-for-device/?postID=163028#post163028

modify_root_profile:
  file.replace:
    - name: '/root/.profile'
    - pattern: '^{{ 'mesg n 2> /dev/null || true' | regex_escape }}$'
    - repl: 'test -t 0 && mesg n 2> /dev/null || true'
    - ignore_if_missing: True
    - backup: False
