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

# Set correct mode of various paths to prevent errors like:
# "Authentication refused: bad ownership or modes for directory /"
# "ignoring unsafe /etc/daemon.conf (/etc is group writable)"
# The issue seems to be related to live-build.
fix_root_path_permissions:
  file.directory:
    - name: /
    - mode: 0755

fix_etc_path_permissions:
  file.directory:
    - name: /etc
    - mode: 0755
