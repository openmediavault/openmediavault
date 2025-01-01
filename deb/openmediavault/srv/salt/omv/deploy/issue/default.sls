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

{% set apply_issue = salt['pillar.get']('default:OMV_ISSUE_APPLY_PRELOGIN_MESSAGE', 'yes') -%}

{% if apply_issue | to_bool %}

configure_issue:
  file.managed:
    - name: "/etc/issue"
    - source:
      - salt://{{ tpldir }}/files/etc-issue.j2
    - template: jinja
    - user: root
    - group: root
    - mode: 644
    - onlyif: udevadm settle

reload_issue:
  cmd.run:
    - name: "agetty --reload"
    - onchanges:
      - file: configure_issue

{% endif %}
