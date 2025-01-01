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
namespace OMV;

require_once("openmediavault/functions.inc");

/**
 * @ingroup api
 * Represents a collection of keys and values.
 */
class Dictionary {
	private $data;

	/**
	 * Constructor.
	 * @param data The associative array containing the data.
	 */
	public function __construct(array $data = []) {
		$this->data = $data;
	}

	/**
	 * Check whether the given key is valid.
	 * @param key The key in dot notation, e.g. 'a.b.c' or 'a.b.0.c.1'
	 * @return Returns TRUE if the key is valid, otherwise FALSE.
	 */
	protected function isValidKey($key) {
		$regex = '/^((_?[a-z]+[a-z0-9_-]*|[0-9]+)\.)*(_?[a-z]+[a-z0-9_-]*|'.
		  '[0-9]+)$/i';
		return (1 === preg_match($regex, $key));
	}

	/**
	 * Assert that the given key is valid.
	 * @param key The key in dot notation, e.g. 'a.b.c'.
	 * @return void
	 * @throw \OMV\AssertException
	 */
	protected function assertValidKey($key) {
		if (FALSE === $this->isValidKey($key)) {
			$message = sprintf("The key '%s' is invalid.", $key);
			throw new \OMV\AssertException($message);
		}
	}

	/**
	 * Check whether the specified key exists.
	 * @param key The key in dot notation, e.g. 'a.b.c'.
	 * @return Returns TRUE if the key exists, otherwise FALSE.
	 */
	public function exists($key) {
		$this->assertValidKey($key);
		$node = $this->data;
		$parts = explode(".", $key);
		foreach ($parts as $partk => $partv) {
			if (isset($node[$partv]))
				$node = $node[$partv];
			else
				return FALSE;
		}
		return TRUE;
	}

	/**
	 * Assert that the specified key exists.
	 * @param key The key in dot notation, e.g. 'a.b.c'.
	 * @return void
	 * @throw \OMV\AssertException
	 */
	public function assertExists($key) {
		if (FALSE === $this->exists($key)) {
			$message = sprintf("The key '%s' does not exist.", $key);
			throw new \OMV\AssertException($message);
		}
	}

	/**
	 * Get the whole data.
	 * @return The whole data.
	 */
	public function getData() {
		return $this->data;
	}

	/**
	 * Get the value for a key.
	 * @param key The key in dot notation, e.g. 'a.b.c'.
	 * @param default The optional default value.
	 * @return The requested value.
	 */
	 public function get($key, $default = NULL) {
		$this->assertValidKey($key);
		$node = $this->data;
		$parts = explode(".", $key);
		foreach ($parts as $partk => $partv) {
			if (!is_array($node))
				return $default;
			if (!isset($node[$partv]))
				return $default;
			$node = $node[$partv];
		}
		return $node === NULL ? $default : $node;
	}

	/**
	 * Set a value for a key.
	 * @param key The key in dot notation, e.g. 'a.b.c'.
	 * @param value The value of the given key.
	 * @return void
	 */
	public function set($key, $value) {
		$this->assertValidKey($key);
		$node = &$this->data;
		$parts = explode(".", $key);
		if (1 == count($parts)) {
			$node[$key] = $value;
			return;
		}
		$lastPart = array_pop($parts);
		foreach ($parts as $partk => $partv) {
			if (!isset($node[$partv]))
				$node[$partv] = [];
			if (!is_array($node[$partv])) {
				$message = sprintf("Failed to index key '%s' of '%s' because ".
				  "it has no children.", $partv, $key);
				throw new \RuntimeException($message);
			}
			$node = &$node[$partv];
		}
		$node[$lastPart] = $value;
	}

	/**
	 * Copy a key.
	 * @param srcKey The source key in dot notation, e.g. 'a.b.c'.
	 * @param destKey The destination key in dot notation, e.g. 'a.b.c'.
	 * @return void
	 */
	public function copy($srcKey, $destKey) {
		$this->assertExists($srcKey);
		$value = $this->get($srcKey);
		$this->set($destKey, $value);
	}

	/**
	 * Remove a key.
	 * @param key The key in dot notation, e.g. 'a.b.c'.
	 * @return void
	 */
	public function remove($key) {
		$this->assertExists($key);
		$node = &$this->data;
		$parts = explode(".", $key);
		if (1 == count($parts)) {
			unset($node[$key]);
			return;
		}
		$lastPart = array_pop($parts);
		foreach ($parts as $partk => $partv) {
			if (!isset($node[$partv]))
				return;
			$node = &$node[$partv];
		}
		unset($node[$lastPart]);
	}

	/**
	 * Convert the data to JSON.
	 * @return JSON serialized data.
	 */
	public function toJson() {
		return json_encode($this->getData());
	}

	/**
	 * Import JSON data.
	 * @param json The JSON string.
	 * @return void
	 */
	public function fromJson($json = "") {
		if (NULL === ($data = json_decode_safe($json, TRUE))) {
			throw new \InvalidArgumentException(sprintf(
			  "Failed to decode data: %s",
			  json_last_error_msg()));
		}
		$this->data = $data;
	}
}
