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
from collections import deque
from functools import total_ordering
from typing import Deque, Dict, Generic, List, Set, TypeVar

T = TypeVar('T')


class CycleError(ValueError):
    pass


@total_ordering
class NodeInfo(Generic[T]):
    __slots__ = 'node', 'num_predecessors', 'successors'

    def __init__(self, node):
        self.node: T = node
        self.num_predecessors: int = 0
        self.successors: List[NodeInfo[T]] = []

    def __hash__(self):
        return hash(self.node)

    def __eq__(self, other):
        return self.node == other.node

    def __lt__(self, other) -> bool:
        return self.node < other.node


class TopologicalSorter(Generic[T]):
    def __init__(self):
        self._nodes: Dict[T, NodeInfo[T]] = {}

    def _get_node_info(self, node: T) -> NodeInfo[T]:
        return self._nodes.setdefault(node, NodeInfo(node))

    def add(self, u: T, successors: List[T]) -> None:
        u_ni = self._get_node_info(u)
        for v in successors:
            v_ni = self._get_node_info(v)
            v_ni.num_predecessors += 1
            u_ni.successors.append(v_ni)

    def add_predecessors(self, v: T, predecessors: List[T]) -> None:
        v_ni = self._get_node_info(v)
        v_ni.num_predecessors += len(predecessors)
        for u in predecessors:
            u_ni = self._get_node_info(u)
            u_ni.successors.append(v_ni)

    def sort(self) -> List[T]:
        result: List[T] = []

        if not self.is_acyclic():
            raise CycleError

        ready_nodes: Deque[NodeInfo] = deque(
            [n for n in self._nodes.values() if n.num_predecessors == 0])
        while ready_nodes:
            current_node: NodeInfo = ready_nodes.popleft()
            for v in current_node.successors:
                v.num_predecessors -= 1
                if v.num_predecessors == 0:
                    ready_nodes.append(v)
            result.append(current_node.node)
        return result

    def is_acyclic(self):
        visited: Set[NodeInfo[T]] = set()
        stack: List[NodeInfo[T]] = []

        def find_cycle(u: NodeInfo[T]) -> bool:
            for v in u.successors:
                if v in stack:
                    # We found a cycle.
                    return True
                visited.add(v)
                stack.append(v)
                if find_cycle(v):
                    return True
                stack.pop()

        for _, node in self._nodes.items():
            if node in visited:
                continue
            if find_cycle(node):
                return False

        return True


if __name__ == "__main__":
    ts: TopologicalSorter[int] = TopologicalSorter()
    ts.add_predecessors(1, [0])
    ts.add_predecessors(3, [2, 1])
    print("True" if ts.is_acyclic() else "False")
    print(ts.sort())  # [0, 2, 1, 3]

    ts: TopologicalSorter[str] = TopologicalSorter()
    ts.add_predecessors('D', ['B', 'C'])
    ts.add_predecessors('C', ['A'])
    ts.add_predecessors('B', ['A'])
    print(ts.sort())  # ['A', 'C', 'B', 'D']

    ts: TopologicalSorter[int] = TopologicalSorter()
    ts.add_predecessors(0, [4, 3, 1])
    ts.add_predecessors(2, [1, 3])
    ts.add_predecessors(3, [1, 4])
    print(ts.sort())  # [4, 1, 3, 0, 2]

    ts: TopologicalSorter[str] = TopologicalSorter()
    ts.add('email', ['monit', 'cron'])
    ts.add('monit', ['collectd', 'rrdcached'])
    ts.add('rrdcached', ['collectd'])
    ts.add('hostname', ['email'])
    ts.add('hosts', ['email'])
    # ['hostname', 'hosts', 'email', 'monit', 'cron', 'rrdcached', 'collectd']
    print(ts.sort())

    ts: TopologicalSorter[str] = TopologicalSorter()
    ts.add_predecessors('collectd', ['rrdcached', 'monit'])
    ts.add_predecessors('rrdcached', ['monit'])
    ts.add_predecessors('monit', ['email'])
    ts.add_predecessors('cron', ['email'])
    ts.add_predecessors('email', ['hostname', 'hosts'])
    # ['hostname', 'hosts', 'email', 'monit', 'cron', 'rrdcached', 'collectd']
    print(ts.sort())

    ts: TopologicalSorter[int] = TopologicalSorter()
    ts.add(5, [2, 0])
    ts.add(4, [0, 1])
    ts.add(2, [3])
    ts.add(3, [1])
    print(ts.sort())  # [5, 4, 2, 0, 3, 1]

    ts: TopologicalSorter[int] = TopologicalSorter()
    ts.add(0, [2, 5])
    ts.add(1, [3, 6])
    ts.add(2, [4])
    ts.add(3, [5])
    ts.add(5, [2, 4])
    ts.add(6, [2])
    print(ts.sort())  # [0, 1, 3, 6, 5, 2, 4]

    ts: TopologicalSorter[int] = TopologicalSorter()
    ts.add_predecessors(1, [0])
    ts.add_predecessors(3, [2, 1])
    ts.add(3, [0])  # Cycle
    print("True" if ts.is_acyclic() else "False")

    ts: TopologicalSorter[int] = TopologicalSorter()
    ts.add(0, [2, 5])
    ts.add(1, [3, 6])
    ts.add(2, [4])
    ts.add(3, [5])
    ts.add(5, [2, 4])
    ts.add(6, [2])
    ts.add(4, [3])  # Cycle
    print("True" if ts.is_acyclic() else "False")
