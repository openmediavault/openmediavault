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
# https://www.ohnekontur.de/2010/03/23/netatalk-2-0-5-time-machine-support/
# https://gist.github.com/jasonm23/2848434#gistcomment-338252
# /System/Library/CoreServices/CoreTypes.bundle/Contents/Info.plist

{% set smb_config = salt['omv_conf.get']('conf.service.smb') %}
{% set smb_zeroconf_enabled = salt['pillar.get']('default:OMV_SAMBA_ZEROCONF_ENABLED', 1) %}
{% set smb_zeroconf_name = salt['pillar.get']('default:OMV_SAMBA_ZEROCONF_NAME', '%h - SMB/CIFS') %}

{% if (smb_config.enable | to_bool) and (smb_zeroconf_enabled | to_bool) %}

configure_avahi_service_smb:
  file.managed:
    - name: "/etc/avahi/services/smb.service"
    - source:
      - salt://{{ tpldir }}/files/smb.j2
    - template: jinja
    - context:
        type: "_smb._tcp"
        port: 445
        name: "{{ smb_zeroconf_name }}"
        shares: {{ smb_config.shares.share | json }}
    - user: root
    - group: root
    - mode: 644

{% endif %}
