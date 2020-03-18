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
    "Database",
    "DatabaseException",
    "DatabaseQueryException",
    "DatabaseQueryNotFoundException",
    "DatabaseFilter",
    "DatabaseGetQuery",
    "DatabaseGetByFilterQuery",
    "DatabaseSetQuery",
    "DatabaseDeleteQuery",
    "DatabaseDeleteByFilterQuery",
    "DatabaseIsReferencedQuery",
]

import abc
import os
import fcntl
import lxml.etree

import openmediavault.collections
import openmediavault.config.datamodel
import openmediavault.config.object


class DatabaseException(Exception):
    pass


class DatabaseQueryException(Exception):
    def __init__(self, xpath, model, message):
        self._xpath = xpath
        self._model = model
        super().__init__(message)

    @property
    def model(self):
        return self._model

    @property
    def xpath(self):
        return self._xpath


class DatabaseQueryNotFoundException(DatabaseQueryException):
    def __init__(self, xpath, model):
        super().__init__(xpath, model, "No such object: %s" % xpath)


class DatabaseFilter(openmediavault.collections.DotDict):
    pass


class Database:
    def get(self, id, identifier=None):
        """
        Get the specified configuration object.
        :param id: The data model identifier, e.g. 'conf.service.ftp'.
        :param identifier: The identifier of the configuration object, e.g.
                           the UUID. Defaults to None.
        :returns: Depending on the configuration object and whether
                  *identifier* is set, a list of configuration
                  objects or a single object is returned.
        """
        query = openmediavault.config.DatabaseGetQuery(id, identifier)
        query.execute()
        return query.response

    def get_by_filter(self, id, filter, **kwargs):
        """
        Get the iterable configuration objects that are matching the specified
        constraints.
        :param id: The data model identifier, e.g. 'conf.service.ftp'.
        :param filter: A filter specifying constraints on the objects
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
        :param min_result: The minimum number of objects that are expected.
        :param max_result: The maximum number of objects that are expected.
        :returns: A list containing the requested configuration objects.
                  If *max_result* is set to 1, then the first found object
                  is returned. In this case the method does not return a
                  list of configuration objects.
        """
        query = openmediavault.config.DatabaseGetByFilterQuery(id, filter)
        query.execute()
        if "min_result" in kwargs:
            min_result = kwargs.get("min_result")
            if min_result is not None and len(query.response) < min_result:
                raise DatabaseException(
                    "The query '{}' does not return the "
                    "minimum number of {} object(s).".format(
                        query.xpath, min_result))
        if "max_result" in kwargs:
            max_result = kwargs.get("max_result")
            if max_result is not None and len(query.response) > max_result:
                raise DatabaseException(
                    "The query '{}' returns more than the "
                    "maximum number of {} object(s).".format(
                        query.xpath, max_result))
        return query.response

    def exists(self, id, filter=None):
        """
        Check if on or more configuration object of the specified data
        model exists.
        :param id: The data model identifier.
        :param filter: A filter specifying constraints on the objects
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
        :returns: True if at least one configuration object exists,
                  otherwise False.
        """
        query = openmediavault.config.DatabaseGetByFilterQuery(id, filter)
        query.execute()
        if query.response is None:
            return False
        # pylint: disable=len-as-condition
        if isinstance(query.response, list) and 0 >= len(query.response):
            return False
        return True

    def is_referenced(self, obj):
        """
        Check if the specified object is referenced.
        :param obj:	The configuration object to use.
        :returns: True if the object is referenced, otherwise False.
        """
        query = openmediavault.config.DatabaseIsReferencedQuery(obj)
        query.execute()
        return query.response

    def is_unique(self, obj, property):
        """
        Check if a configuration object with the value of the specified
        property is unique.
        :param obj: The configuration object to use.
        :param property: The name of the data model property.
        :returns: True if no configuration object with the same
                  property value exists, otherwise False.
        """
        assert isinstance(obj, openmediavault.config.Object)
        return self.is_unique_by_filter(
            obj,
            DatabaseFilter(
                {
                    'operator': 'stringEquals',
                    'arg0': property,
                    'arg1': obj.get(property),
                }
            ),
        )

    def is_unique_by_filter(self, obj, filter):
        """
        Check if a configuration object with the specified constraints
        is unique.
        :param obj: The configuration object to use.
        :param filter: A filter specifying constraints on the objects
                       to retrieve.
                       ``
                       Example:
                       {
                           "operator": "stringEquals",
                           "arg0": "sharename",
                           "arg1": "Movies"
                       }
                       ``
        :returns: True if no configuration object with the same
                  property values (specified by the filter) exists,
                  otherwise False.
        """
        assert isinstance(obj, openmediavault.config.Object)
        assert isinstance(filter, DatabaseFilter)
        # If the object is iterable and not new, then we need to modify the
        # filter to do not find the configuration object itself.
        if not obj.is_new and obj.model.is_iterable:
            filter = DatabaseFilter(
                {
                    'operator': 'and',
                    'arg0': {
                        'operator': 'stringNotEquals',
                        'arg0': obj.model.idproperty,
                        'arg1': obj.get(obj.model.idproperty),
                    },
                    'arg1': filter,
                }
            )
        query = openmediavault.config.DatabaseGetByFilterQuery(
            obj.model.id, filter
        )
        query.execute()
        return 0 == len(query.response)

    def delete(self, obj):
        """
        Delete the specified configuration object.
        :param obj: The configuration object to delete.
        :returns: Returns the deleted configuration object.
        """
        assert isinstance(obj, openmediavault.config.Object)
        query = openmediavault.config.DatabaseDeleteQuery(obj)
        query.execute()
        return query.response

    def delete_by_filter(self, id, filter):
        """
        Delete the specified configuration objects that are matching the
        specified constraints.
        :param id: The data model identifier, e.g. 'conf.service.ftp'.
        :param filter: A filter specifying constraints on the objects
                       to retrieve.
                       ``
                       Example:
                       {
                           "operator": "not",
                           "arg0": {
                               "operator": "or",
                               "arg0": {
                                   "operator": "contains",
                                   "arg0": "opts",
                                   "arg1": "bind"
                               },
                               "arg1": {
                                   "operator": "contains",
                                   "arg0": "opts",
                                   "arg1": "loop"
                               }
                           }
                       }
                       ``
        :returns: Returns the deleted configuration objects.
        """
        query = openmediavault.config.DatabaseDeleteByFilterQuery(id, filter)
        query.execute()
        return query.response

    def set(self, obj):
        """
        Set the specified configuration object.
        :param obj: The configuration object to set.
        :returns: Returns the given configuration object. If the
                  configuration object was new, then the identifier has
                  been modified, e.g. a new UUID.
        """
        assert isinstance(obj, openmediavault.config.Object)
        query = openmediavault.config.DatabaseSetQuery(obj)
        query.execute()
        return obj


