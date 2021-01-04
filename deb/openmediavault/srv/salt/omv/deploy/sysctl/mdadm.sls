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
# http://ticktoo.com/blog/32-Linux+Software+Raid%3A+mdadm+Performance+Tuning
# http://www.cyberciti.biz/tips/linux-raid-increase-resync-rebuild-speed.html

{% set speed_limit_min = salt['pillar.get']('default:OMV_SYSCTL_DEV_RAID_SPEEDLIMITMIN', '10000') %}

# Improve maximun md array reconstruction speed.
configure_sysctl_mdadm:
  file.managed:
    - name: "/etc/sysctl.d/99-openmediavault-mdadm.conf"
    - contents:
      - "{{ pillar['headers']['auto_generated'] }}"
      - "{{ pillar['headers']['warning'] }}"
      - "#"
      - "# Default values:"
      - "# dev.raid.speed_limit_min = 1000"
      - "dev.raid.speed_limit_min = {{ speed_limit_min }}"
    - user: root
    - group: root
    - mode: 644
