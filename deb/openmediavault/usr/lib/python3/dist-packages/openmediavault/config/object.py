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
__all__ = ["Object"]

import json
import uuid

import openmediavault
import openmediavault.string
import openmediavault.config.datamodel
import openmediavault.collections
import openmediavault.exceptions
import openmediavault.json.schema


class Object:
    def __init__(self, id):  # pylint: disable=redefined-builtin
        """
        :param id: The data model identifier, e.g. 'conf.service.ftp.share'.
        """
        # Set the data model.
        self._model = openmediavault.config.Datamodel(id)
        # Set the default values.
        self.reset_all()

    @property
    def model(self):
        """
        Get the data model of the configuration object.
        :returns: The openmediavault.config.Datamodel object of the
                  data model.
        """
        return self._model

    @property
    def properties(self):
        """
        Get the configuration object properties.
        :returns: Returns the properties of the configuration object as
                  openmediavault.collections.DotDict dictionary.
        """
        return self._properties

    @property
    def id(self):
        """
        Get the object identifier, e.g. UUID.
        :returns: Returns the object identifier, e.g. an UUID.
        """
        if not (self.model.is_iterable and self.model.is_identifiable):
            raise Exception(
                "The configuration object '%s' is not iterable "
                "and identifiable." % self.model.id
            )
        return self.get(self.model.idproperty)

    def create_id(self):
        """
        Create and set a new object identifier (UUID). Use this method only
        if the configuration object has an 'uuid' property.
        :returns: Returns the new object identifier.
        """
        if not (self.model.is_iterable and self.model.is_identifiable):
            raise Exception(
                "The configuration object '%s' is not iterable "
                "and identifiable." % self.model.id
            )
        if "uuid" != self.model.idproperty.lower():
            raise Exception(
                "The configuration object identifier must be " "of type UUID."
            )
        new_id = str(uuid.uuid4())
        self.set(self.model.idproperty, new_id)
        return new_id

    @property
    def is_new(self):
        """
        Check if the configuration object is new. Use this method only if
        the configuration object has an 'uuid' property.
        :returns: Returns True if the configuration object is identified as
                  new, otherwise False.
        """
        if not openmediavault.string.is_uuid4(self.id):
            return False
        return self.id == openmediavault.getenv("OMV_CONFIGOBJECT_NEW_UUID")

    def get_defaults(self):
        """
        Get the default values as defined in the data model.
        :returns: Returns the default values as defined in the data model
                  as openmediavault.collections.DotDict dictionary.
        """

        def callback(
            model, name, path, schema, user_data
        ):  # pylint: disable=unused-argument
            # Abort immediatelly if the path is empty.
            if not path:
                return None
            # Get the default value.
            user_data[path] = model.property_get_default(path)
            # Only get the default value for 'array' items, but do not further
            # walk down the data model definition.
            if schema['type'] == "array":
                return False

        defaults = openmediavault.collections.DotDict()
        self.model.walk_schema("", callback, defaults)
        return defaults

    def reset_all(self):
        """
        Reset all properties to their default values.
        """
        self._properties = self.get_defaults()

    def exists(self, name):
        """
        Check if the specified property exists.
        :param name: The name of the property in dot notation, e.g. 'a.b.c'.
        :returns: Returns True if the property exists, otherwise False.
        """
        return self.model.property_exists(name)

    def assert_exists(self, name):
        """
        Assert that the specified property exists.
        :param name: The name of the property in dot notation, e.g. 'a.b.c'.
        :raises openmediavault.exceptions.AssertException:
        """
        if not self.exists(name):
            raise openmediavault.exceptions.AssertException(
                "The property '%s' does not exist in the model '%s'."
                % (name, self.model.id)
            )

    def get(self, name):
        """
        Get a property.
        :param name: The name of the property in dot notation, e.g. 'a.b.c'.
        :returns: The property value.
        """
        self.assert_exists(name)
        return self.properties[name]

    def get_dict(self):
        """
        Get the properties.
        :returns: Returns the properties of the configuration object as
                  openmediavault.collections.DotDict dictionary.
        """
        return self.properties

    def set(self, name, value, validate=True):
        """
        Set a property.
        :param name: The name of the property in dot notation, e.g. 'a.b.c'.
        :param value: The value of the property.
        :param validate: Set to False to do not validate the value.
                         Defaults to True.
        """
        self.assert_exists(name)
        if validate:
            self.model.property_validate(name, value)
        # Convert the value into the proper type.
        value = self.model.property_convert(name, value)
        # Set the property value in the dictionary.
        self.properties[name] = value

    def set_dict(self, values, validate=True, ignore=False):
        """
        Set properties.
        :param values: The dictionary of key/value pairs.
        :param validate: Set to False to do not validate the property
                         values against the property schema defined in
                         the model.
        :param ignore: Set to True to ignore and skip unknown properties.
                       Defaults to False.
        """
        if not isinstance(values, dict):
            raise TypeError("Expected dictionary")
        # Create a flat representation of the dictionary and set the
        # key/value pairs.
        values_flat = openmediavault.collections.flatten(values)
        for key, value in values_flat.items():
            if ignore and not self.exists(key):
                continue
            self.set(key, value, validate)

    def reset(self, name):
        """
        Reset a property to its default value as defined in the data model.
        :param name: The name of the property in dot notation, e.g. 'a.b.c'.
        """
        defaults = self.get_defaults()
        self.set(name, defaults[name])

    def is_empty(self, name):
        """
        Determine whether a property value is empty.
        :param name: The name of the property in dot notation, e.g. 'a.b.c'.
        :returns: Returns False if the property exists and has a non-empty,
                  non-zero value, otherwise returns True. If the property
                  does not exist an exception is thrown.
        """
        value = self.get(name)
        return not bool(value and value.strip())

    def __str__(self):
        """
        Return the properties as JSON string.
        :returns: Returns a JSON string.
        """
        return json.dumps(self.properties)
