<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
require_once("openmediavault/object.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/functions.inc");

/**
 * This class provides various helper methods.
 * @ingroup api
 */
class OMVUtil {
	/**
	* Create a UUID Version 4 (random).
	* @return A new UUID string if successful, otherwise FALSE.
	*/
	public static function uuid() {
		@OMVUtil::exec("export LANG=C; uuid -v 4 -F STR", $output, $result);
		if($result !== 0)
			return FALSE;
		return $output[0];
	}

	/**
	* Execute the given command.
	*/
	public static function exec($command, &$output = NULL, &$exitStatus = NULL) {
		putenv("PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin");
		return exec($command, $output, $exitStatus);
	}

	/**
	 * Write debug message to file '/tmp/phpdebug.log'.
	 * Take care about the permissions:
	 * touch /tmp/phpdebug.log
	 * chmod openmediavault:openmediavault /tmp/phpdebug.log
	 */
	public static function debug() {
		if(!file_exists("/tmp/phpdebug.log"))
			return;

		$args = func_get_args();
		$fmt = array_shift($args);

		// Get the backtrace
		$bt = debug_backtrace();
		$bt = $bt[1];

		// Prepare debug output
		$msg = sprintf("%s File: %s Line: %d", date(DATE_RFC822),
			basename($bt['file']), $bt['line']);
		if(isset($bt['class'])) {
			$msg .= sprintf(" Method: %s::%s()", $bt['class'], $bt['function']);
		} else if(isset($bt['function'])) {
			$msg .= sprintf(" Function: %s()", $entry['function']);
		}
		$msg .= sprintf(" Message: %s\n", vsprintf($fmt, $args));

		// Append debug output to log file
		file_put_contents("/tmp/phpdebug.log", $msg, FILE_APPEND);
	}

	/**
	 * Send an email.
	 * @param from The sender email address.
	 * @param to The receiver email address(es), e.g. 'test@xzy.com' or
	 *   'xzy@123.org,test@xzy.com'. Use the character , or ; as seperator.
	 * @param subject The subject.
	 * @param message The message.
	 * @return Return TRUE if successful, otherwise FALSE.
	 */
	public static function mail($from, $to, $subject, $message) {
		// Get date in RFC 2882 format
		$rfcdate = date('r');

		// Prepare the receivers of the email. The formatting of this string
		// must comply with RFC 2822.
		$to = strtr($to, array(";" => ","));

		// Prepare additional email headers
		$headers = <<<EOD
From: {$from}
Date: {$rfcdate}
EOD;

	  return mail($to, $subject, $message, $headers);
	}

	/**
	 * Returns the target of a symbolic link. Canonicalize by following
	 * every symlink in every component of the given name recursively,
	 * without requirements on components existence.
	 * @param path The symbolic link path.
	 * @return Returns the value of the symbolic link or FALSE on error.
	 */
	public static function readlink($path) {
		$cmd = sprintf("export LANG=C; readlink -mn %s", escapeshellarg(
		  $path));
		@self::exec($cmd, $output, $result);
		if($result !== 0)
			return FALSE;
		return $output[0];
	}
}

/**
 * Read and write JSON encoded content to a file. The file is locked when
 * it is opened and the lock is removed when it is closed.
 * @ingroup api
 */
