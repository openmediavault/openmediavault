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

clear_cache:
  module.run:
    - saltutil.clear_cache:

# Sync grains from salt://_grains to the minion.
sync_grains:
  saltutil.sync_grains:
    - refresh: True

# Sync state modules from salt://_states to the minion.
sync_states:
  saltutil.sync_states:
    - refresh: True

# Sync execution modules from salt://_modules to the minion. The 'module.run'
# form with 'reload_modules: True' is required so that freshly synced modules
# are available within the same run. 'refresh: True' alone only fires an
# asynchronous event which has no effect in a one-shot salt-call invocation.
sync_modules:
  module.run:
    - saltutil.sync_modules:
      - refresh: True
    - reload_modules: True

# Generate the openmediavault pillar data.
# Skip this state if 'sync_modules' fails to prevent a crash with a
# 'module not found' error.
populate_pillar:
  module.run:
    - omv_pillar.populate:
    - require:
      - module: sync_modules
