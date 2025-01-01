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
