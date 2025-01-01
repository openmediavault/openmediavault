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

import openmediavault.config.datamodel


class DatamodelTestCase(unittest.TestCase):
    def _get_model(self):
        return openmediavault.config.Datamodel(
            {
                "type": "config",
                "id": "conf.system.apt.distribution",
                "queryinfo": {
                    "xpath": "//system/apt/distribution",
                    "iterable": False,
                },
                "properties": {
                    "proposed": {"type": "boolean", "default": False},
                    "partner": {"type": "boolean", "default": False},
                    "integer": {"type": "integer", "default": 10},
                },
            }
        )

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

    def test_validate(self):
        datamodel = self._get_model()
        datamodel.validate({"proposed": True, "partner": False, "integer": 20})

    def test_validate_fail(self):
        datamodel = self._get_model()
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: datamodel.validate(
                {"proposed": "foo", "partner": True, "integer": True}
            ),
        )


if __name__ == "__main__":
    unittest.main()
