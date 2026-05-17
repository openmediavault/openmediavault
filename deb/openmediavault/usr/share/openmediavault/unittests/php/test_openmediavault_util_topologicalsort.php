#!/usr/bin/phpunit -c/etc/openmediavault
<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2026 Volker Theile
 *
 * OpenMediaVault is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * OpenMediaVault is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.
 */
require_once("openmediavault/autoloader.inc");
require_once("openmediavault/globals.inc");

class test_openmediavault_util_topologicalsort extends \PHPUnit\Framework\TestCase {
	public function test_sort_with_acyclic_graph() {
		$tsort = new \OMV\Util\TopologicalSort();
		$tsort->add("node1", []);
		$tsort->add("node2", "node1");
		$tsort->add("node4", ["node2"]);
		$tsort->add("node5", ["node4", "node3"]);

		$this->assertTrue($tsort->isAcyclic());
		$this->assertEquals(["node1", "node2", "node4", "node5"],
			$tsort->sort(TRUE));
	}

	public function test_sort_fails_when_missing_dependencies_are_not_ignored() {
		$tsort = new \OMV\Util\TopologicalSort();
		$tsort->add("node2", "node1");

		$this->assertFalse($tsort->sort(FALSE));
	}

	public function test_sort_can_be_called_multiple_times() {
		$tsort = new \OMV\Util\TopologicalSort();
		$tsort->add("node1", []);
		$tsort->add("node2", ["node1"]);

		$this->assertEquals(["node1", "node2"], $tsort->sort(TRUE));
		$this->assertEquals(["node1", "node2"], $tsort->sort(TRUE));
	}

	public function test_is_acyclic_returns_false_on_cycles() {
		$tsort = new \OMV\Util\TopologicalSort();
		$tsort->add("a", ["b"]);
		$tsort->add("b", ["c"]);
		$tsort->add("c", ["a"]);

		$this->assertFalse($tsort->isAcyclic());
		$this->assertFalse($tsort->sort(FALSE));
	}

	public function test_clean_removes_all_nodes() {
		$tsort = new \OMV\Util\TopologicalSort();
		$tsort->add("node1", []);
		$tsort->add("node2", ["node1"]);
		$tsort->clean();

		$this->assertTrue($tsort->isAcyclic());
		$this->assertEquals([], $tsort->sort(TRUE));
	}

	public function test_sort_order_for_independent_nodes_follows_add_order() {
		$tsort = new \OMV\Util\TopologicalSort();
		$tsort->add("node2", []);
		$tsort->add("node1", []);
		$tsort->add("node3", []);

		$this->assertEquals(["node2", "node1", "node3"],
			$tsort->sort(TRUE));
	}

	public function test_sort_order_for_branching_graph_is_deterministic() {
		$tsort = new \OMV\Util\TopologicalSort();
		$tsort->add("prepare", []);
		$tsort->add("compile", ["prepare"]);
		$tsort->add("lint", ["prepare"]);
		$tsort->add("package", ["compile", "lint"]);

		$this->assertEquals(["prepare", "compile", "lint", "package"],
			$tsort->sort(TRUE));
	}

	public function test_sort_order_with_reverse_addition_still_resolves_chain() {
		$tsort = new \OMV\Util\TopologicalSort();
		$tsort->add("c", ["b"]);
		$tsort->add("b", ["a"]);
		$tsort->add("a", []);

		$this->assertEquals(["a", "b", "c"], $tsort->sort(TRUE));
	}

	public function test_sort_order_service_dependency_graph() {
		$tsort = new \OMV\Util\TopologicalSort();
		$tsort->add("collectd", ["rrdcached", "monit"]);
		$tsort->add("rrdcached", ["monit"]);
		$tsort->add("monit", ["email"]);
		$tsort->add("cron", ["email"]);
		$tsort->add("email", ["hostname", "hosts"]);
		$tsort->add("hostname", []);
		$tsort->add("hosts", []);

		$this->assertEquals([
			"hostname", "hosts", "email", "monit", "cron", "rrdcached",
			"collectd"
		], $tsort->sort(TRUE));
	}

	public function test_sort_order_complex_integer_graph() {
		$tsort = new \OMV\Util\TopologicalSort();
		$tsort->add(0, []);
		$tsort->add(1, []);
		$tsort->add(2, [0, 5, 6]);
		$tsort->add(3, [1]);
		$tsort->add(4, [2, 5]);
		$tsort->add(6, [1]);
		$tsort->add(5, [0, 3]);

		$this->assertEquals([0, 1, 3, 6, 5, 2, 4],
			$tsort->sort(TRUE));
	}
}
