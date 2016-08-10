# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2016 Volker Theile
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
__all__ = [ "ProductInfo" ]

import apt
import xml.etree.ElementTree
import openmediavault as omv

class ProductInfo(object):
	"""
	This class provides a simple interface to get product information.
	"""

	def __init__(self):
		self.__dict = {}
		prod_file = omv.getenv("OMV_PRODUCTINFO_FILE",
			"/usr/share/openmediavault/productinfo.xml")
		tree = xml.etree.ElementTree.parse(prod_file)
		for child in tree.iter():
			self.__dict[child.tag] = child.text

	def get_name(self):
		"""
		Get the product name.
		"""
		return self.__dict['name']

	def get_version(self):
		"""
		Get the product version.
		"""
		cache = apt.cache.Cache()
		package = cache[self.get_package_name()]
		return package.candidate.version

	def get_version_name(self):
		"""
		Get the product version/release name.
		"""
		return self.__dict['versionname']

	def get_url(self):
		"""
		Get the URL to the product homepage.
		"""
		return self.__dict['url']

	def get_copyright(self):
		"""
		Get the copyright text.
		"""
		return self.__dict['copyright']

	def get_package_name(self):
		"""
		Get the Debian package name.
		"""
		return self.__dict['packagename']

	def get_distribution_name(self):
		"""
		Get the package repository distribution name.
		"""
		return self.__dict['distribution']
