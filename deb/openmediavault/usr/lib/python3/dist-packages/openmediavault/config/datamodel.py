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
__all__ = [ "Datamodel" ]

import os
import openmediavault
import openmediavault.datamodel
import openmediavault.json

class Datamodel(openmediavault.datamodel.Datamodel):
	def __init__(self, id):
		"""
		:param id: The data model identifier, e.g. 'conf.service.ftp.share'.
		"""
		# Load the data model.
		if isinstance(id, dict):
			model = id
		else:
			model = self._load(id)
		# Validate the data model.
		self._validate(model)
		# Call the parent constructor.
		super().__init__(model)

	def _load(self, id):
		"""
		Load the specified data model from file system.
		:param id: The data model identifier, e.g. 'conf.service.ftp.share'.
		:returns: The JSON object of the data model.
		:raises FileNotFoundError:
		"""
		# Load the file content.
		datamodels_dir = openmediavault.getenv("OMV_DATAMODELS_DIR",
			"/usr/share/openmediavault/datamodels");
		with open(os.path.join(datamodels_dir, "%s.json" % id)) as f:
			content = f.read()
		return content

	def _validate(self, model):
		"""
		Validate the data model.
		:param model: The data model as JSON object or string.
		:returns: None.
		"""
		schema = openmediavault.json.Schema({
			"type":"object",
			"properties":{
				"type":{"type":"string","enum":["config"],"required":True},
				"id":{"type":"string","required":True},
				"alias":{"type":"string","required":False},
				"persistent":{"type":"boolean","required":False},
				"title":{"type":"string","required":False},
				"description":{"type":"string","required":False},
				"notificationid":{"type":"string","required":False},
				"properties":{"type":"any","required":True},
				"queryinfo":{
					"type":"object",
					"required":False,
					"properties":{
						"xpath":{"type":"string","required":True},
						"iterable":{"type":"boolean","required":True},
						"idproperty":{"type":"string","required":False},
						"refproperty":{"type":"string","required":False}
					}
				}
			}
		})
		try:
			schema.validate(model)
		except openmediavault.json.SchemaValidationException as e:
			raise Exception("The data model is invalid: %s" % str(e))

	def is_iterable(self):
		if not "idproperty" in self.queryinfo:
			return False
		if not "iterable" in self.queryinfo:
			return False
		return self.queryinfo['iterable']

	def is_referenceable(self):
		if not "idproperty" in self.queryinfo:
			return False
		if not "refproperty" in self.queryinfo:
			return False
		return self.queryinfo['refproperty']

	def is_identifiable(self):
		return "idproperty" in self.queryinfo

	def is_persistent(self):
		"""
		Tests whether the data model instance is persistent.
		:returns:	Returns True if the data model instance is persistent.
					If the property is not available in the data model
					definition, then True is assumed.
		"""
		if not "persistent" in self.model:
			return True
		return self.model['persistent']

	@property
	def schema(self):
		"""
		Get the JSON schema of the data model properties.
		:returns:	Returns the JSON schema of the data model properties
					as openmediavault.datamodel.Schema object.
		"""
		# Build a valid JSON schema.
		schema = openmediavault.datamodel.Schema({
			"type": "object",
			"properties": self.model['properties']
		})
		return schema

	@property
	def queryinfo(self):
		"""
		Get the query information.
		:returns: Returns a dictionary with the query information.
		"""
		return self.model['queryinfo']

	@property
	def idproperty(self):
		return self.queryinfo['idproperty']

	@property
	def refproperty(self):
		return self.queryinfo['refproperty']

	@property
	def notificationid(self):
		"""
		Get the notification identifier. It is auto-generated based on the
		data model property named 'id': 'org.openmediavault.' + <id>
		The identifier can be overwritten using the property that is named
		'notificationid'.
 		:returns:	The notification identifier string, e.g.
					'org.openmediavault.x.y.z'.
		"""
		if not "notificationid" in self.model:
			return "org.openmediavault.%s" % self.id
		return self.model['notificationid']

	def validate(self, data):
		"""
		Validate the specified data against the data model.
		:param data:	The JSON data to validate.
		:returns:		None.
		:raises openmediavault.json.SchemaException:
		:raises openmediavault.json.SchemaValidationException:
		"""
		self.schema.validate(value, name)

	def property_get_default(self, path):
		"""
		Get the default value of the specified property as defined in
		the data model.
		:param path:	The path of the property, e.g. "aaa.bbb.ccc".
		:returns:		The default value as specified in the data model
						schema or by the type of the property.
		"""
		prop_schema = self.schema.get_by_path("properties.%s" % path)
		if "default" in prop_schema:
			result = prop_schema['default']
		else:
			if isinstance(prop_schema['type'], list):
				raise openmediavault.json.SchemaException(
					"The attribute 'type' must not be an array at '%s'." %
					path)
			type = "any"
			if "type" in prop_schema:
				type = prop_schema['type']
			if "array" == type:
				result = []
			elif "object" == type:
				result = {}
			elif "boolean" == type:
				result = False
			elif "integer" == type:
				result = 0
			elif type in [ "number", "double", "float" ]:
				result = 0.0
			elif "string" == type:
				result = ""
				if "format" in prop_schema:
					if "uuidv4" == prop_schema['format']:
						if self.is_identifiable and self.idproperty == path:
							result = openmediavault.getenv(
								"OMV_CONFIGOBJECT_NEW_UUID")
