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

# Documentation/Howto:
# http://www.proftpd.org/docs/contrib/mod_tls.html
# https://www.fairssl.net/en/create-ssl

{% set config = salt['omv_conf.get']('conf.service.ftp') %}

configure_proftpd_mod_tls:
  file.managed:
    - name: "/etc/proftpd/tls.conf"
    - source:
      - salt://{{ tpldir }}/files/mod_tls.j2
    - template: jinja
    - context:
        config: {{ config.modules.mod_tls | json }}
    - user: root
    - group: root
    - mode: 644
