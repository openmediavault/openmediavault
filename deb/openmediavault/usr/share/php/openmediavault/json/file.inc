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
namespace OMV\Json;

require_once("openmediavault/functions.inc");

/**
 * Read and write JSON encoded content to a file. The file is locked when
 * it is opened and the lock is removed when it is closed.
 * @ingroup api
 */
class File extends \OMV\BaseObject {
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
		return file_exists($this->filename);
	}

	/**
	 * Open the file.
	 * @param accessMode The parameter specifies the type of access you
	 *   require to the stream. Defaults to 'w+'.
	 * @param lockMode The lock mode. Defaults to LOCK_EX.
	 * @return The file handle.
	 * @throw E_MISC_FAILURE
	 */
	public function open($accessMode = "w+", $lockMode = LOCK_EX) {
		if (FALSE === ($this->fh = fopen($this->filename, $accessMode))) {
			throw new \OMV\Exception(
			  "Failed to open file (filename=%s, mode=%s).",
			  $this->filename, $accessMode);
		}
		if (FALSE === flock($this->fh, $lockMode)) {
			throw new \OMV\Exception(
			  "Failed to lock file (filename=%s, mode=%d).",
			  $this->filename, $lockMode);
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
	 */
	public function isEmpty() {
		$stat = fstat($this->fh);
		return (0 == $stat['size']);
	}

	/**
	 * Close the file.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 */
	public function close() {
		if (FALSE === flock($this->fh, LOCK_UN)) {
			throw new \OMV\Exception("Failed to unlock file (filename=%s).",
			  $this->filename);
		}
		if (FALSE === fclose($this->fh)) {
			throw new \OMV\Exception("Failed to close file (filename=%s).",
			  $this->filename);
		}
		$this->fh = NULL;
	}

	/**
	 * Write the given content as JSON encoded string to the file.
	 * The file will be truncated to zero length before the content
	 * is written.
	 * @param content The content to be written.
	 * @return Returns the JSON representation of the content which was
	 *   written to the file.
	 * @throw E_MISC_FAILURE
	 */
	public function write($content) {
		if (FALSE === rewind($this->fh)) {
			throw new \OMV\Exception("Failed to rewind file (filename=%s).",
			  $this->filename);
		}
		// Get the JSON representation of the content.
		if (FALSE === ($content = json_encode_safe($content,
		  JSON_PRETTY_PRINT))) {
			throw new \OMV\Exception(
			  "Failed to encode content (filename=%s): %s",
			  $this->filename, json_last_error_msg());
		}
		// Empty the whole file.
		$this->truncate(0);
		// Now write the new content.
		if (FALSE === fwrite($this->fh, $content)) {
			throw new \OMV\Exception("Failed to write file (filename=%s).",
			  $this->filename);
		}
		if (FALSE === fflush($this->fh)) {
			throw new \OMV\Exception("Failed to flush file (filename=%s).",
			  $this->filename);
		}
		return $content;
	}

	/**
	 * Read the JSON encoded data from the file.
	 * @param decode Set to TRUE to decode the JSON content. Defaults to TRUE.
	 * @return The content read from the file. If \em decode was set to TRUE
	 *   the decoded content is returned.
	 * @throw E_MISC_FAILURE
	 */
	public function read($decode = TRUE, $assoc = TRUE) {
		if (FALSE === rewind($this->fh)) {
			throw new \OMV\Exception("Failed to rewind file (filename=%s).",
			  $this->filename);
		}
		if (FALSE === ($content = stream_get_contents($this->fh))) {
			throw new \OMV\Exception("Failed to read file (filename=%s).",
			  $this->filename);
		}
		// Validate JSON content.
		if (!empty($content) && !is_json($content)) {
			throw new \OMV\Exception(
			  "File contains invalid JSON (filename=%s): %s",
			  $this->filename, json_last_error_msg());
		}
		// Decode the content if requested.
		if (TRUE === $decode) {
			if (FALSE === ($content = json_decode_safe($content, $assoc))) {
				throw new \OMV\Exception(
				  "Failed to decode content (filename=%s): %s",
				  $this->filename, json_last_error_msg());
			}
		}
		return $content;
	}

	/**
	 * Truncates the file to a given length.
	 * @param size The size to truncate to.
	 * @return None.
	 * @throw E_MISC_FAILURE
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
		return @unlink($this->filename);
	}
}
