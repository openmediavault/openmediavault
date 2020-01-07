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
__all__ = ["Datamodel"]

import abc
import json


class Datamodel:
    def __init__(self, model):
        """
        :param model: The data model as Python dictionary or JSON string.
        """
        # Convert into a JSON object if it is a string.
        if isinstance(model, str):
            model = json.loads(model)
        else:
            if not isinstance(model, dict):
                raise TypeError("Expected dictionary.")
        self._model = model

    def as_dict(self):
        """
        Get the data model as Python dictionary.
        :returns: Returns the data model as Python dictionary.
        """
        return self._model

    @property
    def model(self):
        """
        Get the data model as Python dictionary.
        :returns: Returns the data model as Python dictionary.
        """
        return self.as_dict()

    @property
    def id(self):  # pylint: disable=invalid-name
        """
        Get the model identifier, e.g. 'conf.service.rsyncd.module'.
        :returns: Returns the model identifier.
        """
        if not "id" in self.model:
            return ""
        return self.model['id']

    @property
    def alias(self):
        """
        Get the model identifier alias.
        :returns: Returns the model identifier alias.
        """
        if not "alias" in self.model:
            return ""
        return self._model['alias']

    @property
    def title(self):
        """
        Get the model title, e.g. 'SSH certificate'.
        :returns: Returns the model titel.
        """
        if not "title" in self.model:
            return ""
        return self._model['title']

    @property
    def description(self):
        """
        Get the model description.
        :returns: Returns the model description.
        """
        if not "description" in self.model:
            return ""
        return self._model['description']

    @abc.abstractmethod
    def validate(self, data):
        """
        Validate the specified data against the data model.
        :param data: The JSON data to validate.
        :returns: None.
        """

    def __str__(self):
        """
        Return the data model as JSON string.
        :returns: Returns a JSON string.
        """
        return json.dumps(self.model)
