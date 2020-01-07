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
import unittest
import openmediavault.settings


class EnvironmentTestCase(unittest.TestCase):
    def test_get_bool(self):
        value = openmediavault.settings.Environment.get_str("OMV_DEBUG_SCRIPT")
        self.assertTrue(isinstance(value, str))

    def test_get_bool(self):
        value = openmediavault.settings.Environment.get_bool("OMV_DEBUG_PHP")
        self.assertTrue(isinstance(value, bool))

    def test_get_int(self):
        value = openmediavault.settings.Environment.get_int(
            "OMV_ENGINED_SO_SNDTIMEO"
        )
        self.assertTrue(isinstance(value, int))

    def test_get_float(self):
        value = openmediavault.settings.Environment.get_float(
            "OMV_XXX_YYY_ZZZ", 10.50
        )
        self.assertTrue(isinstance(value, float))


if __name__ == "__main__":
    unittest.main()
