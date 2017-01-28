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
import unittest
import openmediavault.collections

class DotDictTestCase(unittest.TestCase):
	def _get_dict(self):
		return openmediavault.collections.DotDict({
			'x': 3, 'a': { 'b': { 'c': 100 }}})

	def test_1(self):
		d = self._get_dict()
		self.assertEqual(d['a.b'], {'c': 100})

	def test_2(self):
		d = self._get_dict()
		self.assertEqual(d['a.b.c'], 100)

	def test_3(self):
		d = self._get_dict()
		d['a.b.c'] = 50
		self.assertEqual(d['a.b.c'], 50)

	def test_4(self):
		d = self._get_dict()
		d['a.b.d'] = { 'x': 'y' }
		self.assertEqual(d['a.b.d'], { 'x': 'y' })

	def test_5(self):
		d = self._get_dict()
		d['a'] = { 'z': 2.3 }
		self.assertEqual(d['a'], { 'z': 2.3 })

	def test_6(self):
		d = openmediavault.collections.DotDict()
		d['a'] = False
		self.assertEqual(d['a'], False)

if __name__ == "__main__":
	unittest.main()
