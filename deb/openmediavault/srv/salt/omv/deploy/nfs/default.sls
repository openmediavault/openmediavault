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
# https://wiki.debian.org/NFSServerSetup
# https://www.willhaley.com/blog/ubuntu-nfs-server
# http://wiki.ubuntuusers.de/NFS
# https://help.ubuntu.com/community/NFSv4Howto
# https://www.kernel.org/doc/Documentation/filesystems/nfs/nfs41-server.txt
# https://ngelinux.com/difference-between-nfs-v2-v3-v4-v4-1-and-v4-2/
# https://kifarunix.com/install-and-setup-nfs-server-on-debian-12/
# /usr/lib/systemd/scripts/nfs-utils_env.sh

# Testing:
# showmount -e <nfs-server>
# cat /proc/fs/nfsd/versions
# nfsconf --dump
# rpcinfo -s
# grep CONFIG_NFS /boot/config-$(uname -r)

{% set nfs_config = salt['omv_conf.get']('conf.service.nfs') %}
{% set dns_config = salt['omv_conf.get']('conf.system.network.dns') %}

{% if nfs_config.enable | to_bool %}

# This file is generated on upgrade to Debian 12.
remove_upgrade_debian12_conf:
  file.absent:
    - name: /etc/nfs.conf.d/local.conf

configure_nfs_conf:
  file.managed:
    - name: "/etc/nfs.conf.d/99-openmediavault.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-nfs_conf_d-openmediavault.conf.j2
    - template: jinja
    - context:
        config: {{ nfs_config | json }}
    - user: root
    - group: root
    - mode: 644

configure_idmapd_conf:
  file.managed:
    - name: "/etc/idmapd.conf"
    - source:
      - salt://{{ tpldir }}/files/etc-idmapd.conf.j2
    - template: jinja
    - context:
        config: {{ dns_config | json }}
    - user: root
    - group: root
    - mode: 644

divert_idmapd_conf:
  omv_dpkg.divert_add:
    - name: "/etc/idmapd.conf"

configure_nfs_exports:
  file.managed:
    - name: "/etc/exports"
    - source:
      - salt://{{ tpldir }}/files/etc-exports.j2
    - template: jinja
    - context:
        config: {{ nfs_config | json }}
    - user: root
    - group: root
    - mode: 644

divert_nfs_exports:
  omv_dpkg.divert_add:
    - name: "/etc/exports"

stop_nfs_blkmap_service:
  service.dead:
    - name: nfs-blkmap
    - enable: False

mask_nfs_blkmap_service:
  service.masked:
    - name: nfs-blkmap

start_nfs_server_service:
  service.running:
    - name: nfs-server
    - enable: True
    - watch:
      - file: configure_nfs_conf
      - file: configure_nfs_exports

restart_nfs_utils_service:
  service.running:
    - name: nfs-utils
    - watch:
      - file: configure_nfs_conf
      - file: configure_nfs_exports

{% else %}

stop_nfs_server_service:
  service.dead:
    - name: nfs-server
    - enable: False

{% endif %}