#			elif type in [ "any", "null", "float" ]:
#				result = None
			else:
				result = None
		return result

	def property_exists(self, name):
		"""
		Check if the specified property exists.
		:param name:	The path of the property, e.g. "aaa.bbb.ccc".
		:returns:		Returns True if the property exists, otherwise False.
		"""
		result = True
		try:
			self.schema.get_by_path("properties.%s" % name)
		except openmediavault.json.SchemaPathException:
			result = False
		return result

	def property_validate(self, name, value):
		"""
		Validate the specified property against the data model.
		:param name:	The JSON path of the entity to validate, e.g.
						'aa.bb.cc', defaults to an empty string. Use an
						empty value for the root.
		:param value:	The value to validate.
		:returns: 		None.
		:raises openmediavault.json.SchemaException:
		:raises openmediavault.json.SchemaValidationException:
		"""
		self.schema.validate(value, "properties.%s" % name)

	def property_convert(self, name, value):
		"""
		Convert the given value into the type which is declared in the
		property schema at the specified path. The original value is returned
		when the property type can not be processed, e.g. 'any', 'array',
		'object' or 'null'.
		:param name:	The path of the property, e.g. "aaa.bbb.ccc".
		:param value:	The value to convert.
		:returns:		The converted value.
		"""
		prop_schema = self.schema.get_by_path("properties.%s" % name)
		if isinstance(prop_schema['type'], list):
			raise openmediavault.json.SchemaException(
				"The attribute 'type' must not be an array at '%s'." %
				path)
		if "boolean" == prop_schema['type']:
			result = bool(value)
		elif "integer" == prop_schema['type']:
			result = int(value)
		elif prop_schema['type'] in [ "number", "double", "float" ]:
			result = float(value)
		elif "string" == prop_schema['type']:
			result = str(value)
		else:
			result = value
		return result

if __name__ == "__main__":
	import unittest

	class DatamodelTestCase(unittest.TestCase):
		def _get_model(self):
			return Datamodel({
				"type": "config",
				"id": "conf.system.apt.distribution",
				"queryinfo": {
					"xpath": "//system/apt/distribution",
					"iterable": False
				},
				"properties": {
					"proposed": {
						"type": "boolean",
						"default": False
					},
					"partner": {
						"type": "boolean",
						"default": False
					},
					"integer": {
						"type": "integer",
						"default": 10
					}
				}
			})

		def test_schema(self):
			datamodel = self._get_model()
			datamodel.schema

		def test_queryinfo(self):
			datamodel = self._get_model()
			datamodel.queryinfo

		def test_notificationid(self):
			datamodel = self._get_model()
			datamodel.notificationid

		def test_property_get_default(self):
			datamodel = self._get_model()
			default = datamodel.property_get_default("partner")
			self.assertEqual(default, False)

		def test_property_exists(self):
			datamodel = self._get_model()
			default = datamodel.property_exists("proposed")
			self.assertEqual(default, True)

		def test_property_convert(self):
			datamodel = self._get_model()
			value = datamodel.property_convert("integer", "20")
			self.assertEqual(value, 20)

	unittest.main()
