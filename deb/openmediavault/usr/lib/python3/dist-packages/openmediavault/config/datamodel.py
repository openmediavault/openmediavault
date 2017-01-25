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
					as openmediavault.json.Schema object.
		"""
		schema = openmediavault.json.Schema({
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
		:param data: The JSON data to validate.
		:returns: None.
		:raises openmediavault.json.SchemaException:
		:raises openmediavault.json.SchemaValidationException:
		"""
		self.schema.validate(data)

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

	unittest.main()
