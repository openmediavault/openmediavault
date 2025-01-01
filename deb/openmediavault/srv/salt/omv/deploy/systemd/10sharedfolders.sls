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
# https://www.freedesktop.org/software/systemd/man/bootup.html#System%20Manager%20Bootup

{% set sharedfolders_dir_enabled = salt['pillar.get']('default:OMV_SHAREDFOLDERS_DIR_ENABLED', 'no') %}
{% set sharedfolders_dir = salt['pillar.get']('default:OMV_SHAREDFOLDERS_DIR', '/sharedfolders') %}
{% set sharedfolders_dir_escaped = salt['cmd.run']('systemd-escape --path ' ~ sharedfolders_dir) %}
{% set sharedfolder_config = salt['omv_conf.get']('conf.system.sharedfolder') %}

remove_sharedfolder_mount_unit_files:
  module.run:
    - file.find:
      - path: "/etc/systemd/system"
      - iname: "{{ sharedfolders_dir_escaped }}-*.mount"
      - delete: "f"

{% if sharedfolders_dir_enabled | to_bool %}

create_sharedfolders_dir:
  file.directory:
    - name: {{ sharedfolders_dir }}

{% for sharedfolder in sharedfolder_config %}
{% set mntdir = salt['omv_conf.get_sharedfolder_mount_path'](sharedfolder.uuid) %}
{% set what = salt['omv_conf.get_sharedfolder_path'](sharedfolder.uuid) %}
{% set where = sharedfolders_dir | path_join(sharedfolder.name) %}
{% set unit_name = salt['cmd.run']('systemd-escape --path --suffix=mount ' ~ where) %}

configure_sharedfolder_{{ sharedfolder.name }}_mount_unit_file:
  file.managed:
    - name: {{ '/etc/systemd/system' | path_join(unit_name) }}
    - contents: |
        {{ pillar['headers']['auto_generated'] }}
        {{ pillar['headers']['warning'] }}
        [Unit]
        Description=Mount shared folder {{ sharedfolder.name }} to {{ where }}
        DefaultDependencies=no
        After=zfs-mount.service
        Wants=sysinit.target
        Conflicts=umount.target
        RequiresMountsFor={{ mntdir }}
        AssertPathIsDirectory={{ sharedfolders_dir }}
        AssertPathIsMountPoint={{ mntdir }}
        AssertPathIsDirectory={{ what }}

        [Mount]
        What={{ what }}
        Where={{ where }}
        Type=none
        Options=bind,nofail

        [Install]
        WantedBy=basic.target
    - user: root
    - group: root
    - mode: 644

{% endfor %}
{% endif %}

sharedfolder_mount_units_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:

{% if sharedfolders_dir_enabled | to_bool %}
{% for sharedfolder in sharedfolder_config %}
{% set where = sharedfolders_dir | path_join(sharedfolder.name) %}
{% set unit_name = salt['cmd.run']('systemd-escape --path --suffix=mount ' ~ where) %}

# Only enable the mount unit, do NOT take care that it is running.
enable_sharedfolder_{{ sharedfolder.name }}_mount_unit:
  service.enabled:
    - name: {{ unit_name }}
    - enable: True

# Restart the mount unit to take care it is in an up-to-date state. This
# is necessary because it is possible that the file system of the shared
# folder has been changed in the meanwhile.
restart_sharedfolder_{{ sharedfolder.name }}_mount_unit:
  module.run:
    - service.restart:
      - name: {{ unit_name }}

{% endfor %}
{% endif %}
