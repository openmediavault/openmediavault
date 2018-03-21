<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2018 Volker Theile
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
namespace OMV\Json;

require_once("openmediavault/functions.inc");

/**
 * Read and write JSON encoded content to a file. The file is locked when
 * it is opened and the lock is removed when it is closed.
 * @ingroup api
 */
class File {
	private $filename = "";
	private $fh = NULL;

	/**
	 * Constructor
	 * @param filename The name of the file.
	 */
	function __construct($filename) {
		$this->filename = $filename;
	}

	/**
	 * Destructor
	 */
	function __destruct() {
		// Always ensure the file is unlocked.
		if (!is_null($this->fh))
        	flock($this->fh, LOCK_UN);
    }

	/**
	 * Checks whether the file exists.
	 * @return Returns TRUE if the file exists, FALSE otherwise.
	 */
	public function exists() {
		clearstatcache(TRUE, $this->filename);
		return file_exists($this->filename);
	}

	/**
	 * Open the file.
	 * @param accessMode The parameter specifies the type of access you
	 *   require to the stream. Defaults to 'w+'.
	 * @param lockMode The lock mode. Defaults to LOCK_EX.
	 * @return The file handle.
	 * @throw \OMV\Exception
	 */
	public function open($accessMode = "w+", $lockMode = LOCK_EX) {
		if (FALSE === ($this->fh = fopen($this->filename, $accessMode))) {
			throw new \OMV\Exception(
			  "Failed to open file (filename=%s, mode=%s).",
			  $this->filename, $accessMode);
		}
		if (FALSE === flock($this->fh, $lockMode, $wouldBlock)) {
			throw new \OMV\Exception(
			  "Failed to lock file (filename=%s, mode=%d)%s.",
			  $this->filename, $lockMode,
			  $wouldBlock ? ": File is already locked" : "");
		}
		return $this->fh;
	}

	/**
	 * Is the file opened?
	 * @return TRUE if the file is opened, otherwise FALSE.
	 */
	public function isOpen() {
		return !is_null($this->fh);
	}

	/**
	 * Is the file empty?
	 * @return TRUE if the file is empty, otherwise FALSE.
	 * @throw \OMV\Exception
	 */
	public function isEmpty() {
		// Get the information about the file.
		if (FALSE === ($fstats = fstat($this->fh))) {
			throw new \OMV\Exception("Failed to get file stats from '%s'.",
			  $this->filename);
		}
		return (0 == $fstats['size']);
	}

	/**
	 * Validate the JSON data.
	 * @param data The JSON data to validate.
	 * @return TRUE if the JSON data is valid, otherwise FALSE.
	 */
	protected function validate($data) {
		// An empty JSON file must contain '[]' or '{}' at least.
		if (TRUE === empty($data))
			return FALSE;
		// Validate the JSON data.
		return is_json($data);
	}

	/**
	 * Check if the JSON data is valid.
	 * @return TRUE if the JSON data is valid, otherwise FALSE.
	 */
	public function isValid() {
		$contents = $this->getContents();
		return $this->validate($contents);
	}

	/**
	 * Close the file.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function close() {
		if (FALSE === $this->isOpen())
			return;
		if (FALSE === flock($this->fh, LOCK_UN)) {
			throw new \OMV\Exception("Failed to unlock file '%s'.",
			  $this->filename);
		}
		if (FALSE === fclose($this->fh)) {
			throw new \OMV\Exception("Failed to close file '%s'.",
			  $this->filename);
		}
		$this->fh = NULL;
	}

	/**
	 * Write the given content as JSON encoded string to the file.
	 * The file will be truncated to zero length before the content
	 * is written.
	 * @param data The data to write.
	 * @return Returns the JSON representation of the data which was
	 *   written to the file.
	 * @throw \OMV\Exception
	 */
	public function write($data) {
		if (FALSE === rewind($this->fh)) {
			throw new \OMV\Exception("Failed to rewind file '%s'.",
			  $this->filename);
		}
		// Get the JSON representation of the content.
		if (FALSE === ($data = json_encode_safe($data, JSON_PRETTY_PRINT))) {
			throw new \OMV\Exception(
			  "Failed to encode content in file '%s': %s",
			  $this->filename, json_last_error_msg());
		}
		// Empty the whole file.
		$this->truncate(0);
		// Now write the new content.
		if (FALSE === fwrite($this->fh, $data)) {
			throw new \OMV\Exception("Failed to write file '%s'.",
			  $this->filename);
		}
		if (FALSE === fflush($this->fh)) {
			throw new \OMV\Exception("Failed to flush file '%s'.",
			  $this->filename);
		}
		return $data;
	}

	/**
	 * Reads entire file into a string.
	 * @return The method returns the read data.
	 * @throw \OMV\Exception
	 */
	public function getContents() {
		// Seek to the beginning of the file.
		if (FALSE === rewind($this->fh)) {
			throw new \OMV\Exception("Failed to rewind file '%s'.",
			  $this->filename);
		}
		// Get the information about the file.
		if (FALSE === ($fstats = fstat($this->fh))) {
			throw new \OMV\Exception("Failed to get file stats from '%s'.",
			  $this->filename);
		}
		// Get the whole file contents.
		if (FALSE === ($contents = fread($this->fh, $fstats['size']))) {
			$error = error_get_last();
			throw new \OMV\Exception("Failed to read file '%s' (size=%d): %s",
			  $this->filename, $fstats['size'], $error['message']);
		}
		return $contents;
	}

	/**
	 * Read the JSON encoded data from the file.
	 * @param decode Set to TRUE to decode the JSON content. Defaults to TRUE.
	 * @return The content read from the file. If \em decode was set to TRUE
	 *   the decoded content is returned.
	 * @throw \OMV\Exception
	 */
	public function read($decode = TRUE, $assoc = TRUE) {
		// Read the whole file contents.
		$contents = $this->getContents();
		// Validate the JSON data.
		if (FALSE === $this->validate($contents)) {
			throw new \OMV\Exception("File '%s' contains invalid JSON: %s",
			  $this->filename, json_last_error_msg());
		}
		// Decode the content if requested.
		if (TRUE === $decode) {
			if (NULL === ($contents = json_decode_safe($contents, $assoc))) {
				throw new \OMV\Exception(
				  "Failed to decode content in file '%s': %s",
				  $this->filename, json_last_error_msg());
			}
		}
		return $contents;
	}

	/**
	 * Truncates the file to a given length.
	 * @param size The size to truncate to.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function truncate($size) {
		if (FALSE === ftruncate($this->fh, $size)) {
			throw new \OMV\Exception(
			  "Failed to truncate file (filename=%s, size=%d).",
			  $this->filename, $size);
		}
	}

	/**
	 * Unlink the given file.
	 * @return Returns TRUE on success or FALSE on failure.
	 */
	public function unlink() {
		if ($this->isOpen())
			$this->close();
		$result = @unlink($this->filename);
		clearstatcache(TRUE, $this->filename);
		return $result;
	}
}
