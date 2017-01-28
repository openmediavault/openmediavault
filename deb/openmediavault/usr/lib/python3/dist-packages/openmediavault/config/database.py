# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2017 Volker Theile
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
__all__ = [
	"Database",
	"DatabaseException",
	"DataBaseFilter"
]

import openmediavault.collections
import openmediavault.config.object

class DatabaseException(Exception):
	pass

class Database(object):
	def get(self, id, uuid=None):
		"""
		Get the specified configuration object.
		:param id:		The data model identifier, e.g. 'conf.service.ftp'.
		:param uuid:	The UUID of an configuration object. Defaults to None.
		:returns:		Depending on the configuration object and whether
						*uuid* is set, a list of configuration objects or
						a single object is returned.
		"""
		pass

	def get_by_filter(self, filter, max_result=None):
		"""
		Get the iterable configuration objects that are matching the specified
		constraints.
		:param id:			The data model identifier, e.g. 'conf.service.ftp'.
		:param filter:		A filter specifying constraints on the objects
							to retrieve.
							``
							Example 1:
							{
								"operator": "stringEquals",
								"arg0": "fsname",
								"arg1": "xyz"
							}

							Example 2:
							{
								"operator": "and",
								"arg0": {
									"operator": "stringEquals",
									"arg0": "type",
									"arg1": "bond"
								},
								"arg1": {
									"operator": "stringEquals",
									"arg0": "devicename",
									"arg1": "bond0"
								}
							}
							``
		:param max_result:	The maximum number of objects that are returned.
							Defaults to None.
		:returns:			An array containing the requested configuration
							objects. If *max_result* is set to 1, then the
							first found object is returned.
							In this case the method does not return a list
							of configuration objects.
		"""
		assertTrue(isinstance(filter, openmediavault.config.DataBaseFilter),
			"filter must be an openmediavault.config.DataBaseFilter object")

		def exists(self, id, filter):
			"""
			Check if on or more configuration object of the specified data
			model exists.
			:param id:		The data model identifier.
			:param filter:	A filter specifying constraints on the objects
							to retrieve. Defaults to NULL.
							``
							Example:
							{
								"operator": "not",
								"arg0": {
									"operator": "stringEquals",
									"arg0": "type",
									"arg1": "vlan"
								}
							}
							``
			:returns:		True if at least one configuration object exists,
							otherwise False.
			"""
			assertTrue(isinstance(filter, openmediavault.config.DataBaseFilter),
				"filter must be an openmediavault.config.DataBaseFilter object")

		def is_referenced(self, obj):
			"""
			Check if the specified object is referenced.
			:param obj:	The configuration object to use.
			:returns:	True if the object is referenced, otherwise False.
			"""
			assertTrue(isinstance(obj, openmediavault.config.Object),
				"obj must be an openmediavault.config.Object object")

		def is_unique(self, obj, property):
			"""
			Check if a configuration object with the value of the specified
			property is unique.
			:param obj:			The configuration object to use.
			:param property:	The name of the data model property.
			:returns:			True if no configuration object with the same
								property value exists, otherwise False.
			"""
			assertTrue(isinstance(obj, openmediavault.config.Object),
				"obj must be an openmediavault.config.Object object")

		def is_unique_by_filter(self, obj, filter):
			"""
			Check if a configuration object with the specified constraints
			is unique.
			:param obj:		The configuration object to use.
			:param filter:	A filter specifying constraints on the objects
							to retrieve.
							``
							Example:
							{
								"operator": "stringEquals",
								"arg0": "sharename",
								"arg1": "Movies"
							}
							``
			:returns:		True if no configuration object with the same
							property values (specified by the filter) exists,
							otherwise False.
			"""
			assertTrue(isinstance(obj, openmediavault.config.Object),
				"obj must be an openmediavault.config.Object object")
			assertTrue(isinstance(filter, openmediavault.config.DataBaseFilter),
				"filter must be an openmediavault.config.DataBaseFilter object")

class DataBaseFilter(openmediavault.collections.DotDict):
	pass
