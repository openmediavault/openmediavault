# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2023 Volker Theile
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

# https://github.com/saltstack/salt/pull/62952
patch_utils_event_pr_62952:
  file.patch:
    - name: /lib/python3/dist-packages/salt/utils/event.py
    - source:
      - salt://{{ tpldir }}/files/salt-pr-62952.patch
    - unless: test -e /var/lib/openmediavault/salt/patch_utils_event_pr_62952_stamp

touch_utils_event_pr_62952_stamp:
  file.touch:
    - name: /var/lib/openmediavault/salt/patch_utils_event_pr_62952_stamp
    - makedirs: true
