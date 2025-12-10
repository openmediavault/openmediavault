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

proftpd_mod_delay_present:
  file.replace:
    - name: /etc/proftpd/modules.conf
    - pattern: '^#\s*(LoadModule\s+mod_delay\.c)\s*$'
    - repl: "LoadModule mod_delay.c"
    - append_if_not_found: True

proftpd_mod_ident_present:
  file.replace:
    - name: /etc/proftpd/modules.conf
    - pattern: '^#\s*(LoadModule\s+mod_ident\.c)\s*$'
    - repl: "LoadModule mod_ident.c"
    - append_if_not_found: True

proftpd_mod_ls_present:
  file.replace:
    - name: /etc/proftpd/modules.conf
    - pattern: '^#\s*(LoadModule\s+mod_ls\.c)\s*$'
    - repl: "LoadModule mod_ls.c"
    - append_if_not_found: True

proftpd_mod_tls_present:
  file.replace:
    - name: /etc/proftpd/modules.conf
    - pattern: '^#\s*(LoadModule\s+mod_tls\.c)\s*$'
    - repl: "LoadModule mod_tls.c"
    - append_if_not_found: True

proftpd_mod_vroot_present:
  file.replace:
    - name: /etc/proftpd/modules.conf
    - pattern: '^#\s*(LoadModule\s+mod_vroot\.c)\s*$'
    - repl: "LoadModule mod_vroot.c"
    - append_if_not_found: True

proftpd_mod_xfer_present:
  file.replace:
    - name: /etc/proftpd/modules.conf
    - pattern: '^#\s*(LoadModule\s+mod_xfer\.c)\s*$'
    - repl: "LoadModule mod_xfer.c"
    - append_if_not_found: True
