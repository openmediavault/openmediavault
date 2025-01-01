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

{% set nfs_export_dir = salt['pillar.get']('default:OMV_NFSD_EXPORT_DIR', '/export') %}
{% set nfs_config = salt['omv_conf.get']('conf.service.nfs') %}
{% set nfs_zeroconf_enabled = salt['pillar.get']('default:OMV_NFSD_ZEROCONF_ENABLED', 1) %}
{% set nfs_zeroconf_name = salt['pillar.get']('default:OMV_NFSD_ZEROCONF_NAME', '%h - NFS') %}

{% if (nfs_config.enable | to_bool) and (nfs_zeroconf_enabled | to_bool) %}

# Announce duplicate shares only once.
{% set nfsshares = salt['omv_conf.get_by_filter'](
  'conf.service.nfs.share',
  {"operator": "distinct", "arg0": "sharedfolderref"}) %}
{% for nfsshare in nfsshares %}

{% set sharedfolder = salt['omv_conf.get'](
  'conf.system.sharedfolder', nfsshare.sharedfolderref) %}

configure_avahi_service_nfs_{{ sharedfolder.name }}:
  file.managed:
    - name: "/etc/avahi/services/nfs-{{ sharedfolder.name }}.service"
    - source:
      - salt://{{ tpldir }}/files/template.j2
    - template: jinja
    - context:
        type: "_nfs._tcp"
        port: 2049
        name: "{{ nfs_zeroconf_name }} [{{ sharedfolder.name }}]"
        txtrecord: "path={{ nfs_export_dir }}/{{ sharedfolder.name }}"
    - user: root
    - group: root
    - mode: 644

{% endfor %}

{% endif %}
