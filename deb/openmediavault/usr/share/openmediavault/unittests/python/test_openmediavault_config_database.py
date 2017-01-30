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
import openmediavault.config.database
import openmediavault.config.object

class DatabaseTestCase(unittest.TestCase):
	def test_constructor(self):
		db = openmediavault.config.Database()

	def test_get(self):
		db = openmediavault.config.Database()
		db.get("conf.system.time")

	def test_get_iterable(self):
		#db = openmediavault.config.Database()
		#obj = db.get("conf.system.notification.notification",
		#	"c1cd54af-660d-4311-8e21-2a19420355bb")
		#self.assertTrue(isinstance(obj, openmediavault.config.Object))
		#self.assertEqual(obj.get("id"), "monitloadavg")
		pass

	def test_exists(self):
		#db = openmediavault.config.Database()
		#exists = db.exists("conf.system.notification.notification",
		#	openmediavault.config.DatabaseFilter({
		#		'operator': 'stringEquals',
		#		'arg0': 'id',
		#		'arg1': 'smartmontools'
		#	}))
		#self.assertTrue(exists)
		pass

	def test_get_list_elements(self):
		query = openmediavault.config.DatabaseGetRequest("conf.service.rsyncd")
		self.assertEqual(query._get_list_elements(), [ "module", "user" ])

	def test_get_query(self):
		query = openmediavault.config.DatabaseGetRequest("conf.system.time")
		self.assertEqual(query.xpath, "//system/time")

	def test_get_query_iterable(self):
		query = openmediavault.config.DatabaseGetRequest(
			"conf.system.notification.notification",
			"394cd565-e463-4094-a6ab-12e80270e9b4")
		self.assertEqual(query.xpath, "//system/notification/notifications/" \
			"notification[uuid='394cd565-e463-4094-a6ab-12e80270e9b4']")

	def test_filter_query_1(self):
		query = openmediavault.config.DatabaseFilterRequest(
			"conf.system.notification.notification",
			openmediavault.config.DatabaseFilter({
				'operator': 'stringContains',
				'arg0': 'id',
				'arg1': 'monit'
			}))
		self.assertEqual(query.xpath, "//system/notification/notifications/" \
			"notification[contains(id,'monit')]")

	def test_filter_query_2(self):
		query = openmediavault.config.DatabaseFilterRequest(
			"conf.system.network.proxy",
			openmediavault.config.DatabaseFilter({
				'operator': 'or',
				'arg0': {
					'operator': '=',
					'arg0': 'port',
					'arg1': 8080
				},
				'arg1': {
					'operator': 'equals',
					'arg0': 'port',
					'arg1': 4443
				}
			}))
		self.assertEqual(query.xpath, "//system/network/proxy[(port=8080 or " \
			"port=4443)]")

	def test_is_referenced_query(self):
		pass

if __name__ == "__main__":
	unittest.main()
