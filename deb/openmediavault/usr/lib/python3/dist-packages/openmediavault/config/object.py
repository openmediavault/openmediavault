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
__all__ = [ "Object" ]

import openmediavault
import openmediavault.string
import openmediavault.config.datamodel
import openmediavault.collections
import openmediavault.exceptions
import openmediavault.json.schema

class Object(object):
	def __init__(self, id):
		"""
		:param id:	The data model identifier, e.g. 'conf.service.ftp.share'.
		"""
		# Set the data model.
		self._model = openmediavault.config.Datamodel(id)
		# Set the default values.
		self.reset_all()

	@property
	def model(self):
		"""
		Get the data model of the configuration object.
		:returns:	The openmediavault.config.Datamodel object of the
					data model.
		"""
		return self._model

	@property
	def properties(self):
		"""
		Get the configuration object properties.
		:returns:	Returns the properties of the configuration object as
					openmediavault.collections.DotDict dictionary.
		"""
		return self._properties

	@property
	def id(self):
		"""
		Get the object identifier.
		:returns: Returns the object identifier, e.g. an UUID.
		"""
		if not (self.model.is_iterable and self.model.is_identifiable):
			raise Exception("The configuration object '%s' is not iterable " \
				"and identifiable." % self.model.id)
		return self.get(self.model.idproperty)

	@property
	def is_new(self):
		"""
		Check if the configuration object is new. Use this method only if
		the configuration object has an 'uuid' property.
		:returns:	Returns True if the configuration object is identified as
					new, otherwise False.
		"""
		if not openmediavault.string.is_uuid4(self.id):
			return False
		return self.id == openmediavault.getenv("OMV_CONFIGOBJECT_NEW_UUID")

	def get_defaults(self):
		"""
		Get the default values as defined in the data model.
		:returns:	Returns the default values as defined in the data model
					as openmediavault.collections.DotDict dictionary.
		"""
		def _walk_object(path, schema, defaults=None):
			if not "type" in schema:
				raise openmediavault.json.SchemaException(
					"No 'type' attribute defined at '%s'." % path)
			if "array" == schema['type']:
				raise Exception("Not supported yet.")
			elif "object" == schema['type']:
				if not "properties" in schema:
					raise openmediavault.json.SchemaException(
						"No 'properties' attribute defined at '%s'." % path)
				for prop_name, prop_schema in schema['properties'].items():
					# Build the property path. Take care that a valid path
					# is generated. To ensure this, empty parts are removed.
					prop_path = ".".join([ x for x in [ path,
						prop_name ] if x ])
					# Process the property node.
					_walk_object(prop_path, prop_schema, defaults)
			else:
				defaults[path] = self.model.property_get_default(path)

		defaults = openmediavault.collections.DotDict();
		_walk_object(None, self.model.schema.get(), defaults)
		return defaults

	def reset_all(self):
		"""
		Reset all properties to their default values.
		"""
		self._properties = self.get_defaults()

	def exists(self, name):
		"""
		Check if the specified property exists.
		:param name:	The name of the property in dot notation, e.g. 'a.b.c'.
		:returns:		Returns True if the property exists, otherwise False.
		"""
		return self.model.property_exists(name)

	def assert_exists(self, name):
		"""
		Assert that the specified property exists.
		:param name:	The name of the property in dot notation, e.g. 'a.b.c'.
		:raises openmediavault.exceptions.AssertException:
		"""
		if not self.exists(name):
			raise openmediavault.exceptions.AssertException(
				"The property '%s' does not exist in the model '%s'." %
				(name, self.model.id))

	def get(self, name):
		"""
		Get a property.
		:param name:	The name of the property in dot notation, e.g. 'a.b.c'.
		:returns:		The property value.
		"""
		self.assert_exists(name)
		return self.properties[name]

	def set(self, name, value, validate=True):
		"""
		Set a property.
		:param name:		The name of the property in dot notation,
							e.g. 'a.b.c'.
		:param value:		The value of the property.
		:param validate:	Set to False to do not validate the value.
							Defaults to True.
		"""
		self.assert_exists(name)
		if validate:
			self.model.property_validate(name, value)
		# Convert the value into the proper type.
		value = self.model.property_convert(name, value)
		# Set the property value in the dictionary.
		self.properties[name] = value

	def reset(self, name):
		"""
		Reset a property to its default value as defined in the data model.
		:param name:	The name of the property in dot notation, e.g. 'a.b.c'.
		"""
		defaults = self.get_defaults()
		self.set(name, defaults[name])

	def is_empty(self, name):
		"""
		Determine whether a property value is empty.
		:param name:	The name of the property in dot notation, e.g. 'a.b.c'.
		:returns:		Returns False if the property exists and has a
						non-empty, non-zero value, otherwise returns True.
						If the property does not exist an exception is thrown.
		"""
		value = self.get(name);
		return not bool(value and value.strip())
