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

import openmediavault.collectiontools


class DotDictTestCase(unittest.TestCase):
    def _get_dict(self):
        return openmediavault.collectiontools.DotDict(
            {
                'x': 3,
                'a': {'b': {'c': 100}},
                'k': ['u', 'i', 'o'],
                'y': {
                    'z': [
                        {'aa': '1', 'bb': '2', 'cc': '3'},
                        {'aa': '11', 'bb': '22', 'cc': '33'},
                    ]
                },
            }
        )

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
        d['a.b.d'] = {'x': 'y'}
        self.assertEqual(d['a.b.d'], {'x': 'y'})

    def test_5(self):
        d = self._get_dict()
        d['a'] = {'z': 2.3}
        self.assertEqual(d['a'], {'z': 2.3})

    def test_6(self):
        d = openmediavault.collectiontools.DotDict()
        d['a'] = False
        self.assertEqual(d['a'], False)

    def test_7(self):
        d = self._get_dict()
        self.assertEqual(d['y.z.0.bb'], "2")

    def test_8(self):
        d = self._get_dict()
        self.assertEqual(d.y.z[0].bb, "2")

    def test_9(self):
        d = self._get_dict()
        self.assertEqual(d['k.2'], "o")

    def test_10(self):
        d = self._get_dict()
        self.assertEqual(d.k[2], "o")

    def test_in(self):
        d = self._get_dict()
        self.assertTrue("a.b.c" in d)

    def test_in_fail(self):
        d = self._get_dict()
        self.assertFalse("a.x" in d)

    def test_set_1(self):
        d = self._get_dict()
        d.y.z[0].bb = "bb"
        self.assertEqual(d.y.z[0], {'aa': '1', 'bb': 'bb', 'cc': '3'})

    def test_set_2(self):
        d = self._get_dict()
        d["y.z[0].bb"] = "bb"
        self.assertEqual(d["y.z[0]"], {'aa': '1', 'bb': 'bb', 'cc': '3'})

    def test_set_3(self):
        d = self._get_dict()
        d.y.z[0].dd = "dd"
        self.assertEqual(
            d.y.z[0], {'aa': '1', 'bb': '2', 'cc': '3', 'dd': 'dd'}
        )

    def test_set_4(self):
        d = self._get_dict()
        d["y.z[0].dd"] = "dd"
        self.assertEqual(
            d["y.z[0]"], {'aa': '1', 'bb': '2', 'cc': '3', 'dd': 'dd'}
        )

    def test_set_5(self):
        d = openmediavault.collectiontools.DotDict({'jobs': {'job': []}})
        d["jobs.job.0"] = {'acls': 0}
        self.assertEqual(d["jobs.job"], [{'acls': 0}])
        d["jobs.job.0.comment"] = ""
        d["jobs.job[0].enable"] = False
        self.assertEqual(
            d["jobs.job"], [{'acls': 0, 'comment': '', 'enable': False}]
        )

    def test_set_6(self):
        d = openmediavault.collectiontools.DotDict({'modules': {'module': []}})
        self.assertIsInstance(d.modules, object)
        self.assertIsInstance(d['modules.module'], list)
        d['modules.module[0]'] = {
            'enable': False,
            'readonly': True,
            'users': {'user': []},
        }
        self.assertIsInstance(d['modules.module.0.users'], object)
        self.assertIsInstance(d.modules.module[0].users.user, list)
        self.assertEqual(len(d.modules.module[0].users.user), 0)
        d['modules.module.0.users.user.0.name'] = "user01"
        d['modules.module.0.users.user.0.password'] = "test"
        self.assertIsInstance(d['modules.module.0.users'], object)
        self.assertIsInstance(d['modules.module[0].users.user'], list)
        self.assertEqual(len(d.modules.module[0].users.user), 1)
        self.assertIsInstance(d['modules.module.0.users.user[0].name'], str)

    def test_flatten(self):
        d = self._get_dict()
        d_flat = openmediavault.collectiontools.flatten(d)
        self.assertEqual(
            d_flat,
            {
                'x': 3,
                'a.b.c': 100,
                'k.0': 'u',
                'k.1': 'i',
                'k.2': 'o',
                'y.z.0.aa': '1',
                'y.z.0.bb': '2',
                'y.z.0.cc': '3',
                'y.z.1.aa': '11',
                'y.z.1.bb': '22',
                'y.z.1.cc': '33',
            },
        )

    def test_dotcollapseddict(self):
        d = self._get_dict()
        d_flat = openmediavault.collectiontools.DotCollapsedDict(d)
        self.assertEqual(
            d_flat,
            {
                'x': 3,
                'a.b.c': 100,
                'k[0]': 'u',
                'k[1]': 'i',
                'k[2]': 'o',
                'y.z[0].aa': '1',
                'y.z[0].bb': '2',
                'y.z[0].cc': '3',
                'y.z[1].aa': '11',
                'y.z[1].bb': '22',
                'y.z[1].cc': '33',
            },
        )

    def test_find_1(self):
        c = [1, 2, 3, 4]
        res = openmediavault.collectiontools.find(c, lambda i: i >= 2)
        self.assertEqual(res, 2)

    def test_find_2(self):
        c = [{'x': 0}, {'x': 1}, {'x': 2, 'y': 2}, {'x': 3, 'y': 3}]
        res = openmediavault.collectiontools.find(c, lambda d: d['x'] == 2)
        self.assertEqual(res, {'x': 2, 'y': 2})

    def test_find_3(self):
        c = [{'x': 0}, {'x': 1}, {'x': 2, 'y': 2}, {'x': 3, 'y': 3}]
        res = openmediavault.collectiontools.find(c, lambda d: d['x'] > 3)
        self.assertIsNone(res)

    def test_merge_1(self):
        dst = {'a': 1, 'b': {'x': 2, 'y': 3}, 'c': 4, 'd': 0}
        src1 = {'b': {'x': 2, 'y': 4}, 'c': 5}
        src2 = {'d': 7}
        openmediavault.collectiontools.merge(dst, src1, src2)
        self.assertEqual(dst, {'a': 1, 'b': {'x': 2, 'y': 4}, 'c': 5, 'd': 7})


if __name__ == "__main__":
    unittest.main()
