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
namespace OMV\System;

/**
 * Creates a temporary file. The file will be unlinked automatically
 * on object destruction.
 * @ingroup api
 */
class TmpFile {
	private $autoUnlink;
	private $filename;

	/**
	 * Constructor
	 * @param string prefix The prefix of the generated temporary filename.
	 *   Defaults to empty string.
	 * @param bool autoUnlink If set to FALSE, the file will not be unlinked
	 *   on object destruction. Defaults to TRUE.
	 */
	function __construct(string $prefix = "", bool $autoUnlink = TRUE) {
		$this->autoUnlink = $autoUnlink;
		$this->filename = tempnam(sys_get_temp_dir(), $prefix);
	}

	/**
	 * Destructor
	 */
	function __destruct() {
		if ($this->autoUnlink) {
			$this->unlink();
		}
	}

	/**
	 * @return string The full file name.
	 */
	public function getFileName(): string {
		return $this->filename;
	}

	/**
	 * Write data to the temporary file.
	 * @param string data The data to write.
	 * @return mixed This function returns the number of bytes that
	 *   were written to the file, or FALSE on failure.
	 */
	public function write($data) {
		return file_put_contents($this->getFileName(), $data);
	}

	/**
	 * Reads the entire file into a string.
	 * @return mixed The function returns the read data as a string
	 *   or FALSE on failure.
	 */
	public function read() {
		return file_get_contents($this->getFileName());
	}

	/**
	 * Unlink the temporary file.
	 * @return bool Returns TRUE on success or FALSE on failure.
	 */
	public function unlink(): bool {
		return unlink($this->getFileName());
	}
}
