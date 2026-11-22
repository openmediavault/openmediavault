# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2026 Volker Theile
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.

# Expose every shared folder as a symlink '<OMV_SHAREDFOLDERS_DIR>/<name>'
# pointing to the shared folder's absolute path. Earlier versions used bind
# mount units for this, those are stopped and removed by this state.
#
# Only symlinks are pruned from OMV_SHAREDFOLDERS_DIR, regular files and
# directories are never deleted.
#
# Testing:
# ls -la /sharedfolders
# omv-salt deploy run systemd

{% set sharedfolders_dir_enabled = salt['pillar.get']('default:OMV_SHAREDFOLDERS_DIR_ENABLED', 'no') %}
{% set sharedfolders_dir = salt['pillar.get']('default:OMV_SHAREDFOLDERS_DIR', '/sharedfolders') %}
{% set sharedfolders_dir_escaped = salt['cmd.run']('systemd-escape --path ' ~ sharedfolders_dir) %}
{% set sharedfolder_config = salt['omv_conf.get']('conf.system.sharedfolder') %}

# Migration: Stop and disable the bind mount units created by earlier
# versions and remove their (empty) mount points to make room for the
# symlinks. A mount point that still contains data is left untouched.
{% set legacy_unit_files = salt['file.find']('/etc/systemd/system', iname=sharedfolders_dir_escaped ~ '-*.mount', type='f') %}
{% for unit_file in legacy_unit_files %}
{% set unit_name = salt['file.basename'](unit_file) %}
{% set where = salt['cmd.run']('systemd-escape --unescape --path ' ~ (unit_name | replace('.mount', '') | quote)) %}

stop_legacy_sharedfolder_mount_unit_{{ loop.index }}:
  service.dead:
    - name: "{{ unit_name }}"
    - enable: False

remove_legacy_sharedfolder_mount_point_{{ loop.index }}:
  cmd.run:
    - name: rmdir {{ where | quote }}
    - onlyif: >-
        test -d {{ where | quote }} &&
        ! test -L {{ where | quote }} &&
        ! mountpoint -q {{ where | quote }} &&
        test -z "$(ls -A {{ where | quote }})"
    - require:
      - service: stop_legacy_sharedfolder_mount_unit_{{ loop.index }}
    - require_in:
      - module: remove_sharedfolder_mount_unit_files

{% endfor %}

remove_sharedfolder_mount_unit_files:
  module.run:
    - file.find:
      - path: "/etc/systemd/system"
      - iname: "{{ sharedfolders_dir_escaped }}-*.mount"
      - delete: "f"

sharedfolder_mount_units_systemctl_daemon_reload:
  module.run:
    - service.systemctl_reload:
    - require:
      - module: remove_sharedfolder_mount_unit_files

{% set managed_names = [] %}

{% if sharedfolders_dir_enabled | to_bool %}

create_sharedfolders_dir:
  file.directory:
    - name: "{{ sharedfolders_dir }}"
    - user: root
    - group: root
    - mode: "0755"

{% for sharedfolder in sharedfolder_config %}
{% set target = salt['omv_conf.get_sharedfolder_path'](sharedfolder.uuid) %}
{% set link_path = sharedfolders_dir | path_join(sharedfolder.name) %}
{% do managed_names.append(sharedfolder.name) %}

# Do NOT use 'force' here, otherwise Salt would recursively delete a
# directory that is in the way, e.g. a still mounted legacy mount point.
create_sharedfolder_{{ sharedfolder.uuid }}_symlink:
  file.symlink:
    - name: "{{ link_path }}"
    - target: "{{ target }}"
    - user: root
    - group: root
    - require:
      - file: create_sharedfolders_dir
      - module: remove_sharedfolder_mount_unit_files

{% endfor %}
{% endif %}

# Remove symlinks of deleted or renamed shared folders. If the feature is
# disabled, all symlinks are removed.
{% if salt['file.directory_exists'](sharedfolders_dir) %}
{% for entry in salt['file.readdir'](sharedfolders_dir) %}
{% set entry_path = sharedfolders_dir | path_join(entry) %}
{% if entry not in ['.', '..'] and entry not in managed_names and salt['file.is_link'](entry_path) %}

remove_stale_sharedfolder_symlink_{{ loop.index }}:
  file.absent:
    - name: "{{ entry_path }}"

{% endif %}
{% endfor %}
{% endif %}
