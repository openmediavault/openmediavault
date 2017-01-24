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

import openmediavault as omv

class Datamodel(omv.datamodel.Datamodel):
	def __init__(self, model):
		"""
		:param model: The data model as JSON object or string.
		"""
		# Validate the model.
		self._validate_model(model)
		# Call the parent constructor.
		super().__init__(self, model)

	def _validate_model(self, model):
		"""
		Check the given data model.
		:param model: The data model as JSON object or string.
		:returns: None
		"""
		schema = omv.json.Schema({
			"type":"object",
			"properties":{
				"type":{"type":"string","enum":["config"],"required":true},
				"id":{"type":"string","required":true},
				"alias":{"type":"string","required":false},
				"persistent":{"type":"boolean","required":false},
				"title":{"type":"string","required":false},
				"description":{"type":"string","required":false},
				"notificationid":{"type":"string","required":false},
				"properties":{"type":"any","required":true},
				"queryinfo":{
					"type":"object",
					"required":false,
					"properties":{
						"xpath":{"type":"string","required":true},
						"iterable":{"type":"boolean","required":true},
						"idproperty":{"type":"string","required":false},
						"refproperty":{"type":"string","required":false}
					}
				}
			}
		})
		try:
			schema.validate(model)
		except omv.json.SchemaValidationException as e:
			raise Exception(
				"The data model of the configuration object is invalid: %s" %
				str(e))

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
	def properties(self):
		"""
		Get the data model properties.
		:returns:	Returns the data model properties as JSON object.
		"""
		return self.model['properties']

	@property
	def properties_schema(self):
		"""
		Get the JSON schema of the data model properties.
		:returns:	Returns the JSON schema of the data model properties
					as JSON object.
		"""
		schema = omv.json.Schema({
			"type": "object",
			"properties": self.properties
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
		:param data: The JSON data to validate.
		:returns: None.
		:raises omv.json.SchemaException:
		:raises omv.json.SchemaValidationException:
		"""
		self.properties_schema.validate(data)

	def property_exists(self, path):
		"""
		Check if the specified property exists.
		:param path: The path of the property, e.g. "aaa.bbb.ccc".
		:returns: True if the property exists, otherwise False.
		"""
		exists = True
		try:
			self.properties_schema.get_dict_by_path(path)
		except omv.json.SchemaException:
			exists = False
		except omv.json.SchemaPathException:
			exists = False
		return exists

if __name__ == "__main__":
	import unittest

	class DatamodelTestCase(unittest.TestCase):
		def test_construction(self):
			datamodel = Datamodel({
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
					}
				}
			})

	unittest.main()
