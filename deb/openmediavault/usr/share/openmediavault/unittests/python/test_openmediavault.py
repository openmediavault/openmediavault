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

import openmediavault


class FunctionsTestCase(unittest.TestCase):
    def test_bool_yes(self):
        value = openmediavault.bool("yes")
        self.assertEqual(value, True)

    def test_bool_no(self):
        value = openmediavault.bool("no")
        self.assertEqual(value, False)

    def test_getenv_bool(self):
        value = openmediavault.getenv("OMV_DEBUG_SCRIPT", return_type="bool")
        self.assertEqual(isinstance(value, bool), True)

    def test_getenv_str(self):
        value = openmediavault.getenv("OMV_MOUNT_DIR")
        self.assertEqual(isinstance(value, str), True)

    def test_getenv_int(self):
        value = openmediavault.getenv(
            "OMV_ENGINED_SO_SNDTIMEO", return_type="int")
        self.assertEqual(isinstance(value, int), True)

    def test_getenv_float(self):
        value = openmediavault.getenv("OMV_ABC", 1.5, return_type="float")
        self.assertEqual(isinstance(value, float), True)

    def test_getenv_fail(self):
        self.assertRaises(
            TypeError,
            lambda: openmediavault.getenv("OMV_ABC", 1.5, return_type="list"),
        )


if __name__ == "__main__":
    unittest.main()
