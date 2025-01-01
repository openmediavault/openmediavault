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
import os
import shutil
import tempfile
import unittest

import lxml.etree
import openmediavault.config.database
import openmediavault.config.object

import openmediavault


class DatabaseTestCase(unittest.TestCase):
    def setUp(self):
        # Tell the database implementation to use the test database.
        openmediavault.setenv(
            "OMV_CONFIG_FILE", "%s/../data/config.xml" % os.getcwd()
        )

    def _use_tmp_config_database(self):
        """
        Create a copy of the database and use this. This is useful if the
        database is modified during the test.
        """
        config_file = "%s/../data/config.xml" % os.getcwd()
        (fh, tmp_config_file) = tempfile.mkstemp()
        shutil.copy(config_file, tmp_config_file)
        openmediavault.setenv("OMV_CONFIG_FILE", tmp_config_file)

    def test_constructor(self):
        db = openmediavault.config.Database()
        self.assertIsNotNone(db)

    def test_empty_objects(self):
        db = openmediavault.config.Database()
        obj = db.get("conf.service.rsyncd")
        self.assertIsInstance(obj.get("modules"), dict)
        self.assertIsInstance(obj.get("modules.module"), list)
        self.assertEqual(obj.get("modules.module"), [])

    def test_get_1(self):
        db = openmediavault.config.Database()
        obj = db.get("conf.system.time")
        self.assertIsInstance(obj, openmediavault.config.Object)
        self.assertEqual(obj.get("timezone"), "Europe/Berlin")
        self.assertFalse(obj.get("ntp.enable"))
        self.assertEqual(obj.get("ntp.timeservers"), "pool.ntp.org")
        self.assertEqual(obj.get("ntp.clients"), "")

    def test_get_2(self):
        db = openmediavault.config.Database()
        obj = db.get("conf.system.apt.distribution")
        self.assertIsInstance(obj, openmediavault.config.Object)
        self.assertFalse(obj.get("proposed"))

    def test_get_iterable(self):
        db = openmediavault.config.Database()
        objs = db.get("conf.system.notification.notification")
        self.assertIsInstance(objs, list)
        self.assertTrue(0 < len(objs))
        self.assertIsInstance(objs[0], openmediavault.config.Object)

    def test_get_by_filter_1(self):
        db = openmediavault.config.Database()
        objs = db.get_by_filter(
            "conf.system.notification.notification",
            openmediavault.config.DatabaseFilter(
                {
                    'operator': 'stringEquals',
                    'arg0': 'uuid',
                    'arg1': '03dc067d-1310-45b5-899f-b471a0ae9233',
                }
            ),
        )
        self.assertIsInstance(objs, list)
        self.assertEqual(len(objs), 1)
        self.assertIsInstance(objs[0], openmediavault.config.Object)

    def test_get_by_filter_2(self):
        db = openmediavault.config.Database()
        objs = db.get_by_filter(
            "conf.system.notification.notification",
            openmediavault.config.DatabaseFilter(
                {'operator': 'stringContains', 'arg0': 'id', 'arg1': 'monit'}
            ),
        )
        self.assertIsInstance(objs, list)
        self.assertEqual(len(objs), 5)

    def test_get_by_filter_3(self):
        db = openmediavault.config.Database()
        objs = db.get_by_filter(
            "conf.service.smartmontools.device",
            openmediavault.config.DatabaseFilter(
                {'operator': 'distinct', 'arg0': 'enable'}
            ),
        )
        self.assertIsInstance(objs, list)
        self.assertEqual(len(objs), 2)

    def test_exists(self):
        db = openmediavault.config.Database()
        self.assertTrue(
            db.exists(
                "conf.system.notification.notification",
                openmediavault.config.DatabaseFilter(
                    {
                        'operator': 'stringEquals',
                        'arg0': 'id',
                        'arg1': 'smartmontools',
                    }
                ),
            )
        )

    def test_get_list_tags(self):
        query = openmediavault.config.DatabaseGetQuery("conf.service.rsyncd")
        # Attention, this is a private class member.
        self.assertIsInstance(query._force_list_tags, list)
        self.assertEqual(query._force_list_tags, ["module", "user"])

    def test_get_dict_tags(self):
        query = openmediavault.config.DatabaseGetQuery("conf.service.rsyncd")
        # Attention, this is a private class member.
        self.assertIsInstance(query._force_dict_tags, list)
        self.assertEqual(query._force_dict_tags, ["modules", "users"])

    def test_get_query(self):
        query = openmediavault.config.DatabaseGetQuery("conf.system.time")
        self.assertEqual(query.xpath, "//system/time")

    def test_get_query_fail(self):
        # Query a non-existend configuration object.
        db = openmediavault.config.Database()
        self.assertRaises(
            openmediavault.config.database.DatabaseQueryNotFoundException,
            lambda: db.get(
                "conf.system.notification.notification",
                "c1cd54af-0000-1111-2222-2a19420355bb",
            ),
        )

    def test_get_query_iterable(self):
        query = openmediavault.config.DatabaseGetQuery(
            "conf.system.notification.notification",
            "394cd565-e463-4094-a6ab-12e80270e9b4",
        )
        self.assertEqual(
            query.xpath,
            "//system/notification/notifications/"
            "notification[uuid='394cd565-e463-4094-a6ab-12e80270e9b4']",
        )

    def test_filter_query_1(self):
        query = openmediavault.config.DatabaseGetByFilterQuery(
            "conf.system.notification.notification",
            openmediavault.config.DatabaseFilter(
                {'operator': 'stringContains', 'arg0': 'id', 'arg1': 'monit'}
            ),
        )
        self.assertEqual(
            query.xpath,
            "//system/notification/notifications/"
            "notification[contains(id,'monit')]",
        )

    def test_filter_query_2(self):
        query = openmediavault.config.DatabaseGetByFilterQuery(
            "conf.system.network.proxy",
            openmediavault.config.DatabaseFilter(
                {
                    'operator': 'or',
                    'arg0': {'operator': '=', 'arg0': 'port', 'arg1': 8080},
                    'arg1': {
                        'operator': 'equals',
                        'arg0': 'port',
                        'arg1': 4443,
                    },
                }
            ),
        )
        self.assertEqual(
            query.xpath, "//system/network/proxy[(port=8080 or " "port=4443)]"
        )

    def test_is_unique(self):
        db = openmediavault.config.Database()
        obj = db.get(
            "conf.system.notification.notification",
            "c1cd54af-660d-4311-8e21-2a19420355bb",
        )
        self.assertIsInstance(obj, openmediavault.config.Object)
        self.assertTrue(db.is_unique(obj, "uuid"))

    def test_is_referenced_query_1(self):
        db = openmediavault.config.Database()
        obj = db.get(
            "conf.system.sharedfolder", "339bd101-5744-4017-9392-01a156f15ab9"
        )
        self.assertIsInstance(obj, openmediavault.config.Object)
        self.assertTrue(db.is_referenced(obj))

    def test_is_referenced_query_2(self):
        db = openmediavault.config.Database()
        obj = db.get(
            "conf.system.sharedfolder", "91fe93fc-ef9d-11e6-9b06-000c2900c2de"
        )
        self.assertIsInstance(obj, openmediavault.config.Object)
        self.assertFalse(db.is_referenced(obj))

    def test_delete(self):
        self._use_tmp_config_database()
        db = openmediavault.config.Database()
        obj = db.get(
            "conf.system.notification.notification",
            "03dc067d-1310-45b5-899f-b471a0ae9233",
        )
        self.assertIsInstance(obj, openmediavault.config.Object)
        query = openmediavault.config.DatabaseDeleteQuery(obj)
        self.assertEqual(
            query.xpath,
            "//system/notification/notifications/"
            "notification[uuid='03dc067d-1310-45b5-899f-b471a0ae9233']",
        )
        query.execute()
        self.assertIsInstance(query.response, openmediavault.config.Object)
        self.assertEqual(query.response.get("id"), "monitmemoryusage")
        # Ensure that the object does not exist anymore.
        self.assertFalse(
            db.exists(
                obj.model.id,
                openmediavault.config.DatabaseFilter(
                    {
                        'operator': 'stringEquals',
                        'arg0': obj.model.idproperty,
                        'arg1': obj.get(obj.model.idproperty),
                    }
                ),
            )
        )

    def test_delete_by_filter(self):
        self._use_tmp_config_database()
        db = openmediavault.config.Database()
        objs = db.delete_by_filter(
            "conf.system.notification.notification",
            openmediavault.config.DatabaseFilter(
                {'operator': 'stringContains', 'arg0': 'id', 'arg1': 'monit'}
            ),
        )
        self.assertIsInstance(objs, list)
        self.assertEqual(len(objs), 5)
        # Check the number of remaining configuration objects.
        objs = db.get("conf.system.notification.notification")
        self.assertEqual(len(objs), 3)

    def test_dict_to_elements(self):
        #  Create a fake query object to access the helper method.
        query = openmediavault.config.DatabaseGetQuery(
            "conf.system.notification.notification"
        )
        root = lxml.etree.Element("config")
        query._dict_to_elements(
            {
                'a': 1,
                'b1': False,
                'b2': True,
                'c': {'x': "test"},
                'd': 5.45,
                'f': {'z': [1, 2, 3]},
                'g': ['x', 'y'],
            },
            root,
        )
        """
		The result XML tree should look like:
		<config>
			<a>1</a>
			<b1>0</b1>
			<b2>1</b2>
			<c>
				<x>test</x>
			</c>
			<d>5.45</d>
			<f>
				<z>1</z>
				<z>2</z>
				<z>3</z>
			</f>
			<g>x</g>
			<g>y</g>
		</config>
		"""
        # xml = lxml.etree.tostring(root, encoding="unicode")
        # self.assertEqual(xml, "<config><b1>0</b1><c><x>test</x></c>" \
        # 	"<b2>1</b2><f><z>1</z><z>2</z><z>3</z></f><d>5.45</d>" \
        # 	"<a>1</a><g>x</g><g>y</g></config>")

    def test_update(self):
        self._use_tmp_config_database()
        db = openmediavault.config.Database()
        obj = db.get("conf.system.apt.distribution")
        self.assertIsInstance(obj, openmediavault.config.Object)
        self.assertFalse(obj.get("proposed"))
        obj.set("proposed", True)
        db.set(obj)
        # Get the configuration object to validate its properties.
        obj = db.get("conf.system.apt.distribution")
        self.assertIsInstance(obj, openmediavault.config.Object)
        self.assertTrue(obj.get("proposed"))

    def test_set_new_iterable(self):
        self._use_tmp_config_database()
        db = openmediavault.config.Database()
        # Check the current number of configuration objects.
        objs = db.get("conf.system.notification.notification")
        self.assertIsInstance(objs, list)
        self.assertEqual(len(objs), 8)
        # Create a new configuration object.
        new_obj = openmediavault.config.Object(
            "conf.system.notification.notification"
        )
        new_obj.set_dict(
            {
                'uuid': openmediavault.getenv('OMV_CONFIGOBJECT_NEW_UUID'),
                'id': 'test',
                'enable': False,
            }
        )
        result_obj = db.set(new_obj)
        # Validate the returned configuration object.
        self.assertIsInstance(result_obj, openmediavault.config.Object)
        self.assertNotEqual(
            result_obj.get("uuid"),
            openmediavault.getenv('OMV_CONFIGOBJECT_NEW_UUID'),
        )
        # Check whether the new configuration object was stored.
        objs = db.get("conf.system.notification.notification")
        self.assertIsInstance(objs, list)
        self.assertEqual(len(objs), 9)
        # Get the configuration object to validate its properties.
        obj = db.get(
            "conf.system.notification.notification", result_obj.get("uuid")
        )
        self.assertIsInstance(obj, openmediavault.config.Object)
        self.assertEqual(obj.get("id"), "test")
        self.assertFalse(obj.get("enable"))

    def test_set_fail(self):
        # Try to put an object that does not exist.
        db = openmediavault.config.Database()
        obj = openmediavault.config.Object(
            "conf.system.notification.notification"
        )
        obj.set_dict(
            {
                'uuid': '2f6bffd8-f5c2-11e6-9584-17a40dfa0331',
                'id': 'xyz',
                'enable': True,
            }
        )
        self.assertRaises(
            openmediavault.config.database.DatabaseQueryNotFoundException,
            lambda: db.set(obj),
        )

    def test_update_iterable(self):
        self._use_tmp_config_database()
        db = openmediavault.config.Database()
        # Get the configuration object.
        obj = db.get(
            "conf.system.notification.notification",
            "c1cd54af-660d-4311-8e21-2a19420355bb",
        )
        self.assertIsInstance(obj, openmediavault.config.Object)
        self.assertTrue(obj.get("enable"))
        # Modify a property and put the configuration object.
        obj.set("enable", False)
        result_obj = db.set(obj)
        # Validate the returned configuration object.
        self.assertIsInstance(result_obj, openmediavault.config.Object)
        self.assertEqual(
            result_obj.get("uuid"), "c1cd54af-660d-4311-8e21-2a19420355bb"
        )
        self.assertFalse(result_obj.get("enable"))
        # Get the configuration object to validate its properties.
        obj = db.get(
            "conf.system.notification.notification",
            "c1cd54af-660d-4311-8e21-2a19420355bb",
        )
        self.assertIsInstance(obj, openmediavault.config.Object)
        self.assertFalse(obj.get("enable"))

    def test_get_identifiable(self):
        db = openmediavault.config.Database()
        obj = db.get(
            "conf.system.sharedfolder", "339bd101-5744-4017-9392-01a156f15ab9"
        )
        self.assertIsInstance(obj, openmediavault.config.Object)
        self.assertIsInstance(obj.get("privileges.privilege"), list)
        self.assertEqual(obj.get("privileges.privilege.0.perms"), 7)
        self.assertEqual(obj.get("privileges.privilege.1.name"), "test2")


if __name__ == "__main__":
    unittest.main()
