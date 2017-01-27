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
	"Schema",
	"SchemaException",
	"SchemaPathException",
	"SchemaValidationException"
]

import decimal
import json
import jsonpath_rw
import re
import urllib.parse
import socket
import openmediavault.string

class SchemaException(Exception):
	pass

class SchemaPathException(Exception):
	def __init__(self, path):
		self.path = path
		super().__init__(self, "The path '%s' is invalid." % path)

class SchemaValidationException(Exception):
	def __init__(self, path, message):
		self.path = path
		super().__init__(self, "%s: %s" % (path, message))

class Schema(object):
	def __init__(self, schema):
		"""
		:param schema: A JSON object (Python dictionary) describing the schema.
		"""
		self._schema = schema

	@property
	def schema(self):
		"""
		Get the JSON schema as Python dictionary.
		:returns: Returns the JSON schema as dictionary.
		"""
		return self.get()

	def get(self):
		"""
		Get the JSON schema as Python dictionary.
		:returns: Returns the JSON schema as dictionary.
		"""
		if isinstance(self._schema, str):
			self._schema = json.loads(self._schema)
		if not isinstance(self._schema, dict):
			raise TypeError("Expected dictionary.")
		return self._schema

	def get_by_path(self, path):
		"""
		Get the JSON schema for the given path.
		:param path:	The path of the requested attribute, e.g. "a.b.c".
		:returns:		The JSON schema as Python dictionary describing the
						requested attribute.
		"""
		if not path:
			return self.schema
		jsonpath_expr = jsonpath_rw.parse(path)
		matches = [ match.value for match in jsonpath_expr.find(self.schema) ]
		if not matches:
			raise SchemaPathException(path)
		return matches[0]

	def validate(self, value, name=""):
		"""
		Validate the given value.
		:param value:	The value to validate.
		:param name:	The JSON path of the entity to validate, e.g.
						'aa.bb.cc', defaults to an empty string. Use an
						empty value for the root.
		:returns: None
		"""
		# Convert all arrays (indexed and associative) to JSON string to
		# ensure that they are converted into the required dictionary format.
		if isinstance(value, (dict, list)):
			value = json.dumps(value)
		# Convert JSON string to dictionary.
		if openmediavault.string.is_json(value):
			value = json.loads(value)
		schema = self.get_by_path(name);
		self._validate_type(value, schema, name)

	def _validate_type(self, value, schema, name):
		"""
		:returns: None
		"""
		types = "any"
		if "type" in schema:
			types = schema['type']
		if not isinstance(types, list):
			types = [ types ];
		valid = False;
		last_exception = None;
		for typek, typev in enumerate(types):
			try:
				if "any" == typev:
					self._validate_any(value, schema, name)
					valid = True
				elif "array" == typev:
					self._validate_array(value, schema, name)
					valid = True
				elif "boolean" == typev:
					self._validate_boolean(value, schema, name)
					valid = True
				elif "object" == typev:
					self._validate_object(value, schema, name)
					valid = True
				elif "integer" == typev:
					self._validate_integer(value, schema, name)
					valid = True
				elif "number" == typev:
					self._validate_number(value, schema, name)
					valid = True
				elif "string" == typev:
					self._validate_string(value, schema, name)
					valid = True
				elif "null" == typev:
					self._validate_null(value, schema, name)
					valid = True
				else:
					raise SchemaException(
						"%s: The type '%s' is not defined." %
						(name, typev));
			except SchemaValidationException as e:
				# Continue with the next type but remember the exception.
				last_exception = e;
			# Break the foreach loop here because one of the defined types
			# is successfully validated.
			if True == valid:
				break;
		# If the validation is not successful, then trow the
		# last exception.
		if False == valid:
			raise last_exception;

	def _validate_any(self, value, schema, name):
		pass

	def _validate_boolean(self, value, schema, name):
		if not isinstance(value, bool):
			raise SchemaValidationException(name,
				"The value '%s' is not a boolean." %
				("NULL" if (value is None) else str(value)))

	def _validate_integer(self, value, schema, name):
		if not isinstance(value, int):
			raise SchemaValidationException(name,
				"The value '%s' is not an integer." %
				("NULL" if (value is None) else str(value)))
		self._check_minimum(value, schema, name)
		self._check_exclusive_minimum(value, schema, name)
		self._check_maximum(value, schema, name)
		self._check_exclusive_maximum(value, schema, name)
		self._check_enum(value, schema, name)
		self._check_one_of(value, schema, name)

	def _validate_number(self, value, schema, name):
		if not isinstance(value, (int, float, decimal.Decimal)):
			raise SchemaValidationException(name,
				"The value '%s' is not a number." %
				("NULL" if (value is None) else str(value)))
		self._check_minimum(value, schema, name)
		self._check_exclusive_minimum(value, schema, name)
		self._check_maximum(value, schema, name)
		self._check_exclusive_maximum(value, schema, name)
		self._check_enum(value, schema, name)
		self._check_one_of(value, schema, name)

	def _validate_string(self, value, schema, name):
		if not isinstance(value, str):
			raise SchemaValidationException(name,
				"The value '%s' is not a number." %
				"NULL" if (value is None) else str(value))
		self._check_pattern(value, schema, name)
		self._check_min_length(value, schema, name)
		self._check_max_length(value, schema, name)
		self._check_format(value, schema, name)
		self._check_enum(value, schema, name)
		self._check_one_of(value, schema, name)

	def _validate_array(self, value, schema, name):
		if not isinstance(value, list):
			raise SchemaValidationException(name,
				"The value '%s' is not an array." %
				"NULL" if (value is None) else str(value))
		self._check_min_items(value, schema, name)
		self._check_max_items(value, schema, name)
		self._check_items(value, schema, name)

	def _validate_object(self, value, schema, name):
		if not isinstance(value, dict):
			raise SchemaValidationException(name,
				"The value '%s' is not an object." %
				"NULL" if (value is None) else str(value))
		self._check_properties(value, schema, name)

	def _validate_null(self, value, schema, name):
		if not isinstance(value, type(None)):
			raise SchemaValidationException(name,
				"The value '%s' is not NULL." %
				str(value))

	def _check_minimum(self, value, schema, name):
		if not "minimum" in schema:
			return
		if schema['minimum'] > value:
			raise SchemaValidationException(name,
				"The value %s is less than %s." %
				(str(value), str(schema['minimum'])))

	def _check_maximum(self, value, schema, name):
		if not "maximum" in schema:
			return
		if schema['maximum'] < value:
			raise SchemaValidationException(name,
				"The value %s is bigger than %s." %
				(str(value), str(schema['maximum'])))

	def _check_exclusive_minimum(self, value, schema, name):
		if not "minimum" in schema:
			return
		if not ("exclusiveMinimum" in schema and
			(True == schema['exclusiveMinimum'])):
			return
		if schema['minimum'] == value:
			raise SchemaValidationException(name,
				"Invalid value %s, must be greater than %s." %
				(str(value), str(schema['minimum'])))

	def _check_exclusive_maximum(self, value, schema, name):
		if not "maximum" in schema:
			return
		if not ("exclusiveMaximum" in schema and
			(True == schema['exclusiveMaximum'])):
			return
		if schema['maximum'] == value:
			raise SchemaValidationException(name,
				"Invalid value %s, must be greater than %s." %
				(str(value), str(schema['maximum'])))

	def _check_min_length(self, value, schema, name):
		if not "minLength" in schema:
			return
		if schema['minLength'] > len(value):
			raise SchemaValidationException(name,
				"The value '%s' is too short, minimum length is %d." %
				(value, schema['minLength']))

	def _check_max_length(self, value, schema, name):
		if not "maxLength" in schema:
			return
		if schema['maxLength'] < len(value):
			raise SchemaValidationException(name,
				"The value '%s' is too long, maximum length is %d." %
				(value, schema['maxLength']))

	def _check_pattern(self, value, schema, name):
		if not "pattern" in schema:
			return
		if not re.match(schema['pattern'], value):
			raise SchemaValidationException(name,
				"The value '%s' doesn't match the pattern '%s'." %
				(value, schema['pattern']))

	def _check_format(self, value, schema, name):
		"""
		Check https://tools.ietf.org/html/draft-zyp-json-schema-03#section-5.23
		"""
		if not "format" in schema:
			return
		if schema['format'] in [ "date-time", "datetime" ]:
			if not re.match(r'/^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$/', value):
				raise SchemaValidationException(name,
					"The value '%s' does not match to ISO 8601." % value)
		elif "date" == schema['format']:
			if not re.match(r'/^\d{4}-\d{2}-\d{2}$/', value):
				raise SchemaValidationException(name,
					"The value '%s' does not match to YYYY-MM-DD." % value)
		elif "time" == schema['format']:
			if not re.match(r'/^\d{2}:\d{2}:\d{2}$/', value):
				raise SchemaValidationException(name,
					"The value '%s' does not match to hh:mm:ss." % value)
		elif schema['format'] in [ "host-name", "hostname" ]:
			if not re.match(r'/^[a-zA-Z]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9])' \
				'{0,1}$/', value):
				raise SchemaValidationException(name,
					"The value '%s' is not a valid hostname." % value)
		elif "regex" == schema['format']:
			try:
				re.compile(value)
			except:
				raise SchemaValidationException(name,
					"The value '%s' is not a valid regex." % value)
		elif "uri" == schema['format']:
			try:
				urllib.parse.urlparse(value);
			except:
				raise SchemaValidationException(name,
					"The value '%s' is not an URI." % value)
		elif "email" == schema['format']:
			if not "@" in value:
				raise SchemaValidationException(name,
					"The value '%s' is not an email." % value)
		elif schema['format'] in [ "ip-address", "ipv4" ]:
			try:
				socket.inet_aton(value)
			except socket.error:
				raise SchemaValidationException(name,
					"The value '%s' is not an IPv4 address." % value)
		elif "ipv6" == schema['format']:
			try:
				socket.inet_pton(socket.AF_INET6, value)
			except socket.error:
				raise SchemaValidationException(name,
					"The value '%s' is not an IPv6 address." % value)
		else:
			raise SchemaException(
				"%s: The format '%s' is not defined." %
				(name, schema['format']))

	def _check_enum(self, value, schema, name):
		if not "enum" in schema:
			return
		if not isinstance(schema['enum'], list):
			raise SchemaException(
				"The attribute 'enum' is not an array." % name)
		if not isinstance(value, list):
			value = [ value ]
		for valuev in value:
			if not valuev in schema['enum']:
				raise SchemaValidationException(name,
					"Invalid value '%s', allowed values are '%s'.",
					(valuev, ", ".join(schema['enum'])))

	def _check_min_items(self, value, schema, name):
		if not "minItems" in schema:
			return
		if schema['minItems'] > len(value):
			raise SchemaValidationException(name,
				"Not enough array items, minimum is %d." %
				schema['minItems'])

	def _check_max_items(self, value, schema, name):
		if not "maxItems" in schema:
			return
		if schema['maxItems'] < len(value):
			raise SchemaValidationException(name,
				"Too many array items, maximum is %d." %
				schema['maxItems'])

	def _check_properties(self, value, schema, name):
		if not "properties" in schema:
			raise SchemaException(name,
				"No 'properties' attribute defined.")
		if not isinstance(schema['properties'], dict):
			raise SchemaException(name,
				"The attribute 'properties' is not a dictionary.")
		for propk, propv in schema['properties'].items():
			# Build the new path. Strip empty parts.
			parts = [ name, propk ];
			parts = [ part for part in parts if part ]
			path = ".".join(parts);
			# Check if the 'required' attribute is set.
			if not propk in value:
				if ("required" in propv) and (True == propv['required']):
					raise SchemaValidationException(name,
						"Missing 'required' attribute '%s'." %
						path)
				continue
			self._validate_type(value[propk], propv, path)

	def _check_items(self, value, schema, name):
		if not "items" in schema:
			raise SchemaException(
				"No 'items' attribute defined." %
				name)
		if isinstance(schema['items'], dict):
			for itemk, itemv in value.items():
				path = "%s[%d]" % (name, itemk)
				valid = True;
				for item_schema in schema['items']:
					try:
						self._validate_type(itemv, item_schema, path);
					except SchemaValidationException:
						valid = False
				if not valid:
					types = map(lambda x: x['type'], schema['items'])
					raise SchemaValidationException(name,
						"Invalid 'items' value, must be one of the " \
						"following types '%s'." %
						", ".join(types))
		elif isinstance(schema['items'], list):
			for itemk, itemv in enumerate(value):
				path = "%s[%d]" % (name, itemk)
				self._validate_type(itemv, schema['items'], path);
		else:
			raise SchemaValidationException(name,
				"Invalid 'items' value.")

	def _check_one_of(self, value, schema, name):
		if not "oneOf" in schema:
			return
		if not isinstance(schema['oneOf'], list):
			raise SchemaException(
				"The 'oneOf' attribute is not an array." %
				name)
		valid = False
		for subSchemak, subSchemav in enumerate(schema['oneOf']):
			try:
				self._validate_type(value, subSchemav, name);
				# If validation succeeds for one of the schema, then we
				# can exit immediatelly.
				valid = True
			except SchemaValidationException:
				# Nothing to do here.
				pass
		if not valid:
			raise SchemaValidationException(name,
				"Failed to match exactly one schema.")

