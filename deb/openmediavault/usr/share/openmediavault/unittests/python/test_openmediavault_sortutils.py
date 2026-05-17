# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2026 Volker Theile
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
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.
import unittest

import openmediavault.sortutils


class SortUtilsTestCase(unittest.TestCase):
    def test_node_info_hash_equals_node_hash(self):
        node_info = openmediavault.sortutils.NodeInfo('alpha')
        self.assertEqual(hash(node_info), hash('alpha'))

    def test_node_info_equality_based_on_node_value(self):
        left = openmediavault.sortutils.NodeInfo('node')
        right = openmediavault.sortutils.NodeInfo('node')
        other = openmediavault.sortutils.NodeInfo('other')
        self.assertEqual(left, right)
        self.assertNotEqual(left, other)

    def test_node_info_less_than_compares_node_value(self):
        left = openmediavault.sortutils.NodeInfo(1)
        right = openmediavault.sortutils.NodeInfo(2)
        self.assertTrue(left < right)

    def test_get_node_info_returns_same_instance_for_same_node(self):
        ts = openmediavault.sortutils.TopologicalSorter()
        first = ts._get_node_info('x')
        second = ts._get_node_info('x')
        self.assertIs(first, second)

    def test_add_updates_successors_and_predecessor_count(self):
        ts = openmediavault.sortutils.TopologicalSorter()
        ts.add('a', ['b', 'c'])
        a_ni = ts._get_node_info('a')
        b_ni = ts._get_node_info('b')
        c_ni = ts._get_node_info('c')
        self.assertEqual([ni.node for ni in a_ni.successors], ['b', 'c'])
        self.assertEqual(b_ni.num_predecessors, 1)
        self.assertEqual(c_ni.num_predecessors, 1)

    def test_add_predecessors_updates_successors_and_predecessor_count(self):
        ts = openmediavault.sortutils.TopologicalSorter()
        ts.add_predecessors('v', ['u1', 'u2'])
        v_ni = ts._get_node_info('v')
        u1_ni = ts._get_node_info('u1')
        u2_ni = ts._get_node_info('u2')
        self.assertEqual(v_ni.num_predecessors, 2)
        self.assertEqual([ni.node for ni in u1_ni.successors], ['v'])
        self.assertEqual([ni.node for ni in u2_ni.successors], ['v'])

    def test_empty_graph_is_acyclic_and_sorts_to_empty_list(self):
        ts = openmediavault.sortutils.TopologicalSorter()
        self.assertTrue(ts.is_acyclic())
        self.assertEqual(ts.sort(), [])

    def test_sort_order_simple_predecessor_graph(self):
        ts = openmediavault.sortutils.TopologicalSorter()
        ts.add_predecessors(1, [0])
        ts.add_predecessors(3, [2, 1])
        self.assertEqual(ts.sort(), [0, 2, 1, 3])

    def test_sort_order_service_graph_using_add(self):
        ts = openmediavault.sortutils.TopologicalSorter()
        ts.add('email', ['monit', 'cron'])
        ts.add('monit', ['collectd', 'rrdcached'])
        ts.add('rrdcached', ['collectd'])
        ts.add('hostname', ['email'])
        ts.add('hosts', ['email'])
        self.assertEqual(
            ts.sort(),
            ['hostname', 'hosts', 'email', 'monit',
                'cron', 'rrdcached', 'collectd'],
        )

    def test_sort_order_service_graph_using_predecessors(self):
        ts = openmediavault.sortutils.TopologicalSorter()
        ts.add_predecessors('collectd', ['rrdcached', 'monit'])
        ts.add_predecessors('rrdcached', ['monit'])
        ts.add_predecessors('monit', ['email'])
        ts.add_predecessors('cron', ['email'])
        ts.add_predecessors('email', ['hostname', 'hosts'])
        self.assertEqual(
            ts.sort(),
            ['hostname', 'hosts', 'email', 'monit',
                'cron', 'rrdcached', 'collectd'],
        )

    def test_sort_order_complex_integer_graph(self):
        ts = openmediavault.sortutils.TopologicalSorter()
        ts.add(0, [2, 5])
        ts.add(1, [3, 6])
        ts.add(2, [4])
        ts.add(3, [5])
        ts.add(5, [2, 4])
        ts.add(6, [2])
        self.assertEqual(ts.sort(), [0, 1, 3, 6, 5, 2, 4])

    def test_sort_can_be_called_multiple_times(self):
        ts = openmediavault.sortutils.TopologicalSorter()
        ts.add(0, [2, 5])
        ts.add(1, [3, 6])
        ts.add(2, [4])
        ts.add(3, [5])
        ts.add(5, [2, 4])
        ts.add(6, [2])
        self.assertEqual(ts.sort(), [0, 1, 3, 6, 5, 2, 4])
        self.assertEqual(ts.sort(), [0, 1, 3, 6, 5, 2, 4])

    def test_cycle_detection(self):
        ts = openmediavault.sortutils.TopologicalSorter()
        ts.add(0, [2, 5])
        ts.add(1, [3, 6])
        ts.add(2, [4])
        ts.add(3, [5])
        ts.add(5, [2, 4])
        ts.add(6, [2])
        ts.add(4, [3])
        self.assertFalse(ts.is_acyclic())
        self.assertRaises(openmediavault.sortutils.CycleError, ts.sort)


if __name__ == '__main__':
    unittest.main()
