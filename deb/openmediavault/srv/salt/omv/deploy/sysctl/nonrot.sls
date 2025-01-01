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
# https://www.kernel.org/doc/Documentation/sysctl/vm.txt
# https://www.kernel.org/doc/Documentation/laptops/laptop-mode.txt
# http://lonesysadmin.net/2013/12/22/better-linux-disk-caching-performance-vm-dirty_ratio
# https://major.io/2008/08/07/reduce-disk-io-for-small-reads-using-memory

{% set dirty_background_ratio = salt['pillar.get']('default:OMV_SYSCTL_VM_DIRTYBACKGROUNDRATIO', '1') %}
{% set dirty_ratio = salt['pillar.get']('default:OMV_SYSCTL_VM_DIRTYRATIO', '20') %}
{% set dirty_writeback_centisecs = salt['pillar.get']('default:OMV_SYSCTL_VM_DIRTYWRITEBACKCENTISECS', '12000') %}
{% set dirty_expire_centisecs = salt['pillar.get']('default:OMV_SYSCTL_VM_DIRTYEXPIRECENTISECS', '12000') %}
{% set laptop_mode = salt['pillar.get']('default:OMV_SYSCTL_VM_LAPTOPMODE', '5') %}
{% set swappiness = salt['pillar.get']('default:OMV_SYSCTL_VM_SWAPPINESS', '1') %}
{% set vfs_cache_pressure = salt['pillar.get']('default:OMV_SYSCTL_VM_VFSCACHEPRESSURE', '50') %}

remove_sysctl_nonrot:
  file.absent:
    - name: /etc/sysctl.d/99-openmediavault-nonrot.conf

{% set mounts = salt['mount.active'](True) %}
{% if '/' in mounts %}

{% set rotational = salt['sysfs.attr']('/sys/dev/block/' ~ mounts['/'].major ~ ':0/queue/rotational') %}
{% if rotational == 0 %}

# Modify some kernel parameters to reduce disk I/O for non-rotating devices.
# The improvements are only done if the root device is a non-rotating
# device, e.g. SSD or DOM.
configure_sysctl_nonrot:
  file.managed:
    - name: /etc/sysctl.d/99-openmediavault-nonrot.conf
    - contents:
      - "{{ pillar['headers']['auto_generated'] }}"
      - "{{ pillar['headers']['warning'] }}"
      - "# Modify some kernel parameters to reduce disk I/O for non-rotating devices."
      - "#"
      - "# Default values:"
      - "# vm.dirty_background_ratio = 10"
      - "# vm.dirty_ratio = 20"
      - "# vm.dirty_writeback_centisecs = 500"
      - "# vm.dirty_expire_centisecs = 3000"
      - "# vm.laptop_mode = 0"
      - "# vm.swappiness = 60"
      - "# vm.vfs_cache_pressure = 100"
      - "vm.dirty_background_ratio = {{ dirty_background_ratio }}"
      - "vm.dirty_ratio = {{ dirty_ratio }}"
      - "vm.dirty_writeback_centisecs = {{ dirty_writeback_centisecs }}"
      - "vm.dirty_expire_centisecs = {{ dirty_expire_centisecs }}"
      - "vm.laptop_mode = {{ laptop_mode }}"
      - "vm.swappiness = {{ swappiness }}"
      - "vm.vfs_cache_pressure = {{ vfs_cache_pressure }}"
    - user: root
    - group: root
    - mode: 644

load_sysctl_settings:
  cmd.run:
    - name: sysctl --load
    - onchanges:
      - file: configure_sysctl_nonrot

{% endif %}

{% endif %}
