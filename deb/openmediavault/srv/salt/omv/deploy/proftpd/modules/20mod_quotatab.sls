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

{% set quota_engine = salt['pillar.get']('default:OMV_PROFTPD_MODQUOTATAB_QUOTAENGINE', 'off') %}

configure_proftpd_mod_quotatab:
  file.append:
    - name: "/etc/proftpd/proftpd.conf"
    - text: |
        <IfModule mod_quotatab.c>
          QuotaEngine {{ quota_engine }}
        </IfModule>
    - watch_in:
      - service: start_proftpd_service
