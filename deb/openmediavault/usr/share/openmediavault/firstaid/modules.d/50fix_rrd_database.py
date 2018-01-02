#!/usr/bin/env python3
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2018 Volker Theile
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
import os
import sys
import glob
import datetime
import dialog
import openmediavault
import openmediavault.config.database
import openmediavault.firstaid
import openmediavault.subprocess

class Module(openmediavault.firstaid.IModule):
	@property
	def description(self):
		return "Check RRD database"

	def execute(self):
		print("Checking all RRD files. Please wait ...")
		path = openmediavault.getenv("OMV_RRDCACHED_BASEDIR",
			"/var/lib/rrdcached/db/")
		rrd_files = glob.glob(os.path.join(path, "localhost",
			"*", "*.rrd"))
		invalid = 0
		# Get the performance statistics configuration?
		db = openmediavault.config.Database()
		obj = db.get("conf.system.monitoring.perfstats")
		# Disable rrdcached if performance stats are enabled.
		if obj.get("enable"):
			openmediavault.subprocess.check_call(
				["monit", "stop", "rrdcached"])
		for rrd_file in rrd_files:
			ts_last = int(openmediavault.subprocess.check_output(
				[ "rrdtool", "last", rrd_file ]).decode())
			dt_now = datetime.datetime.now()
			if datetime.datetime.utcfromtimestamp(ts_last) > dt_now:
				invalid += 1
				dirname = os.path.basename(os.path.dirname(rrd_file))
				basename = os.path.basename(rrd_file)
				d = dialog.Dialog(dialog="dialog")
				code = d.yesno("The RRD file '../{}/{}' contains " \
					"timestamps in future.\nDo you want to delete " \
					"it?".format(dirname, basename),
					backtitle=self.description,
					height=7, width=65)
				if code == d.ESC:
					return 0
				if code == d.OK:
					os.remove(rrd_file)
		if invalid == 0:
			print("All RRD database files are valid.")
		else:
			print("{} invalid RRD database files were removed.".format(
				invalid))
		# Re-enable rrdcached if performance stats are enabled.
		if obj.get("enable"):
			openmediavault.subprocess.check_call(
				["monit", "start", "rrdcached"])
		return 0

if __name__ == "__main__":
	module = Module();
	sys.exit(module.execute())
