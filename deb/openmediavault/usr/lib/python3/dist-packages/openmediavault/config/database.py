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
	"DatabaseFilter",
	"DatabaseGetQuery",
	"DatabaseGetByFilterQuery",
	"DatabaseSetQuery",
	"DatabaseDeleteQuery",
	"DatabaseIsReferencedQuery"
]

import abc
import os
import lxml.etree
import openmediavault.collections
import openmediavault.config.datamodel
import openmediavault.config.object

class DatabaseException(Exception):
	pass

class DatabaseFilter(openmediavault.collections.DotDict):
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
		request = openmediavault.config.DatabaseGetQuery(id, uuid)
		request.execute()
		return request.response

	def get_by_filter(self, id, filter, **kwargs):
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
		:param min_result:	The minimum number of objects that are expected.
		:param max_result:	The maximum number of objects that are expected.
		:returns:			An array containing the requested configuration
							objects. If *max_result* is set to 1, then the
							first found object is returned.
							In this case the method does not return a list
							of configuration objects.
		"""
		request = openmediavault.config.DatabaseGetByFilterQuery(id, filter)
		request.execute()
		if "min_result" in kwargs:
			if len(request.response) < kwargs.get("min_result"):
				raise DatabaseException("The query does not return the " \
					"minimum number of %d object(s)." %
					kwargs.get("min_result"))
		if "max_result" in kwargs:
			if len(request.response) > kwargs.get("max_result"):
				raise DatabaseException("The query returns more than the " \
					"maximum number of %d object(s)." %
					kwargs.get("max_result"))
		return request.response

	def exists(self, id, filter=None):
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
		request = openmediavault.config.DatabaseGetByFilterQuery(id, filter)
		request.execute()
		return 0 < len(request.response)

	def is_referenced(self, obj):
		"""
		Check if the specified object is referenced.
		:param obj:	The configuration object to use.
		:returns:	True if the object is referenced, otherwise False.
		"""
		request = openmediavault.config.DatabaseIsReferencedQuery(obj)
		request.execute()
		return request.response

	def is_unique(self, obj, property):
		"""
		Check if a configuration object with the value of the specified
		property is unique.
		:param obj:			The configuration object to use.
		:param property:	The name of the data model property.
		:returns:			True if no configuration object with the same
							property value exists, otherwise False.
		"""
		assert(isinstance(obj, openmediavault.config.Object))
		return self.is_unique_by_filter(obj, DatabaseFilter({
				'operator': 'stringEquals',
				'arg0': property,
				'arg1': obj.get(property)
			}))

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
		assert(isinstance(obj, openmediavault.config.Object))
		assert(isinstance(filter, DatabaseFilter))
		# If the object is iterable and not new, then we need to modify the
		# filter to do not find the configuration object itself.
		if not obj.is_new and obj.model.is_iterable:
			filter = DatabaseFilter({
					'operator': 'and',
					'arg0': {
						'operator': 'stringNotEquals',
						'arg0': obj.model.idproperty,
						'arg1': obj.get(obj.model.idproperty)
					},
					'arg1': filter
				})
		request = openmediavault.config.DatabaseGetByFilterQuery(
			obj.model.id, filter)
		request.execute()
		return 0 == len(request.response)

class DatabaseQuery(metaclass=abc.ABCMeta):
	def __init__(self, id):
		"""
		:param id: The data model identifier, e.g. 'conf.service.ftp.share'.
		"""
		# Create the data model object.
		self._model = openmediavault.config.Datamodel(id)
		# Get the property names that must be handled as lists/arrays.
		self._force_list = self._get_array_properties()
		# Set the default response value.
		self._response = None
		# The XML tree.
		self._root_element = None

	@property
	def model(self):
		"""
		Get the data model as JSON object.
		:returns: Returns the data model as JSON object.
		"""
		return self._model

	@property
	def response(self):
		"""
		Get the response of the database query.
		:returns: Returns the response of the database query.
		"""
		return self._response

	@abc.abstractproperty
	def xpath(self):
		"""
		Get the XPath string used to execute the database query.
		:returns: The XPath string for this database query.
		"""

	@abc.abstractmethod
	def execute(self):
		"""
		Execute the database query.
		"""

	def _get_array_properties(self):
		"""
		Parse the data model and get the properties that must be handled
		as arrays/lists.
		"""
		def callback(model, name, path, schema, user_data):
			if "array" == schema['type']:
				user_data.append(name)

		names = []
		self.model.walk_schema("", callback, names)
		return names

	def _load(self):
		"""
		Helper function to load the XML configuration file.
		"""
		# Get the path of the configuration file.
		config_file = openmediavault.getenv("OMV_CONFIG_FILE")
		# Make sure the file exists, otherwise throw an exception.
		os.stat(config_file)
		# Parse the XML configuration file.
		self._root_element = lxml.etree.parse(config_file)

	def _save(self):
		# Get the path of the configuration file.
		config_file = openmediavault.getenv("OMV_CONFIG_FILE")
		# Save the XML configuration file.
		with open(config_file, "w") as f:
			f.write(lxml.etree.tostring(self._root_element,
				pretty_print=True, xml_declaration=True,
				encoding="UTF-8"))

	def _get_root_element(self):
		"""
		Get the root element of the configuration file XML tree.
		"""
		if self._root_element is None:
			self._load()
		return self._root_element

	def _find_all_elements(self):
		"""
		Helper method to execute the XML query.
		:returns:	Returns a list of XML elements that match the specified
					XPath query.
		"""
		root_element = self._get_root_element()
		# Execute the XPath query and return the matching elements.
		return root_element.xpath(self.xpath)

	def _element_to_dict(self, element):
		"""
		Helper method to convert a XML element to a Python dictionary.
		:param element:	The XML element to convert.
		:returns:		Returns a Python dictionary.
		"""
		assert(lxml.etree.iselement(element))
		result = {}
		for child_element in list(element):
			if list(child_element):
				value = self._element_to_dict(child_element)
			else:
				value = child_element.text
			tag = child_element.tag
			if isinstance(self._force_list, list) and tag in self._force_list:
				try:
					# Add to existing list.
					result[tag].append(value)
				except AttributeError:
					# Convert existing entry into a list.
					result[tag] = [ result[tag], value ]
			else:
				result[tag] = value
		return result

	def _elements_to_object(self, elements):
		"""
		Convert XML elements to openmediavault.config.Object objects.
		:param elements:	A list of XML elements.
		:returns:			Returns a list of openmediavault.config.Object
							objects if the data model of the configuration
							object defines it as iterable. The list may be
							empty.
							A single openmediavault.config.Object object is
							returned for non-iterable configuration objects.
							If no element is given, then None is returned.
		"""
		assert(isinstance(elements, list))
		if self.model.is_iterable:
			result = []
			for element in elements:
				obj = openmediavault.config.Object(self.model.id)
				obj.set_dict(self._element_to_dict(element), False)
				result.append(obj)
		else:
			result = None
			if 0 < len(elements):
				result = openmediavault.config.Object(self.model.id)
				result.set_dict(self._element_to_dict(elements[0]), False)
		return result

	def _dict_to_element(self, dictionary):
		"""
		Helper method to convert a dictionary to a XML element.
		"""
		pass

	def _build_predicate(self, filter):
		"""
		Helper method to build the predicate for the specified filter.
		Supported operators:
		.-------------------------------------------------.
		| operator         | arg0          | arg1         |
		|------------------|---------------|--------------|
		| and              | assoc. array  | assoc. array |
		| or               | assoc. array  | assoc. array |
		| equals           | property name | value        |
		| notEquals        | property name | value        |
		| enum             | property name | array        |
		| stringEquals     | property name | value        |
		| stringNotEquals  | property name | value        |
		| stringContains   | property name | value        |
		| stringStartsWith | property name | value        |
		| stringEnum       | property name | array        |
		| not              | assoc. array  |              |
		| less             | property name | value        |
		| greater          | property name | value        |
		| lessEqual        | property name | value        |
		| greaterEqual     | property name | value        |
		'-------------------------------------------------'
		Example 1:
		[type='bond' and devicename='bond0']
		The filter for the above predicate:
		[
			"operator": "and",
			"arg0": [
				"operator" => "stringEquals",
				"arg0" => "type",
				"arg1" => "bond"
			],
			"arg1": [
				"operator" => "stringEquals",
				"arg0" => "devicename",
				"arg1" => "bond0"
			]
		]
		Example 2:
		[type='bond' and contains(slaves,'eth0')]
		The filter for the above predicate:
		[
			"operator": "and",
			"arg0": [
				"operator" => "stringEquals",
				"arg0" => "type",
				"arg1" => "bond"
			],
			"arg1": [
				"operator" => "stringContains",
				"arg0" => "slaves",
				"arg1" => "eth0"
			]
		]
		Example 3:
		[not type='vlan']
		The filter for the above predicate:
		[
			"operator": "not",
			"arg0": [
				"operator" => "stringEquals",
				"arg0" => "type",
				"arg1" => "vlan"
			]
		]
		Example 4:
		[
			"operator" => "and",
			"arg0" => [
				"operator" => "stringNotEquals",
				"arg0" => "uuid",
				"arg1" => $object->get("uuid")
			],
			"arg1" => [
				"operator" => "and",
				"arg0" => [
					"operator" => "stringEquals",
					"arg0" => "mntentref",
					"arg1" => $object->get("mntentref")
				],
				"arg1" => [
					"operator" => "stringEquals",
					"arg0" => "reldirpath",
					"arg1" => $object->get("reldirpath")
				]
			]
		]
		"""
		assert(isinstance(filter, DatabaseFilter))
		if not "operator" in filter:
			raise KeyError("Invalid filter, the field 'operator' is missing.")
		result = ""
		if filter['operator'] in [ 'and', 'or' ]:
			result = "(%s %s %s)" % (
				self._build_predicate(DatabaseFilter(filter['arg0'])),
				filter['operator'],
				self._build_predicate(DatabaseFilter(filter['arg1'])))
		elif filter['operator'] in [ '=', 'equals' ]:
			result = "%s=%s" % (filter['arg0'], filter['arg1'])
		elif filter['operator'] in [ '!=', 'notEquals' ]:
			result = "%s!=%s" % (filter['arg0'], filter['arg1'])
		elif "enum" == filter['operator']:
			parts = []
			for enumv in filter['arg1']:
				parts.append("%s=%s" % (filter['arg0'], enumv))
			result = "(%s)" % " or ".join(parts)
		elif filter['operator'] in [ '==', 'stringEquals' ]:
			result = "%s='%s'" % (filter['arg0'], filter['arg1'])
		elif filter['operator'] in [ '!==', '!=', 'stringNotEquals' ]:
			result = "%s!='%s'" % (filter['arg0'], filter['arg1'])
		elif "stringContains" == filter['operator']:
			result = "contains(%s,'%s')" % (filter['arg0'],
				filter['arg1'])
		elif "stringStartsWith" == filter['operator']:
			result = "starts-with(%s,'%s')" % (filter['arg0'],
				filter['arg1'])
		elif "stringEnum" == filter['operator']:
			parts = [];
			for enumv in filter['arg1']:
				parts.append("%s='%s'" % (filter['arg0'], enumv))
			result = "(%s)" % " or ".join(parts)
		elif filter['operator'] in [ '!', 'not' ]:
			result = "not(%s)" % DatabaseGetByFilterQuery(self.model.id,
				DatabaseFilter(filter['arg0'])).xpath
		elif filter['operator'] in [ '<', 'less' ]:
			result = "%s<%s" % (filter['arg0'], filter['arg1'])
		elif filter['operator'] in [ '>', 'greater' ]:
			result = "%s>%s" % (filter['arg0'], filter['arg1'])
		elif filter['operator'] in [ '<=', 'lessEqual' ]:
			result = "%s<=%s" % (filter['arg0'], filter['arg1'])
		elif filter['operator'] in [ '>=', 'greaterEqual' ]:
			result = "%s>=%s" % (filter['arg0'], filter['arg1'])
		else:
			raise ValueError("The operator '%s' is not defined." %
				filter['operator'])
		return result

class DatabaseGetByFilterQuery(DatabaseQuery):
	def __init__(self, id, filter):
		assert(isinstance(filter, DatabaseFilter))
		self._filter = filter
		super().__init__(id)

	@property
	def filter(self):
		return self._filter

	@property
	def xpath(self):
		if self.filter:
			return "%s[%s]" % (self.model.queryinfo['xpath'],
				self._build_predicate(self.filter))
		return self.model.queryinfo['xpath']

	def execute(self):
		elements = self._find_all_elements()
		self._response = self._elements_to_object(elements)

class DatabaseGetQuery(DatabaseQuery):
	def __init__(self, id, identifier=None):
		super().__init__(id)
		self._identifier = identifier

	@property
	def identifier(self):
		return self._identifier

	@property
	def xpath(self):
		if self.model.is_iterable and self.identifier:
			return DatabaseGetByFilterQuery(self.model.id,
				DatabaseFilter({
					'operator': 'stringEquals',
					'arg0': self.model.idproperty,
					'arg1': self.identifier
				})).xpath
		return self.model.queryinfo['xpath']

	def execute(self):
		elements = self._find_all_elements()
		self._response = self._elements_to_object(elements)
		# If the object is iterable and if there is an identifier,
		# then only one object must be returned.
		if self.model.is_iterable and self.identifier:
			try:
				self._response = self._response[0]
			except KeyError:
				self._response = None

class DatabaseIsReferencedQuery(DatabaseQuery):
	def __init__(self, obj):
		assert(isinstance(obj, openmediavault.config.Object))
		self._obj = obj
		super().__init__(obj.model.id)

	@property
	def object(self):
		return self._obj

	@property
	def xpath(self):
		return "//%s[%s]" % (self.model.refproperty,
			self._build_predicate(DatabaseFilter({
				'operator': 'stringContains',
				'arg0': '.',
				'arg1': self.object.get(self.model.idproperty)
			})))

	def execute(self):
		elements = self._find_all_elements()
		self._response = 0 < len(elements)

class DatabaseSetQuery(DatabaseQuery):
	def __init__(self, id, obj):
		assert(isinstance(obj, openmediavault.config.Object))
		self._obj = obj
		super().__init__(id)

	@property
	def object(self):
		return self._obj

	@property
	def xpath(self):
		if self.model.is_iterable:
			if self.object.is_new:
				# Update the element with the specified identifier.
				return DatabaseGetByFilterQuery(self.model.id,
					DatabaseFilter({
						'operator': 'stringEquals',
						'arg0': self.model.idproperty,
						'arg1': self.object.get(self.model.idproperty)
					})).xpath
			else:
				# Insert a new element.
				#$parts = explode("/", $xpath);
				#$elementName = array_pop($parts);
				#$xpath = substr($xpath, 0, strrpos($xpath, $elementName) - 1);
				raise NotImplementedError()

		return self.model.queryinfo['xpath']

	def execute(self):
		# ToDo...
		pass

class DatabaseDeleteQuery(DatabaseQuery):
	def __init__(self, obj):
		assert(isinstance(obj, openmediavault.config.Object))
		self._obj = obj
		super().__init__(obj.model.id)

	@property
	def object(self):
		return self._obj

	@property
	def xpath(self):
		if self.model.is_iterable:
			return DatabaseGetByFilterQuery(self.model.id,
				DatabaseFilter({
					'operator': 'stringEquals',
					'arg0': self.model.idproperty,
					'arg1': self.object.get(self.model.idproperty)
				})).xpath
		return self.model.queryinfo['xpath']

	def execute(self):
		elements = self._find_all_elements()
		for element in elements:
			parent = element.getparent()
			if parent is None:
				continue
			parent.remove(element)
		self._save()
