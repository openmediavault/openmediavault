# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2018 Volker Theile
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
{% set nfs_config = salt['omv.get_config']('conf.service.nfs') %}
{% set zeroconf_config = salt['omv.get_config_by_filter'](
  'conf.service.zeroconf.service',
  '{"operator": "stringEquals", "arg0": "id", "arg1": "nfs"}')[0] %}

remove_avahi_service_nfs:
  module.run:
    - name: file.find
    - path: "/etc/avahi/services"
    - kwargs:
        iname: "nfs-*.service"
        delete: "f"

{% if nfs_config.enable | to_bool and zeroconf_config.enable | to_bool %}

# Announce duplicate shares only once.
{% set nfsshares = salt['omv.get_config_by_filter'](
  'conf.service.nfs.share',
  '{"operator": "distinct", "arg0": "sharedfolderref"}') %}
{% for nfsshare in nfsshares %}

{% set sharedfolder = salt['omv.get_config'](
  'conf.system.sharedfolder', nfsshare.sharedfolderref) %}

configure_avahi_service_nfs:
  file.managed:
    - name: "/etc/avahi/services/nfs-{{ sharedfolder.name }}.service"
    - source:
      - salt://{{ slspath }}/files/template.j2
    - template: jinja
    - context:
        type: "_nfs._tcp"
        port: 2049
        name: "{{ zeroconf_config.name }} - {{ sharedfolder.name }}"
        txtrecord: "path={{ nfs_export_dir }}/{{ sharedfolder.name }}"
    - user: root
    - group: root
    - mode: 644

{% endfor %}

{% endif %}
