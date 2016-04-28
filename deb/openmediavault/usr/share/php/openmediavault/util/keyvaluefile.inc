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
namespace OMV\Util;

/**
 * @ingroup api
 */
class KeyValueFile {
	private $delimiter;
	private $comment;
	private $filename;

	function __construct($filename, $delimiter = "=", $comment="#") {
		$this->filename = $filename;
		$this->delimiter = $delimiter;
		$this->comment = $comment;
	}

	final public function getAssoc() {
		$map = [];
		// Read the file into an array.
		$rows = file($this->filename);
		// Prepare regular expression.
		$regex = sprintf('/^(\S+)[%s]+(\S+)$/i', $this->delimiter);
		// Process the rows.
		foreach ($rows as $rowk => $rowv) {
			// Skip comments.
			if ($this->comment == substr($rowv, 0, 1))
				continue;
			// Extract key/value.
			if (1 !== preg_match($regex, $rowv, $matches))
				continue;
			$map[$matches[1]] = $matches[2];
		}
		return $map;
	}
}
