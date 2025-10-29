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

# Set up fallback DNS servers if none are configured. This can happen if the
# systemd-resolved package is installed as a dependency of openmediavault.
setup_fallback_dns_servers:
  cmd.run:
    - name: |
        iface=$(ip route show default 0.0.0.0/0 | awk '/default/ {print $5; exit}')
        [ -n "$iface" ] && resolvectl dns "$iface" 1.1.1.2 9.9.9.9
    - unless: resolvectl query openmediavault.org >/dev/null 2>&1
    - shell: /bin/bash

# Install additional Python modules into the SaltStack onedir installation
# that are required by the openmediavault Python module which is used by
# various Salt runners and execution modules.
# https://docs.saltproject.io/salt/install-guide/en/latest/topics/install-dependencies.html
install_python_modules_pip:
  pip.installed:
    - pkgs:
      - cached-property
      - click
      - lxml
      - natsort
      - pyudev

install_python_module_openmediavault:
  file.symlink:
    - name: /opt/saltstack/salt/lib/python3.10/site-packages/openmediavault
    - target: /usr/lib/python3/dist-packages/openmediavault
    - force: True

run_state_patch:
  salt.state:
    - tgt: '*'
    - tgt_type: compound
    - sls: omv.patch
    - failhard: True

# Sync runners from salt://_runners to the master.
sync_runners:
  salt.runner:
    - name: saltutil.sync_runners

# Sync execution modules from salt://_modules to the master.
sync_modules:
  salt.runner:
    - name: saltutil.sync_modules

# Sync state modules from salt://_states to the master.
sync_states:
  salt.runner:
    - name: saltutil.sync_states

# Create openmediavault pillar data.
populate_pillar:
  salt.runner:
    - name: omv.populate_pillar
    - require:
      - salt: sync_runners

run_state_sync:
  salt.state:
    - tgt: '*'
    - tgt_type: compound
    - sls: omv.sync
    - failhard: True
