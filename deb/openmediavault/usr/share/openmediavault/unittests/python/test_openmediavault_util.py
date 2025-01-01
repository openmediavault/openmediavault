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

import openmediavault.utils


class UtilTestCase(unittest.TestCase):
    def test_to_bool_true_1(self):
        value = openmediavault.utils.to_bool('yes')
        self.assertEqual(value, True)

    def test_to_bool_true_2(self):
        value = openmediavault.utils.to_bool(True)
        self.assertEqual(value, True)

    def test_to_bool_true_3(self):
        value = openmediavault.utils.to_bool(1)
        self.assertEqual(value, True)

    def test_to_bool_true_4(self):
        value = openmediavault.utils.to_bool('1')
        self.assertEqual(value, True)

    def test_to_bool_true_5(self):
        value = openmediavault.utils.to_bool('oN')
        self.assertEqual(value, True)

    def test_to_bool_true_6(self):
        value = openmediavault.utils.to_bool('Y')
        self.assertEqual(value, True)

    def test_to_bool_true_7(self):
        value = openmediavault.utils.to_bool('True')
        self.assertEqual(value, True)

    def test_to_bool_false_1(self):
        value = openmediavault.utils.to_bool('No')
        self.assertEqual(value, False)

    def test_to_bool_false_2(self):
        value = openmediavault.utils.to_bool(False)
        self.assertEqual(value, False)

    def test_to_bool_false_3(self):
        value = openmediavault.utils.to_bool(0)
        self.assertEqual(value, False)


if __name__ == "__main__":
    unittest.main()
