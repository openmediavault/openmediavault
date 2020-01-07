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
__all__ = [
    "Schema",
    "SchemaException",
    "SchemaPathException",
    "SchemaValidationException",
]

import decimal
import json
import re
import urllib.parse
import socket

import openmediavault.collections
import openmediavault.string


class SchemaException(Exception):
    pass


class SchemaPathException(Exception):
    def __init__(self, path):
        self._path = path
        super().__init__("The path '%s' is invalid." % path)

    @property
    def path(self):
        return self._path


class SchemaValidationException(Exception):
    def __init__(self, path, message):
        self._path = path
        super().__init__("%s: %s" % (path, message))

    @property
    def path(self):
        return self._path


# pylint: disable=no-self-use
class Schema:
    def __init__(self, schema):
        """
        :param schema: A JSON object (Python dictionary) describing the schema.
        """
        self._schema = openmediavault.collections.DotDict(schema)

    @property
    def schema(self):
        """
        Get the JSON schema as Python dictionary.
        :returns: Returns the JSON schema as openmediavault.collections.DotDict
                  dictionary.
        """
        return self.as_dict()

    def as_dict(self):
        """
        Get the JSON schema as Python dictionary.
        :returns: Returns the JSON schema as Python dictionary.
        """
        if isinstance(self._schema, str):
            self._schema = json.loads(self._schema)
        if not isinstance(self._schema, dict):
            raise TypeError("Expected dictionary.")
        return self._schema

    def get_by_path(self, path):
        """
        Get the JSON schema for the given path.
        :param path: The path of the requested attribute, e.g. "a.b.c".
        :returns: The JSON schema as Python dictionary describing the
                  requested attribute.
        :raises openmediavault.json.SchemaPathException:
        """

        def _validate_path(path):
            # Do we process the root node?
            if not path:
                return
            # Explode the path in dot notation into its parts.
            parts = path.split(".")
            # Validate the path. Something like "aa.bb.cc." "a..b.c", or
            # ".xx.yy" is invalid.
            if "" in parts:
                raise SchemaPathException(path)

        def _walk_schema(path, schema):
            # The schema must be a dictionary.
            if not isinstance(schema, dict):
                raise TypeError("Expected dictionary at '%s'." % path)
            # Do we have reached the end of the requested path (path is empty)?
            if not path:
                return schema
            # Explode the path in dot notation into its parts.
            parts = path.split(".")
            # Filter array indices from the path in dot notation.
            # Example: shares.share.0.uuid
            # To access the schema of an array we do not need them.
            parts = [part for part in parts if not part.isdigit()]
            # Do we process an 'object' or 'array' node?
            # !!! Note, the 'type' can be an array. How to handles them here?
            # ToDo: Handle types like '{ "type": [ "string", "object" ] }'
            if "type" in schema and isinstance(schema['type'], str):
                if "array" == schema['type']:
                    if not "items" in schema:
                        raise openmediavault.json.SchemaException(
                            "No 'items' attribute defined at '%s'." % path
                        )
                    return _walk_schema(path, schema['items'])
                elif "object" == schema['type']:
                    if not "properties" in schema:
                        raise openmediavault.json.SchemaException(
                            "No 'properties' attribute defined at '%s'." % path
                        )
                    return _walk_schema(path, schema['properties'])
                else:
                    raise SchemaException(
                        "Unknown type '%s' at '%s'." % (schema['type'], path)
                    )
            key = parts.pop(0)
            # Check if the node has the requested key/value pair.
            if not key in schema:
                raise SchemaPathException(path)
            # Continue to walk down the tree.
            return _walk_schema(".".join(parts), schema[key])

        _validate_path(path)
        return _walk_schema(path, self.schema)

    def validate(self, value, name=""):
        """
        Validate the given value.
        :param value: The value to validate.
        :param name: The JSON path of the entity to validate, e.g.
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
        schema = self.get_by_path(name)
        self._validate_type(value, schema, name)

    def _validate_type(self, value, schema, name):
        """
        :returns: None
        """
        types = "any"
        if "type" in schema:
            types = schema['type']
        if not isinstance(types, list):
            types = [types]
        valid = False
        last_exception = None
        for _, typev in enumerate(types):
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
                        "%s: The type '%s' is not defined." % (name, typev)
                    )
            except SchemaValidationException as e:
                # Continue with the next type but remember the exception.
                last_exception = e
            # Break the foreach loop here because one of the defined types
            # is successfully validated.
            if valid is True:
                break
        # If the validation is not successful, then trow the
        # last exception.
        if valid is False and last_exception is not None:
            # pylint: disable=raising-bad-type
            raise last_exception

    # pylint: disable=unused-argument
    def _validate_any(self, value, schema, name):
        pass

    def _validate_boolean(self, value, schema, name):
        if not isinstance(value, bool):
            raise SchemaValidationException(
                name,
                "The value '%s' is not a boolean."
                % ("NULL" if (value is None) else str(value)),
            )

    # lgtm[py/similar-function]
    def _validate_integer(self, value, schema, name):
        if not isinstance(value, int):
            raise SchemaValidationException(
                name,
                "The value '%s' is not an integer."
                % ("NULL" if (value is None) else str(value)),
            )
        self._check_minimum(value, schema, name)
        self._check_exclusive_minimum(value, schema, name)
        self._check_maximum(value, schema, name)
        self._check_exclusive_maximum(value, schema, name)
        self._check_enum(value, schema, name)
        self._check_one_of(value, schema, name)

    def _validate_number(self, value, schema, name):
        if not isinstance(value, (int, float, decimal.Decimal)):
            raise SchemaValidationException(
                name,
                "The value '%s' is not a number."
                % ("NULL" if (value is None) else str(value)),
            )
        self._check_minimum(value, schema, name)
        self._check_exclusive_minimum(value, schema, name)
        self._check_maximum(value, schema, name)
        self._check_exclusive_maximum(value, schema, name)
        self._check_enum(value, schema, name)
        self._check_one_of(value, schema, name)

    # lgtm[py/similar-function]
    def _validate_string(self, value, schema, name):
        if not isinstance(value, str):
            raise SchemaValidationException(
                name,
                "The value '%s' is not a string."
                % ("NULL" if (value is None) else str(value)),
            )
        self._check_pattern(value, schema, name)
        self._check_min_length(value, schema, name)
        self._check_max_length(value, schema, name)
        self._check_format(value, schema, name)
        self._check_enum(value, schema, name)
        self._check_one_of(value, schema, name)

    def _validate_array(self, value, schema, name):
        if not isinstance(value, list):
            raise SchemaValidationException(
                name,
                "The value '%s' is not an array."
                % ("NULL" if (value is None) else str(value)),
            )
        self._check_min_items(value, schema, name)
        self._check_max_items(value, schema, name)
        self._check_items(value, schema, name)

    def _validate_object(self, value, schema, name):
        if not isinstance(value, dict):
            raise SchemaValidationException(
                name,
                "The value '%s' is not an object."
                % ("NULL" if (value is None) else str(value)),
            )
        self._check_properties(value, schema, name)

    def _validate_null(self, value, schema, name):
        if not isinstance(value, type(None)):
            raise SchemaValidationException(
                name, "The value '%s' is not NULL." % str(value)
            )

    def _check_minimum(self, value, schema, name):
        if not "minimum" in schema:
            return
        if schema['minimum'] > value:
            raise SchemaValidationException(
                name,
                "The value %s is less than %s."
                % (str(value), str(schema['minimum'])),
            )

    def _check_maximum(self, value, schema, name):
        if not "maximum" in schema:
            return
        if schema['maximum'] < value:
            raise SchemaValidationException(
                name,
                "The value %s is bigger than %s."
                % (str(value), str(schema['maximum'])),
            )

    def _check_exclusive_minimum(self, value, schema, name):
        if not "minimum" in schema:
            return
        if not (
            "exclusiveMinimum" in schema
            and (schema['exclusiveMinimum'] is True)
        ):
            return
        if schema['minimum'] == value:
            raise SchemaValidationException(
                name,
                "Invalid value %s, must be greater than %s."
                % (str(value), str(schema['minimum'])),
            )

    def _check_exclusive_maximum(self, value, schema, name):
        if not "maximum" in schema:
            return
        if not (
            "exclusiveMaximum" in schema
            and (schema['exclusiveMaximum'] is True)
        ):
            return
        if schema['maximum'] == value:
            raise SchemaValidationException(
                name,
                "Invalid value %s, must be greater than %s."
                % (str(value), str(schema['maximum'])),
            )

    def _check_min_length(self, value, schema, name):
        if not "minLength" in schema:
            return
        if schema['minLength'] > len(value):
            raise SchemaValidationException(
                name,
                "The value '%s' is too short, minimum length is %d."
                % (value, schema['minLength']),
            )

    def _check_max_length(self, value, schema, name):
        if not "maxLength" in schema:
            return
        if schema['maxLength'] < len(value):
            raise SchemaValidationException(
                name,
                "The value '%s' is too long, maximum length is %d."
                % (value, schema['maxLength']),
            )

    def _check_pattern(self, value, schema, name):
        if not "pattern" in schema:
            return
        if not re.match(schema['pattern'], value):
            raise SchemaValidationException(
                name,
                "The value '%s' doesn't match the pattern '%s'."
                % (value, schema['pattern']),
            )

    def _check_format(self, value, schema, name):
        """
        Check https://tools.ietf.org/html/draft-zyp-json-schema-03#section-5.23
        """
        if not "format" in schema:
            return
        if schema['format'] in ["date-time", "datetime"]:
            if not re.match(r'^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$', value):
                raise SchemaValidationException(
                    name, "The value '%s' does not match to ISO 8601." % value
                )
        elif "date" == schema['format']:
            if not re.match(r'^\d{4}-\d{2}-\d{2}$', value):
                raise SchemaValidationException(
                    name, "The value '%s' does not match to YYYY-MM-DD." % value
                )
        elif "time" == schema['format']:
            if not re.match(r'^\d{2}:\d{2}:\d{2}$', value):
                raise SchemaValidationException(
                    name, "The value '%s' does not match to hh:mm:ss." % value
                )
        elif schema['format'] in ["host-name", "hostname"]:
            if not re.match(
                r'^[a-zA-Z]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9])' '{0,1}$', value
            ):
                raise SchemaValidationException(
                    name, "The value '%s' is not a valid hostname." % value
                )
        elif "regex" == schema['format']:
            try:
                re.compile(value)
            except:
                raise SchemaValidationException(
                    name, "The value '%s' is not a valid regex." % value
                )
        elif "uri" == schema['format']:
            try:
                urllib.parse.urlparse(value)
            except:
                raise SchemaValidationException(
                    name, "The value '%s' is not an URI." % value
                )
        elif "email" == schema['format']:
            if not "@" in value:
                raise SchemaValidationException(
                    name, "The value '%s' is not an email." % value
                )
        elif schema['format'] in ["ip-address", "ipv4"]:
            try:
                socket.inet_aton(value)
            except socket.error:
                raise SchemaValidationException(
                    name, "The value '%s' is not an IPv4 address." % value
                )
        elif "ipv6" == schema['format']:
            try:
                socket.inet_pton(socket.AF_INET6, value)
            except socket.error:
                raise SchemaValidationException(
                    name, "The value '%s' is not an IPv6 address." % value
                )
        else:
            raise SchemaException(
                "%s: The format '%s' is not defined." % (name, schema['format'])
            )

    def _check_enum(self, value, schema, name):
        if not "enum" in schema:
            return
        if not isinstance(schema['enum'], list):
            raise SchemaException(
                "The attribute 'enum' is not an array." % name
            )
        if not isinstance(value, list):
            value = [value]
        for valuev in value:
            if not valuev in schema['enum']:
                raise SchemaValidationException(
                    name,
                    "Invalid value '%s', allowed values are '%s'."
                    % (valuev, ", ".join(schema['enum'])),
                )

    def _check_min_items(self, value, schema, name):
        if not "minItems" in schema:
            return
        if schema['minItems'] > len(value):
            raise SchemaValidationException(
                name,
                "Not enough array items, minimum is %d." % schema['minItems'],
            )

    def _check_max_items(self, value, schema, name):
        if not "maxItems" in schema:
            return
        if schema['maxItems'] < len(value):
            raise SchemaValidationException(
                name,
                "Too many array items, maximum is %d." % schema['maxItems'],
            )

    def _check_properties(self, value, schema, name):
        if not "properties" in schema:
            raise SchemaException(name, "No 'properties' attribute defined.")
        if not isinstance(schema['properties'], dict):
            raise SchemaException(
                name, "The attribute 'properties' is not a dictionary."
            )
        for propk, propv in schema['properties'].items():
            # Build the new path. Strip empty parts.
            parts = [name, propk]
            parts = [part for part in parts if part]
            path = ".".join(parts)
            # Check if the 'required' attribute is set.
            if not propk in value:
                if ("required" in propv) and (propv['required'] is True):
                    raise SchemaValidationException(
                        name, "Missing 'required' attribute '%s'." % path
                    )
                continue
            self._validate_type(value[propk], propv, path)

    def _check_items(self, value, schema, name):
        if not "items" in schema:
            raise SchemaException("No 'items' attribute defined." % name)
        if isinstance(schema['items'], dict):
            for itemk, itemv in value.items():
                path = "%s[%d]" % (name, itemk)
                valid = True
                for item_schema in schema['items']:
                    try:
                        self._validate_type(itemv, item_schema, path)
                    except SchemaValidationException:
                        valid = False
                if not valid:
                    types = map(lambda x: x['type'], schema['items'])
                    raise SchemaValidationException(
                        name,
                        "Invalid 'items' value, must be one of the "
                        "following types '%s'." % ", ".join(types),
                    )
        elif isinstance(schema['items'], list):
            for itemk, itemv in enumerate(value):
                path = "%s[%d]" % (name, itemk)
                self._validate_type(itemv, schema['items'], path)
        else:
            raise SchemaValidationException(name, "Invalid 'items' value.")

    def _check_one_of(self, value, schema, name):
        if not "oneOf" in schema:
            return
        if not isinstance(schema['oneOf'], list):
            raise SchemaException(
                "The 'oneOf' attribute is not an array." % name
            )
        valid = False
        for _, sub_schemav in enumerate(schema['oneOf']):
            try:
                self._validate_type(value, sub_schemav, name)
                # If validation succeeds for one of the schema, then we
                # can exit immediatelly.
                valid = True
            except SchemaValidationException:
                # Nothing to do here.
                pass
        if not valid:
            raise SchemaValidationException(
                name, "Failed to match exactly one schema."
            )
