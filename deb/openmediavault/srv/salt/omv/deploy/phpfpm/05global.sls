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

{% set config_file = salt['pillar.get']('default:OMV_PHPFPM_CONFIG_FILE', '/etc/php/8.4/fpm/php-fpm.conf') %}
{% set pid_file = salt['pillar.get']('default:OMV_PHPFPM_PID_FILE', '/run/php/php8.4-fpm.pid') %}

configure_phpfpm_pid:
  file.line:
    - name: "{{ config_file }}"
    - match: "^[;#]?\\s*pid\\s*=.*$"
    - content: "pid = {{ pid_file }}"
    - mode: replace
    - indent: False
    - backup: False

divert_phpfpm_conf:
  omv_dpkg.divert_add:
    - name: "{{ config_file }}"