if __name__ == "__main__":
	import unittest

	class SchemaTestCase(unittest.TestCase):
		def _get_schema(self):
			return Schema({
				"type": "object",
				"properties": {
					"name": { "type": "string", "required": True },
					"price": { "type": "number", "minimum": 35, "maximum": 40 }
				}
			})

		def test_get(self):
			schema = self._get_schema()
			schema.get()

		def test_get_by_path_success(self):
			schema = self._get_schema()
			schema.get_by_path("properties.price")

		def test_get_by_path_fail(self):
			schema = self._get_schema()
			self.assertRaises(SchemaPathException, lambda:
				schema.get_by_path("a.b.c"))

		def test_required(self):
			schema = self._get_schema()
			self.assertRaises(SchemaValidationException, lambda:
				schema.validate({ "price": 38 }))

		def test_validate_maximum(self):
			schema = self._get_schema()
			self.assertRaises(SchemaValidationException, lambda:
				schema.validate({ "name": "Apple", "price": 41 }))

		def test_validate_minimum(self):
			schema = self._get_schema()
			self.assertRaises(SchemaValidationException, lambda:
				schema.validate({ "name": "Eggs", "price": 34.99 }))

		def test_check_format_unknown(self):
			schema = Schema({})
			self.assertRaises(SchemaException, lambda:
				schema._check_format("abc", { "format": "abc" }, "abc"))

		def test_check_format_regex(self):
			schema = Schema({})
			schema._check_format('/^\d{4}-\d{2}-\d{2}$/',
				{ "format": "regex" }, "field1")

		def test_check_format_email(self):
			schema = Schema({})
			schema._check_format("test@test.com",
				{ "format": "email" }, "field2")

		def test_check_one_of(self):
			schema = Schema({})
			schema._check_format("192.168.10.101", {
					"type": "string",
					"oneOf": [{
						"type": "string",
						"format": "ipv6"
					},{
						"type": "string",
						"format": "ipv4"
					}]
				}, "field3")

	unittest.main()
