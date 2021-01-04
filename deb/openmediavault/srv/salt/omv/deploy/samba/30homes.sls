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
# http://us5.samba.org/samba/docs/man/manpages-3/smb.conf.5.html

{% set config = salt['omv_conf.get']('conf.service.smb') %}

{% if config.homesenable | to_bool %}

configure_samba_homes:
  file.append:
    - name: "/etc/samba/smb.conf"
    - sources:
      - salt://{{ tpldir }}/files/homes.j2
    - template: jinja
    - context:
        config: {{ config | json }}
    - watch_in:
      - service: start_samba_service

{% endif %}
