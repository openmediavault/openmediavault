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

{% set document_root = salt['pillar.get']('default:OMV_DOCUMENTROOT_DIR', '/var/www/openmediavault') %}
{% set user = salt['pillar.get']('default:OMV_WEBGUI_FILE_OWNERGROUP_NAME', 'openmediavault-webgui') %}

webgui_document_root_perms_recursive:
  cmd.run:
    - name: "chown -R '{{ user }}:{{ user }}' '{{ document_root }}'"
