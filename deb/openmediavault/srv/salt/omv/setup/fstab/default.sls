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
# http://linux.koolsolutions.com/2009/01/30/installing-linux-on-usb-part-4-noatime-and-relatime-mount-options/
# http://techgage.com/article/enabling_and_testing_ssd_trim_support_under_linux
# https://wiki.archlinux.org/index.php/Solid_State_Drives
# http://wiki.ubuntuusers.de/SSD/TRIM
# http://superuser.com/questions/228657/which-linux-filesystem-works-best-with-ssd

{% set nonrot_options = salt['pillar.get']('default:OMV_FSTAB_MNTOPS_NONROTATIONAL',
                                           'noatime,nodiratime').split(',') %}

# Reload udev rules to apply rules shipped with openmediavault, e.g.
# to detect non-rotational devices.
udevadm_reload_rules:
  cmd.run:
    - name: "udevadm control --reload-rules"

udevadm_trigger:
  cmd.run:
    - name: "udevadm trigger"
    - onchanges:
      - udevadm_reload_rules

{% set fstab_entries = salt['mount.fstab']() %}
{% if '/' in fstab_entries %}

# Get the mount configuration for the root filesystem.
# ATTENTION: It may happen that the UUID is quoted in /etc/fstab.
# {
#   "device": "UUID=199a4bbc-59c9-4a3b-b592-950eaffb2530",
#   "pass": "1",
#   "opts": [
#     "errors=remount-ro"
#   ],
#   "dump": "0",
#   "fstype": "ext4"
# }
{% set root_fstab_entry = fstab_entries['/'] %}
# Get the device file of the root filesystem.
{% set root_fs_device = salt['omv_utils.get_root_filesystem']() %}

{% if salt['omv_utils.is_device_file'](root_fs_device) %}

# Get the parent device file of the root filesystem.
{% set parent_root_fs_device = salt['omv_utils.get_fs_parent_device_file'](root_fs_device) %}

{% if nonrot_options | difference(root_fstab_entry.opts) | length > 0 and
      salt['omv_utils.is_block_device'](parent_root_fs_device) and
      not salt['omv_utils.is_rotational'](parent_root_fs_device) %}

update_root_fstab_entry:
  module.run:
    - mount.set_fstab:
      - name: "/"
      - device: {{ root_fstab_entry.device }}
      - fstype: {{ root_fstab_entry.fstype }}
      - opts: {{ nonrot_options | union(root_fstab_entry.opts) | join(',') }}
      - dump: {{ root_fstab_entry.dump }}
      - pass_num: {{ root_fstab_entry.pass }}

{% endif %}

{% endif %}

{% endif %}
