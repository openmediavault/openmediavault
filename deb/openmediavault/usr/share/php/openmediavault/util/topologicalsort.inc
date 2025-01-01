<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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
 * along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
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
		// Do a simple check to prevent circular dependencies.
		foreach ($deps as $depk => $depv) {
			if (array_key_exists($depv, $this->dependencies)) {
				if (in_array($node, $this->dependencies[$depv]))
					return FALSE;
			}
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
	 * @param bool $ignoreMissing Ignore dependency nodes that do not exist.
	 *   Defaults to TRUE.
	 * @return array|bool Returns the sorted nodes based on the given
	 *   dependencies, otherwise FALSE if the dependencies can not be solved.
	 */
	public function sort($ignoreMissing = TRUE) {
		$sorted = [];
		// Remove dependencies that do not exist.
		if (TRUE === $ignoreMissing) {
			foreach ($this->nodes as $nodek => $nodev) {
				if (array_key_exists($nodev, $this->dependencies))
					continue;
				foreach ($this->dependencies as $depsk => &$depsv) {
					$depsv = array_values(array_diff($depsv, [ $nodev ]));
				}
				unset($this->nodes[$nodek]);
			}
		}
		// Sort the nodes.
		$abort = FALSE;
		while ((FALSE === $abort) && (count($this->nodes) > count($sorted))) {
			$abort = TRUE;
			foreach ($this->dependencies as $node => $deps) {
				$found = TRUE;
				foreach ($deps as $depk => $depv) {
					if (!in_array($depv, $sorted)) {
						$found = FALSE;
						break;
					}
				}
				if (TRUE === $found) {
					$sorted[] = $node;
					unset($this->dependencies[$node]);
					$abort = FALSE;
				}
			}
		}
		// Sucessful?
		if ((TRUE === $abort) && (count($this->nodes) > count($sorted)))
			return FALSE;
		return $sorted;
	}
}
