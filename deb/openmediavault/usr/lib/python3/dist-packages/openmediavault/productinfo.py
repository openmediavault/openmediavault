# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2020 Volker Theile
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
__all__ = ["ProductInfo"]

import xml.etree.ElementTree
import apt

import openmediavault


class ProductInfo:
    """
    This class provides a simple interface to get product information.
    """

    def __init__(self):
        self._dict = {}
        prod_file = openmediavault.getenv(
            "OMV_PRODUCTINFO_FILE", "/usr/share/openmediavault/productinfo.xml"
        )
        tree = xml.etree.ElementTree.parse(prod_file)
        for child in tree.iter():
            # Skip all elements with children.
            if list(child):
                continue
            self._dict[child.tag] = child.text

    def as_dict(self):
        """
        Get the product information as Python dictionary.
        :returns: Returns the product information as Python dictionary.
        """
        result = self._dict.copy()
        result['version'] = self.version
        return result

    @property
    def name(self):
        """
        Get the product name.
        """
        return self._dict['name']

    @property
    def version(self):
        """
        Get the product version.
        """
        cache = apt.cache.Cache()
        package = cache[self.package_name]
        return package.candidate.version

    @property
    def version_name(self):
        """
        Get the product version/release name.
        """
        return self._dict['versionname']

    @property
    def url(self):
        """
        Get the URL to the product homepage.
        """
        return self._dict['url']

    @property
    def copyright(self):
        """
        Get the copyright text.
        """
        return self._dict['copyright']

    @property
    def package_name(self):
        """
        Get the Debian package name.
        """
        return self._dict['packagename']

    @property
    def distribution_name(self):
        """
        Get the package repository distribution name.
        """
        return self._dict['distribution']
