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

{% set controls_engine = salt['pillar.get']('default:OMV_PROFTPD_MODCTRLS_CONTROLSENGINE', 'on') %}
{% set controls_max_clients = salt['pillar.get']('default:OMV_PROFTPD_MODCTRLS_CONTROLSMAXCLIENTS', '2') %}
{% set controls_log = salt['pillar.get']('default:OMV_PROFTPD_MODCTRLS_CONTROLSLOG', '/var/log/proftpd/controls.log') %}
{% set controls_interval = salt['pillar.get']('default:OMV_PROFTPD_MODCTRLS_CONTROLSINTERVAL', '5') %}
{% set controls_socket = salt['pillar.get']('default:OMV_PROFTPD_MODCTRLS_CONTROLSSOCKET', '/run/proftpd/proftpd.sock') %}

configure_proftpd_mod_ctrls:
  file.append:
    - name: "/etc/proftpd/proftpd.conf"
    - text: |
        <IfModule mod_ctrls.c>
          ControlsEngine {{ controls_engine }}
          ControlsMaxClients {{ controls_max_clients }}
          ControlsLog {{ controls_log }}
          ControlsInterval {{ controls_interval }}
          ControlsSocket {{ controls_socket }}
        </IfModule>
