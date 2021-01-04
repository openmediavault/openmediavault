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
# http://de.linwiki.org/wiki/Linuxfibel_-_Netzwerk_Server_-_NFS_Server
# http://wiki.ubuntuusers.de/NFS
# http://www.centos.org/docs/5/html/Deployment_Guide-en-US/s1-nfs-server-config-exports.html
# https://help.ubuntu.com/community/NFSv4Howto
# http://jkossen.nl/2009/05/12/simple-nfsv4-configuration-for-debian-and-ubuntu.html
# http://doc.opensuse.org/products/opensuse/openSUSE/opensuse-reference/cha.nfs.html

# Testing:
# showmount -e <nfs-server>

{% set config = salt['omv_conf.get']('conf.service.nfs') %}

{% if config.enable | to_bool %}

configure_default_nfs-kernel-server:
  file.managed:
    - name: "/etc/default/nfs-kernel-server"
    - source:
      - salt://{{ tpldir }}/files/etc-default-nfs-kernel-server.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - user: root
    - group: root
    - mode: 644

configure_nfsd_exports:
  file.managed:
    - name: "/etc/exports"
    - source:
      - salt://{{ tpldir }}/files/etc-exports.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644

start_rpc_statd_service:
  service.running:
    - name: rpc-statd
    - enable: True

start_nfs_kernel_server_service:
  service.running:
    - name: nfs-kernel-server
    - enable: True
    - watch:
      - file: "/etc/default/nfs-kernel-server"
      - file: "/etc/exports"

{% else %}

stop_nfs_kernel_server_service:
  service.dead:
    - name: nfs-kernel-server
    - enable: False

stop_rpc_statd_service:
  service.dead:
    - name: rpc-statd
    - enable: False

{% endif %}
