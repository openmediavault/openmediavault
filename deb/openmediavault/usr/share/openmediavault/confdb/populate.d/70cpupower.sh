#!/usr/bin/env python3
#
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
import os
import sys
from typing import List

import openmediavault.config.database
import openmediavault.config.object


def main():
	# Get the `scaling_available_governors` for the first CPU.
	sysfs_path: str = '/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors'
	if not os.path.exists(sysfs_path):
		return 0

	governor: str = ''
	# The list of governors is ordered by efficiency.
	# Please note that the list also includes governors that are no longer
	# in the current kernel or shipped with Debian, but are still listed
	# here; you never know.
	governors_by_efficiency: List[str] = [
	    'powersave', 'conservative', 'schedutil', 'ondemand', 'userspace', 'performance']
	scaling_available_governors: List[str] = []

	with open(sysfs_path) as f:
		scaling_available_governors = f.read().strip().split()

	# Iterate over a list of governors that is ordered by efficiency. The
	# first governor that is available will be used.
	for gbe in governors_by_efficiency:
		if gbe in scaling_available_governors:
			governor = gbe
			break

	db: openmediavault.config.Database = openmediavault.config.Database()
	obj: openmediavault.config.object.Object = db.get('conf.system.powermngmnt')
	obj.set('cpufreqgovernor', governor)
	db.set(obj)

	return 0


if __name__ == "__main__":
    sys.exit(main())
