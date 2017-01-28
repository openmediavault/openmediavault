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
import openmediavault.config.object

class ConfigObjectTestCase(unittest.TestCase):
	def test_constructor(self):
		conf_obj = openmediavault.config.Object("conf.service.ftp.share")

	def test_get_defaults(self):
		conf_obj = openmediavault.config.Object("conf.service.ftp.share")
		defaults = conf_obj.get_defaults()
		self.assertEqual(defaults, {
			'comment': '',
			'enable': False,
			'uuid': 'fa4b1c66-ef79-11e5-87a0-0002b3a176b4',
			'sharedfolderref': '',
			'extraoptions': '' })

	def test_set_get(self):
		conf_obj = openmediavault.config.Object("conf.service.ftp.share")
		conf_obj.set("comment", "test")
		value = conf_obj.get("comment")
		self.assertEqual(value, "test")

	def test_is_empty(self):
		conf_obj = openmediavault.config.Object("conf.service.ftp.share")
		self.assertEqual(conf_obj.is_empty("comment"), True)

	def test_not_is_empty(self):
		conf_obj = openmediavault.config.Object("conf.service.ftp.share")
		conf_obj.set("comment", "test")
		self.assertEqual(conf_obj.is_empty("comment"), False)

unittest.main()