class DatabaseQuery(metaclass=abc.ABCMeta):  # lgtm[py/syntax-error]
    def __init__(self, id):
        """
        :param id: The data model identifier, e.g. 'conf.service.ftp.share'.
        """
        # Get the path to the database.
        self._database_file = openmediavault.getenv("OMV_CONFIG_FILE")
        os.stat(self._database_file)
        # Create the data model object.
        self._model = openmediavault.config.Datamodel(id)
        # Get the property names that must be handled as lists and dicts.
        self._force_list_tags = []
        self._force_dict_tags = []
        self._parse_model()
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

    @property
    @abc.abstractmethod
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

    def _parse_model(self):
        """
        Parse the data model and get the properties that must be handled
        as lists and dicts.
        """

        def callback(
            model, name, path, schema, user_data
        ):  # pylint: disable=unused-argument
            if "array" == schema['type'] and name:
                user_data['lists'].append(name)
            if "object" == schema['type'] and name:
                user_data['dicts'].append(name)

        tags = {"lists": [], "dicts": []}
        # ToDo: Refactor the whole process of collecting and processing
        # the special properties.
        self.model.walk_schema("", callback, tags)
        self._force_list_tags = tags['lists']
        self._force_dict_tags = [
            name for name in tags['dicts'] if name not in tags['lists']
        ]

    def _load(self):
        """
        Helper function to load the XML configuration file.
        """
        # Parse the XML configuration file.
        self._root_element = lxml.etree.parse(self._database_file)

    def _save(self):
        # Save the XML configuration file.
        with open(self._database_file, "wb") as f:
            fcntl.flock(f, fcntl.LOCK_EX)
            f.write(
                lxml.etree.tostring(
                    self._root_element,
                    pretty_print=True,
                    xml_declaration=True,
                    encoding="UTF-8",
                )
            )
            fcntl.flock(f, fcntl.LOCK_UN)

    def _get_root_element(self):
        """
        Get the root element of the configuration file XML tree.
        """
        if self._root_element is None:
            self._load()
        return self._root_element

    def _execute_xpath(self):
        """
        Helper method to execute the XPath query.
        :returns: Returns a list of lxml.etree.Element instancess that
                  match the specified XPath query.
        """
        root_element = self._get_root_element()
        # Execute the XPath query and return the matching elements.
        return root_element.xpath(self.xpath)

    def _element_to_dict(self, element):
        """
        Helper method to convert a lxml.etree.Element instance to a Python
        dictionary.
        :param element:	The lxml.etree.Element instance to convert.
        :returns: Returns a Python dictionary.
        """
        assert lxml.etree.iselement(element)
        result = {}
        for child_element in list(element):
            # Skip comments.
            if child_element.tag is lxml.etree.Comment:
                continue
            if list(child_element):
                value = self._element_to_dict(child_element)
            else:
                value = child_element.text
                # Empty strings are None, so convert them.
                value = "" if value is None else value
            tag = child_element.tag
            if tag in self._force_list_tags:
                try:
                    # Add value to an existing list.
                    result[tag].append(value)
                except AttributeError:
                    # Convert existing entry into a list.
                    result[tag] = [result[tag], value]
                except KeyError:
                    # Add a new entry.
                    result[tag] = [value]
            elif tag in self._force_dict_tags and value == "":
                # Create an empty dictionary if value is an empty string.
                result[tag] = {}
            else:
                result[tag] = value
        return result

    def _elements_to_object(self, elements):
        """
        Convert lxml.etree.Element instances to openmediavault.config.Object
        object instance(s).
        :param elements: A list of lxml.etree.Element instances.
        :returns: Returns a list of openmediavault.config.Object
                  objects if the data model of the configuration
                  object defines it as iterable. The list may be
                  empty.
                  A single openmediavault.config.Object object is
                  returned for non-iterable configuration objects.
                  If no element is given, then None is returned.
        """
        assert isinstance(elements, list)
        if self.model.is_iterable:
            result = []
            for element in elements:
                obj = openmediavault.config.Object(self.model.id)
                obj.set_dict(self._element_to_dict(element), False)
                result.append(obj)
        else:
            result = None
            if 0 < len(elements):  # pylint: disable=len-as-condition
                result = openmediavault.config.Object(self.model.id)
                result.set_dict(self._element_to_dict(elements[0]), False)
        return result

    def _dict_to_elements(self, dictionary, element):
        """
        Helper method to convert a Python dictionary into xml.etree.Element
        instances that are appended as sub elements to the specified
        xml.etree.Element instance.
        :param dictionary: The dictionary to process.
        :param element: The parent lxml.etree.Element instance where to
                        append the dictionary values.
        """

        def _process_value(element, value):
            assert lxml.etree.iselement(element)
            if isinstance(value, list):
                for sub_key, sub_value in enumerate(value):
                    if 0 == sub_key:
                        _process_value(element, sub_value)
                    else:
                        sub_element = lxml.etree.Element(element.tag)
                        element.getparent().append(sub_element)
                        _process_value(sub_element, sub_value)
            elif isinstance(value, dict):
                for sub_key, sub_value in value.items():
                    # Skip those properties that must be handled as an
                    # array/list and that are empty.
                    if sub_key in self._force_list_tags and not sub_value:
                        continue
                    sub_element = lxml.etree.Element(sub_key)
                    element.append(sub_element)
                    _process_value(sub_element, sub_value)
            else:
                if type(value) == bool:
                    element.text = "1" if value is True else "0"
                else:
                    element.text = str(value)

        assert isinstance(dictionary, dict)
        assert lxml.etree.iselement(element)
        for sub_key, sub_value in dictionary.items():
            sub_element = lxml.etree.Element(sub_key)
            element.append(sub_element)
            _process_value(sub_element, sub_value)

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
        | distinct         | property name |              |
        '-------------------------------------------------'
        Example 1:
        [type='bond' and devicename='bond0']
        The filter for the above predicate:
        [
            "operator": "and",
            "arg0": [
                "operator": "stringEquals",
                "arg0": "type",
                "arg1": "bond"
            ],
            "arg1": [
                "operator": "stringEquals",
                "arg0": "devicename",
                "arg1": "bond0"
            ]
        ]
        Example 2:
        [type='bond' and contains(slaves,'eth0')]
        The filter for the above predicate:
        [
            "operator": "and",
            "arg0": [
                "operator": "stringEquals",
                "arg0": "type",
                "arg1": "bond"
            ],
            "arg1": [
                "operator": "stringContains",
                "arg0": "slaves",
                "arg1": "eth0"
            ]
        ]
        Example 3:
        [not type='vlan']
        The filter for the above predicate:
        [
            "operator": "not",
            "arg0": [
                "operator": "stringEquals",
                "arg0": "type",
                "arg1": "vlan"
            ]
        ]
        Example 4:
        [
            "operator": "and",
            "arg0": [
                "operator": "stringNotEquals",
                "arg0": "uuid",
                "arg1": object.get("uuid")
            ],
            "arg1": [
                "operator": "and",
                "arg0": [
                    "operator": "stringEquals",
                    "arg0": "mntentref",
                    "arg1": object.get("mntentref")
                ],
                "arg1": [
                    "operator": "stringEquals",
                    "arg0": "reldirpath",
                    "arg1": object.get("reldirpath")
                ]
            ]
        ]
        """
        assert isinstance(filter, DatabaseFilter)
        if not "operator" in filter:
            raise KeyError("Invalid filter, the field 'operator' is missing.")
        result = ""
        if filter['operator'] in ['and', 'or']:
            result = "(%s %s %s)" % (
                self._build_predicate(DatabaseFilter(filter['arg0'])),
                filter['operator'],
                self._build_predicate(DatabaseFilter(filter['arg1'])),
            )
        elif filter['operator'] in ['=', 'equals']:
            result = "%s=%s" % (filter['arg0'], filter['arg1'])
        elif filter['operator'] in ['!=', 'notEquals']:
            result = "%s!=%s" % (filter['arg0'], filter['arg1'])
        elif "enum" == filter['operator']:
            parts = []
            for enumv in filter['arg1']:
                parts.append("%s=%s" % (filter['arg0'], enumv))
            result = "(%s)" % " or ".join(parts)
        elif filter['operator'] in ['==', 'stringEquals']:
            result = "%s='%s'" % (filter['arg0'], filter['arg1'])
        elif filter['operator'] in ['!==', '!=', 'stringNotEquals']:
            result = "%s!='%s'" % (filter['arg0'], filter['arg1'])
        elif "stringContains" == filter['operator']:
            result = "contains(%s,'%s')" % (filter['arg0'], filter['arg1'])
        elif "stringStartsWith" == filter['operator']:
            result = "starts-with(%s,'%s')" % (filter['arg0'], filter['arg1'])
        elif "stringEnum" == filter['operator']:
            parts = []
            for enumv in filter['arg1']:
                parts.append("%s='%s'" % (filter['arg0'], enumv))
            result = "(%s)" % " or ".join(parts)
        elif filter['operator'] in ['!', 'not']:
            result = "not(%s)" % (
                self._build_predicate(DatabaseFilter(filter['arg0']))
            )
        elif filter['operator'] in ['<', 'less']:
            result = "%s<%s" % (filter['arg0'], filter['arg1'])
        elif filter['operator'] in ['>', 'greater']:
            result = "%s>%s" % (filter['arg0'], filter['arg1'])
        elif filter['operator'] in ['<=', 'lessEqual']:
            result = "%s<=%s" % (filter['arg0'], filter['arg1'])
        elif filter['operator'] in ['>=', 'greaterEqual']:
            result = "%s>=%s" % (filter['arg0'], filter['arg1'])
        elif "distinct" == filter['operator']:
            result = "not({0}=preceding-sibling::*/{0})".format(filter['arg0'])
        else:
            raise ValueError(
                "The operator '%s' is not defined." % filter['operator']
            )
        return result


class DatabaseGetByFilterQuery(DatabaseQuery):
    def __init__(self, id, filter):
        if not filter is None:
            assert isinstance(filter, DatabaseFilter)
        self._filter = filter
        super().__init__(id)

    @property
    def filter(self):
        return self._filter

    @property
    def xpath(self):
        if self.filter:
            return "%s[%s]" % (
                self.model.queryinfo['xpath'],
                self._build_predicate(self.filter),
            )
        return self.model.queryinfo['xpath']

    def execute(self):
        elements = self._execute_xpath()
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
            return DatabaseGetByFilterQuery(
                self.model.id,
                DatabaseFilter(
                    {
                        'operator': 'stringEquals',
                        'arg0': self.model.idproperty,
                        'arg1': self.identifier,
                    }
                ),
            ).xpath
        return self.model.queryinfo['xpath']

    def execute(self):
        elements = self._execute_xpath()
        self._response = self._elements_to_object(elements)
        # Validate the query result.
        # If the object is iterable and if there is an identifier,
        # then only one object MUST be returned.
        if self.model.is_iterable and self.identifier:
            try:
                self._response = self._response[0]
            except IndexError:
                # Reset the response object.
                self._response = None
                # Raise an exception.
                raise DatabaseQueryNotFoundException(self.xpath, self.model)


class DatabaseIsReferencedQuery(DatabaseQuery):
    def __init__(self, obj):
        assert isinstance(obj, openmediavault.config.Object)
        self._obj = obj
        super().__init__(obj.model.id)

    @property
    def object(self):
        return self._obj

    @property
    def xpath(self):
        return "//%s[%s]" % (
            self.model.refproperty,
            self._build_predicate(
                DatabaseFilter(
                    {
                        'operator': 'stringContains',
                        'arg0': '.',
                        'arg1': self.object.get(self.model.idproperty),
                    }
                )
            ),
        )

    def execute(self):
        elements = self._execute_xpath()
        self._response = 0 < len(elements)


class DatabaseSetQuery(DatabaseQuery):
    def __init__(self, obj):
        assert isinstance(obj, openmediavault.config.Object)
        self._obj = obj
        super().__init__(obj.model.id)

    @property
    def object(self):
        return self._obj

    @property
    def xpath(self):
        if self.model.is_iterable and not self.object.is_new:
            # Find and update the element with the specified identifier.
            return DatabaseGetByFilterQuery(
                self.model.id,
                DatabaseFilter(
                    {
                        'operator': 'stringEquals',
                        'arg0': self.model.idproperty,
                        'arg1': self.object.get(self.model.idproperty),
                    }
                ),
            ).xpath

        # Find all elements matching the XPath.
        return self.model.queryinfo['xpath']

    def execute(self):
        def _create_child_element():
            # Split the XPath to get the parent path and the tag name.
            # Example:
            # //system/network/interfaces/interface =>
            #   parent path = //system/network/interfaces
            #   tag = interface
            xpath, tag = self.xpath.rsplit("/", 1)
            # Get the first parent element that is found.
            root_element = self._get_root_element()
            parent_element = root_element.find(xpath)
            assert lxml.etree.iselement(parent_element)
            # Create and append the child element to the parent element.
            child_element = lxml.etree.Element(tag)
            parent_element.append(child_element)
            return child_element

        # Execute the query.
        elements = self._execute_xpath()
        # Validate the query result.
        if self.model.is_iterable:
            # If an identifier was set for an iterable object, then there
            # MUST exist at least one element.
            if not self.object.is_new:
                if 0 == len(elements):  # pylint: disable=len-as-condition
                    raise DatabaseQueryNotFoundException(
                        self.xpath, self.object.model
                    )
            # Note, new iterable elements MUST be handled different.
            if self.object.is_new:
                # Create an empty element in the XML tree. It will be replaced
                # with the correct object values in a following step.
                child_element = _create_child_element()
                # Note, we need to take care that already existing elements
                # are not overwritten. So clear them out and only append the
                # new one.
                elements.clear()
                elements.append(child_element)
                # Finally create the new object identifier.
                self.object.create_id()
        else:
            # Handle non-iterable elements that do not exist until now.
            if 0 == len(elements):  # pylint: disable=len-as-condition
                # Create an empty element in the XML tree.
                child_element = _create_child_element()
                elements.append(child_element)
        # Put the configuration object.
        for element in elements:
            # Get the parent element.
            parent = element.getparent()
            assert lxml.etree.iselement(parent)
            # Create the clone of the element and set the new values.
            new_element = lxml.etree.Element(element.tag)
            self._dict_to_elements(self.object.get_dict(), new_element)
            # Replace the old element with the new one.
            parent.replace(element, new_element)
        self._save()


class DatabaseDeleteQuery(DatabaseQuery):
    def __init__(self, obj):
        assert isinstance(obj, openmediavault.config.Object)
        self._obj = obj
        super().__init__(obj.model.id)

    @property
    def object(self):
        return self._obj

    @property
    def xpath(self):
        if self.model.is_iterable:
            return DatabaseGetByFilterQuery(
                self.model.id,
                DatabaseFilter(
                    {
                        'operator': 'stringEquals',
                        'arg0': self.model.idproperty,
                        'arg1': self.object.get(self.model.idproperty),
                    }
                ),
            ).xpath
        return self.model.queryinfo['xpath']

    def execute(self):
        elements = self._execute_xpath()
        self._response = self._elements_to_object(elements)
        try:
            self._response = self._response[0]
        except Exception:
            self._response = None
        for element in elements:
            parent = element.getparent()
            if parent is None:
                continue
            parent.remove(element)
        self._save()


class DatabaseDeleteByFilterQuery(DatabaseGetByFilterQuery):
    def execute(self):
        elements = self._execute_xpath()
        self._response = self._elements_to_object(elements)
        for element in elements:
            parent = element.getparent()
            if parent is None:
                continue
            parent.remove(element)
        self._save()