class OMVJsonFile extends OMVObject {
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
		if(!is_null($this->fh))
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
		if(FALSE === ($this->fh = fopen($this->filename, $accessMode))) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to open file (filename=%s, mode=%s).",
			  $this->filename, $accessMode));
		}
		if(FALSE === flock($this->fh, $lockMode)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to lock file (filename=%s, mode=%d).",
			  $this->filename, $lockMode));
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
	 * Open the file.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 */
	public function close() {
		if(FALSE === flock($this->fh, LOCK_UN)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to unlock file (filename=%s).",
			  $this->filename));
		}
		if(FALSE === fclose($this->fh)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to close file (filename=%s).",
			  $this->filename));
		}
		$this->fh = NULL;
	}

	/**
	 * Write the given content as JSON encoded string to the file.
	 * The file will be truncated to zero length before the content
	 * is written.
	 * @return None.
	 * @throw E_MISC_FAILURE
	 */
	public function write($content) {
		if(FALSE === rewind($this->fh)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to rewind file (filename=%s).",
			  $this->filename));
		}
		if(FALSE === ftruncate($this->fh, 0)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to truncate file (filename=%s).",
			  $this->filename));
		}
		if(FALSE === fwrite($this->fh, json_encode_safe($content))) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to write file (filename=%s).",
			  $this->filename));
		}
		if(FALSE === fflush($this->fh)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to flush file (filename=%s).",
			  $this->filename));
		}
		return $content;
	}

	/**
	 * Read the JSON encoded data from the file.
	 * @param decode Set to TRUE to decode the JSON content. Defaults to TRUE.
	 * @return The decoded JSON content.
	 * @throw E_MISC_FAILURE
	 */
	public function read($decode = TRUE) {
		if(FALSE === rewind($this->fh)) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to rewind file (filename=%s).",
			  $this->filename));
		}
		if(FALSE === ($content = stream_get_contents($this->fh))) {
			throw new OMVException(OMVErrorMsg::E_MISC_FAILURE,
			  sprintf("Failed to read file (filename=%s).",
			  $this->filename));
		}
		return (TRUE === $decode) ? json_decode($content, TRUE) : $content;
	}

	/**
	 * Unlink the given file.
	 * @return Returns TRUE on success or FALSE on failure.
	 */
	public function unlink() {
		if($this->isOpen())
			$this->close();
		return @unlink($this->filename);
	}
}

/**
 * Sorts a series of dependencies in linear order (topological sort).
 * @ingroup api
 * @code
 * $tsort = new OMVToplogicalSort;
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
class OMVToplogicalSort {
	private $nodes;
	private $dependencies;

	function __construct() {
		$this->clean();
	}

	/**
	 * Cleanup the internal structures.
	 */
	public function clean() {
		$this->nodes = array();
		$this->dependencies = array();
	}

	/**
	 * Add a node and its dependencies.
	 * @param node The node.
	 * @param deps The node dependencies. This can be an array or string.
	 *   Defaults to an empty array.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function add($node, $deps = array()) {
		if(!is_array($deps)) {
			$deps = array($deps);
		}
		// Do a simple check to prevent circular dependencies.
		foreach($deps as $depk => $depv) {
			if(array_key_exists($depv, $this->dependencies)) {
				if(in_array($node, $this->dependencies[$depv]))
					return FALSE;
			}
		}
		// Add node to internal structures.
		$this->nodes[] = $node;
		foreach($deps as $depk => $depv) {
			$this->nodes[] = $depv;
		}
		$this->nodes = array_unique($this->nodes);
		$this->dependencies[$node] = $deps;
		return TRUE;
	}

	/**
	 * @param ignoreMissing Ignore dependency nodes that do not exist.
	 *   Defaults to TRUE.
	 * @return Returns the sorted nodes based on the given dependencies,
	 *   otherwise FALSE if the dependencies can not be solved.
	 */
	public function sort($ignoreMissing = TRUE) {
		$sorted = array();
		// Remove dependencies that do not exist.
		if(TRUE === $ignoreMissing) {
			foreach($this->nodes as $nodek => $nodev) {
				if(array_key_exists($nodev, $this->dependencies))
					continue;
				foreach($this->dependencies as $depsk => &$depsv) {
					$depsv = array_values(array_diff($depsv, array($nodev)));
				}
				unset($this->nodes[$nodek]);
			}
		}
		// Sort the nodes.
		$abort = FALSE;
		while((FALSE === $abort) && (count($this->nodes) > count($sorted))) {
			$abort = TRUE;
			foreach($this->dependencies as $node => $deps) {
				$found = TRUE;
				foreach($deps as $depk => $depv) {
					if(!in_array($depv, $sorted)) {
						$found = FALSE;
						break;
					}
				}
				if(TRUE === $found) {
					$sorted[] = $node;
					unset($this->dependencies[$node]);
					$abort = FALSE;
				}
			}
		}
		// Sucessful?
		if((TRUE === $abort) && (count($this->nodes) > count($sorted))) {
			return FALSE;
		}
		return $sorted;
	}
}
?>
