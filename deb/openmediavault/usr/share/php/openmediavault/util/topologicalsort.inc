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
namespace OMV\Util;

require_once("openmediavault/functions.inc");

/**
 * Sorts a series of dependencies in linear order (topological sort).
 * @ingroup api
 * @code
 * $tsort = new \OMV\Util\TopologicalSort;
 * $tsort->add("node1", array());
 * $tsort->add("node2", "node1");
 * $tsort->add("node4", array("node2"));
 * $tsort->add("node5", array("node4", "node3"));
 * print_r($tsort->sort(TRUE));
 *
 * The result will be:
 * Array
 * (
 *     [0] => node1
 *     [1] => node2
 *     [2] => node4
 *     [3] => node5
 * )
 * @endcode
 */
class TopologicalSort {
	private $nodes;
	private $dependencies;

	function __construct() {
		$this->clean();
	}

	/**
	 * Cleanup the internal structures.
	 */
	public function clean() {
		$this->nodes = [];
		$this->dependencies = [];
	}

	/**
	 * Add a node and its dependencies.
	 * @param string $node The node.
	 * @param array|string $deps The node dependencies. This can be an array
	 *   or a string. Defaults to an empty array.
	 * @return bool TRUE if successful, otherwise FALSE.
	 */
	public function add($node, $deps = []) {
		if (!is_array($deps)) {
			$deps = [ $deps ];
		}
		// Add node to internal structures.
		$this->nodes[] = $node;
		foreach ($deps as $depk => $depv) {
			$this->nodes[] = $depv;
		}
		$this->nodes = array_unique($this->nodes);
		$this->dependencies[$node] = $deps;
		return TRUE;
	}

	/**
	 * Check whether the dependency graph is acyclic.
	 * Uses a classic DFS with three node colours:
	 *   0 = white (not yet visited)
	 *   1 = gray  (currently on the recursion stack)
	 *   2 = black (fully explored, no cycle found below)
	 * @return bool TRUE if the graph contains no cycles, otherwise FALSE.
	 */
	public function isAcyclic(): bool {
		$color = array_fill_keys($this->nodes, 0);
		$dfs = function(string $u) use (&$dfs, &$color): bool {
			$color[$u] = 1; // mark as in-progress
			foreach ($this->dependencies[$u] ?? [] as $dep) {
				if (!array_key_exists($dep, $color)) continue;
				if ($color[$dep] === 1) return TRUE; // back edge — cycle detected
				if ($color[$dep] === 0 && $dfs($dep)) return TRUE;
			}
			$color[$u] = 2; // mark as fully explored
			return FALSE;
		};

		foreach ($this->nodes as $node) {
			if ($color[$node] === 0 && $dfs($node)) {
				return FALSE;
			}
		}

		return TRUE;
	}

	/**
	 * @param bool $ignoreMissing Ignore dependency nodes that do not exist.
	 *   Defaults to TRUE.
	 * @return array|bool Returns the sorted nodes based on the given
	 *   dependencies, otherwise FALSE if the dependencies can not be solved.
	 */
	public function sort($ignoreMissing = TRUE) {
		// Work with local copies so that sort() can be called multiple times
		// without corrupting the internal graph state.
		$nodes = $this->nodes;
		$dependencies = $this->dependencies;

		// Remove dependencies that do not exist.
		if (TRUE === $ignoreMissing) {
			foreach ($nodes as $nodek => $nodev) {
				if (array_key_exists($nodev, $dependencies))
					continue;
				foreach ($dependencies as $depsk => &$depsv) {
					$depsv = array_values(array_diff($depsv, [ $nodev ]));
				}
				unset($depsv);
				unset($nodes[$nodek]);
			}
		}

		// Sort the nodes.
		$sorted = [];
		$abort = FALSE;
		while ((FALSE === $abort) && (count($nodes) > count($sorted))) {
			$abort = TRUE;
			foreach ($dependencies as $node => $deps) {
				$found = TRUE;
				foreach ($deps as $depk => $depv) {
					if (!in_array($depv, $sorted)) {
						$found = FALSE;
						break;
					}
				}
				if (TRUE === $found) {
					$sorted[] = $node;
					unset($dependencies[$node]);
					$abort = FALSE;
				}
			}
		}
		if ((TRUE === $abort) && (count($nodes) > count($sorted)))
			return FALSE;
		return $sorted;
	}
}
