# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.

# All modules that are not compiled-in. Run `proftpd --list` to list them.
{% set modules = ['mod_ban', 'mod_ctrls_admin', 'mod_delay', 'mod_dynmasq', 'mod_facl', 'mod_ident', 'mod_ls', 'mod_quotatab', 'mod_ratio', 'mod_tls', 'mod_vroot', 'mod_wrap', 'mod_xfer'] %}

{% for module in modules %}

proftpd_{{ module }}_present:
  file.replace:
    - name: /etc/proftpd/modules.conf
    - pattern: '^#\s*(LoadModule\s+{{ module }}\.c)\s*$'
    - repl: "LoadModule {{ module }}.c"
    - append_if_not_found: True

{% endfor %}
