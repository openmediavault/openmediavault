# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2025 Volker Theile
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
import unittest

import openmediavault.datamodel.datamodel


class DatamodelTestCase(unittest.TestCase):
    def _get_model(self):
        return openmediavault.datamodel.Datamodel(
            {
                "type": "config",
                "id": "conf.system.apt.distribution",
                "alias": "conf.system.xyz",
                "description": "abc",
                "title": "xyz",
                "queryinfo": {
                    "xpath": "//system/apt/distribution",
                    "iterable": False,
                },
                "properties": {
                    "proposed": {"type": "boolean", "default": False},
                    "partner": {"type": "boolean", "default": False},
                },
            }
        )

    def test_model(self):
        datamodel = self._get_model()
        self.assertIsInstance(datamodel.model, dict)

    def test_id(self):
        datamodel = self._get_model()
        self.assertEqual(datamodel.id, "conf.system.apt.distribution")

    def test_alias(self):
        datamodel = self._get_model()
        self.assertEqual(datamodel.alias, "conf.system.xyz")

    def test_title(self):
        datamodel = self._get_model()
        self.assertEqual(datamodel.title, "xyz")

    def test_description(self):
        datamodel = self._get_model()
        self.assertEqual(datamodel.description, "abc")


if __name__ == "__main__":
    unittest.main()
