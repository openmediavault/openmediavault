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
import unittest

import openmediavault.config.object

import openmediavault


class ConfigObjectTestCase(unittest.TestCase):
    def test_constructor(self):
        conf_obj = openmediavault.config.Object("conf.service.ftp.share")

    def test_get_defaults(self):
        conf_obj = openmediavault.config.Object("conf.service.ftp.share")
        defaults = conf_obj.get_defaults()
        self.assertEqual(
            defaults,
            {
                'comment': '',
                'enable': False,
                'uuid': 'fa4b1c66-ef79-11e5-87a0-0002b3a176b4',
                'sharedfolderref': '',
                'extraoptions': '',
            },
        )

    def test_empty_objects(self):
        conf_obj = openmediavault.config.Object("conf.service.rsyncd")
        self.assertIsInstance(conf_obj.get("modules"), dict)
        self.assertIsInstance(conf_obj.get("modules.module"), list)
        self.assertEqual(conf_obj.get("modules.module"), [])

    def test_set_get_1(self):
        conf_obj = openmediavault.config.Object("conf.service.ftp.share")
        conf_obj.set("comment", "test")
        self.assertEqual(conf_obj.get("comment"), "test")

    def test_set_get_2(self):
        conf_obj = openmediavault.config.Object("conf.system.time")
        self.assertEqual(conf_obj.get("timezone"), "Etc/UTC")
        self.assertFalse(conf_obj.get("ntp.enable"))
        self.assertEqual(
            conf_obj.get("ntp.timeservers"),
            "pool.ntp.org,pool1.ntp.org;pool2.ntp.org",
        )
        self.assertEqual(conf_obj.get("ntp.clients"), "")

    def test_set_get_3(self):
        conf_obj = openmediavault.config.Object("conf.system.sharedfolder")
        self.assertIsInstance(conf_obj.get("privileges.privilege"), list)
        self.assertEqual(conf_obj.get("privileges.privilege"), [])

    def test_set_dict(self):
        conf_obj = openmediavault.config.Object("conf.service.ftp.share")
        conf_obj.set_dict({"comment": "test", "enable": True})
        self.assertTrue(conf_obj.get("enable"))
        self.assertEqual(conf_obj.get("comment"), "test")
        self.assertIsNotNone(conf_obj.get("extraoptions"))
        self.assertEqual(conf_obj.get("extraoptions"), "")

    def test_is_empty(self):
        conf_obj = openmediavault.config.Object("conf.service.ftp.share")
        self.assertTrue(conf_obj.is_empty("comment"))

    def test_not_is_empty(self):
        conf_obj = openmediavault.config.Object("conf.service.ftp.share")
        conf_obj.set("comment", "test")
        self.assertFalse(conf_obj.is_empty("comment"))


if __name__ == "__main__":
    unittest.main()
