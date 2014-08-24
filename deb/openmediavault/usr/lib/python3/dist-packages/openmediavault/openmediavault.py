# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2014 Volker Theile
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

import apt
import re
import subprocess
import xml.etree.ElementTree

################################################################################
# Globals
################################################################################
DEFAULT_FILE = "/etc/default/openmediavault"
APT_UPGRADE_INDEX_FILE = "/var/lib/openmediavault/apt/upgradeindex.json"
APT_PLUGINS_INDEX_FILE = "/var/lib/openmediavault/apt/pluginsindex.json"

# Import global variables from '/etc/default/openmediavault'.
__etcDefaultDict = {}
with open(DEFAULT_FILE) as reader:
	for line in reader.readlines():
		m = re.match(r"^(OMV_([A-Z0-9_]+))=(\")?([^\"]+)(\")?$",
			line.strip())
		if not m:
			continue
		# Append variable, e.g. SCRIPTS_DIR
		__etcDefaultDict[m.group(2)] = m.group(4)
		# !!! DEPRECATED !!!
		# Append variable, e.g. OMV_SCRIPTS_DIR (equal to PHP OMV framework)
		__etcDefaultDict[m.group(1)] = m.group(4)
		
locals().update(__etcDefaultDict)

################################################################################
# Functions
################################################################################
def shell(command, stderr=False):
	"""
	Execute an external program in the default shell and returns a tuple
	with the command return code and output.

	:param stderr: If TRUE, then return the stderr output, too.
	:type  stderr: bool
	"""
	p = subprocess.Popen("LANG=C "+command, shell=True,
		stderr=subprocess.PIPE, stdout=subprocess.PIPE)
	t = p.communicate()
	return (p.returncode, t[0].decode() + (t[1].decode() if stderr else ""))

################################################################################
# Classes
################################################################################
class ProductInfo(object):
	"""
	This class provides a simple interface to get product information.
	"""

	def __init__(self):
		self.__dict = {}
		tree = xml.etree.ElementTree.parse(PRODUCTINFO_FILE)
		for child in tree.iter():
			self.__dict[child.tag] = child.text

	def getName(self):
		"""
		Get the product name.
		"""
		return self.__dict["name"]

	def getVersion(self):
		"""
		Get the product version.
		"""
		cache = apt.cache.Cache()
		package = cache[self.getPackageName()]
		return package.candidate.version

	def getVersionName(self):
		"""
		Get the product version/release name.
		"""
		return self.__dict["versionname"]

	def getURL(self):
		"""
		Get the URL to the product homepage.
		"""
		return self.__dict["url"]

	def getCopyright(self):
		"""
		Get the copyright text.
		"""
		return self.__dict["copyright"]

	def getPackageName(self):
		"""
		Get the Debian package name.
		"""
		return self.__dict["packagename"]

	def getDistributionName(self):
		"""
		Get the package repository distribution name.
		"""
		return self.__dict["distribution"]
