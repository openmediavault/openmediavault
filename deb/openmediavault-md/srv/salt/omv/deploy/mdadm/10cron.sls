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

# Remove the '/etc/cron.daily/mdadm' cron file that is installed by
# the Debian package.
remove_cron_daily_mdadm:
  file.absent:
    - name: "/etc/cron.daily/mdadm"

# Make sure it is not re-installed by a package update.
divert_cron_daily_mdadm:
  omv_dpkg.divert_add:
    - name: "/etc/cron.daily/mdadm"
