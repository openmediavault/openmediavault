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

import openmediavault.config
import openmediavault.collections
import openmediavault.json

class Object(object):
	def __init__(self, id):
		"""
		:param id: The data model identifier, e.g. 'conf.service.ftp.share'.
		"""
		# Set the data model.
		self._model = openmediavault.config.Datamodel(id)
		# Set the default values.
		self._properties = self.get_defaults()

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
		:returns: The properties as Python dictionary.
		"""
		return self._properties

	def get_defaults(self):
		"""
		Get the default values as defined in the data model.
		:returns:	Returns the default values as defined in the data
					model as dictionary.
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
				defaults[path] = self.model.get_property_default(path)

		defaults = openmediavault.collections.DotDict();
		_walk_object(None, self.model.schema.get(), defaults)
		return defaults

if __name__ == "__main__":
	import unittest

	class ConfigObjectTestCase(unittest.TestCase):
		def test_constructor(self):
			conf_obj = Object("conf.service.ftp.share")

		def test_get_defaults(self):
			conf_obj = Object("conf.service.ftp.share")
			defaults = conf_obj.get_defaults()
			self.assertEqual(defaults, {
				'comment': '',
				'enable': False,
				'uuid': 'fa4b1c66-ef79-11e5-87a0-0002b3a176b4',
				'sharedfolderref': '',
				'extraoptions': '' })

	unittest.main()
