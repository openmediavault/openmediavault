<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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
	public function __construct(array $data = array()) {
        $this->data = $data;
    }

	/**
	 * Check whether the given key is valid.
	 * @param key The key in dot notation, e.g. 'a.b.c' or 'a.b.0.c.1'
	 * @return Returns TRUE if the key is valid, otherwise FALSE.
	 */
	protected function isValidKey($key) {
		$regex = '/^(([a-z]+[a-z0-9_-]*|[0-9]+)\.)*([a-z]+[a-z0-9_-]*|'.
		  '[0-9]+)$/i';
        return preg_match($regex, $key);
    }

	/**
	 * Assert that the given key is valid.
	 * @param key The key in dot notation, e.g. 'a.b.c'.
	 * @return None.
	 * @throw \InvalidArgumentException
	 */
	protected function assertValidKey($key) {
		if (!$this->isValidKey($key)) {
			$message = sprintf("The key '%s' is invalid.");
            throw new \InvalidArgumentException($message);
        }
	}

	/**
	 * Check whether the specified key exists.
	 * @param key The key in dot notation, e.g. 'a.b.c'.
	 * @return Returns TRUE if the key exists, otherwise FALSE.
	 */
	public function exists($key) {
		$this->assertValidKey($key);
		// ToDo
	}

	/**
	 * Get the value for a key.
     * @param key The key in dot notation, e.g. 'a.b.c'.
     * @param default The optional default value.
     * @return The requested value.
     */
    public function get($key, $default = null) {
		$this->assertValidKey($key);
		// ToDo
	}

	/**
	 * Get the whole data.
	 * @return The whole data.
	 */
	public function getData() {
		return $this->data;
	}

	/**
	 * Set a value for a key.
     * @param key The key in dot notation, e.g. 'a.b.c'.
     * @param value The value of the given key.
     * @return The value.
     */
    public function set($key, $value) {
		$this->assertValidKey($key);
		// ToDo
	}

	/**
	 * Remove a key.
     * @param key The key in dot notation, e.g. 'a.b.c'.
     * @return None.
     */
	public function remove($key) {
		$this->assertValidKey($key);
		// ToDo
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
     * @return None.
     */
    public function fromJson($json = "") {
        if (NULL === ($data = json_decode_safe($json, TRUE))) {
			throw new \InvalidArgumentException("Failed to decode data: %s",
			  json_last_error_msg());
		}
        $this->data = $data;
    }
}
