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

/**
 * @ingroup api
 */
class KeyValueFile {
	private $keyNameDelimiter;
	private $keyValueDelimiter;
	private $commentDelimiter;
	private $filename;
	private $keyCaseSensitiv;

	/**
	 * Constructor
	 * @param string $filename Path to the file.
	 * @param string $keyValueDelimiter The key/value delimiter.
	 *   Defaults to '='.
	 * @param string $keyNameDelimiter The delimiter that is used to replace
	 *   white spaces within the key name. Defaults to '_'.
	 * @param string $commentDelimiter The comment line delimiter.
	 *   Defaults to '#'.
	 */
	function __construct($filename, $keyValueDelimiter = "=",
		$keyNameDelimiter = "_", $commentDelimiter = "#") {
		$this->filename = $filename;
		$this->keyNameDelimiter = $keyNameDelimiter;
		$this->keyValueDelimiter = $keyValueDelimiter;
		$this->commentDelimiter = $commentDelimiter;
		$this->keyCaseSensitiv = FALSE;
	}

	final public function setKeyNameDelimiter($char) {
		$this->keyNameDelimiter = $char;
	}

	final public function setKeyValueDelimiter($char) {
		$this->keyValueDelimiter = $char;
	}

	final public function setCommentDelimiter($char) {
		$this->commentDelimiter = $char;
	}

	/**
	 * Specifies whether case-sensitive key matching should be
	 * used.
	 * @param caseSensitive TRUE for case-sensitive matching.
	 * @return void
	 */
	final public function setKeyCaseSensitiv($caseSensitive) {
		$this->keyCaseSensitiv = $caseSensitive;
	}

	/**
	 * Check whether the specified key exists.
	 * @param string $key The name of the key.
	 * @return Returns TRUE if the key exists, otherwise FALSE.
	 */
	final public function exists($key) {
		if (FALSE === $this->keyCaseSensitiv) {
			$key = mb_strtolower($key);
		}
		$dict = new \OMV\Dictionary($this->getAssoc());
		return $dict->exists($key);
	}

	/**
	 * Get the value of the given key.
	 * @param string $key The name of the key.
	 * @param mixed $default The default value if key does not exist.
	 *   Defaults to NULL.
	 * @return The value of the given key.
	 */
	final public function get($key, $default = NULL) {
		if (FALSE === $this->keyCaseSensitiv) {
			$key = mb_strtolower($key);
		}
		$dict = new \OMV\Dictionary($this->getAssoc());
		return $dict->get($key, $default);
	}

	/**
	 * Get the key/value pairs as associative array.
	 * @return The array of key/value pairs.
	 */
	final public function getAssoc() {
		$map = [];
		// Read the file into an array.
		$rows = file($this->filename);
		// Prepare regular expression.
		$regex = sprintf('/^([^%s]+)[%s](.*)$/i', $this->keyValueDelimiter,
			$this->keyValueDelimiter);
		// Process the rows.
		foreach ($rows as $rowk => $rowv) {
			// Skip comment lines.
			if ($this->commentDelimiter == substr(ltrim($rowv), 0, 1)) {
				continue;
			}
			// Extract key/value.
			if (1 !== preg_match($regex, $rowv, $matches)) {
				continue;
			}
			// Harmonize the key.
			$key = trim($matches[1]);
			$key = str_replace(" ", $this->keyNameDelimiter, $key);
			if (FALSE === $this->keyCaseSensitiv) {
				$key = mb_strtolower($key);
			}
			// Add key/value pair.
			$map[$key] = trim($matches[2]);
		}
		return $map;
	}
}
