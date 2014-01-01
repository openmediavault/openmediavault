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
require_once("openmediavault/globals.inc");
require_once("openmediavault/functions.inc");

/**
 * @ingroup api
 */
abstract class OMVObject {
	private $lastError = "";

	/**
	 * Constructor
	 */
	public function __construct() {
	}

	/**
	 * Set the last error.
	 * @param error The error description.
	 * @return Returns the previous error.
	 */
	final protected function setLastError($error) {
		$lastError = $this->lastError;
		$this->lastError = $error;
		return $lastError;
	}

	/**
	 * Get the last error.
	 * @return The error description of the last error if set.
	 */
	final public function getLastError() {
		$lastError = $this->lastError;
		if(is_array($lastError))
			$lastError = implode("\n", $lastError);
		return $lastError;
	}

	/**
	 * Print debug message to stdout and file if debugging is enabled.
	 */
	final protected function debug() {
		if(boolval($GLOBALS['OMV_DEBUG_PHP']) === FALSE)
			return;

		$args = func_get_args();
		$fmt = array_shift($args);

		// Get the backtrace.
		$bt = debug_backtrace();
		$bt0 = $bt[0];
		$bt1 = $bt[1];

		// Prepare debug message.
		$msg = sprintf("%s File: %s Line: %d", date(DATE_RFC822),
		  basename($bt0['file']), $bt0['line']);
		if(isset($bt1['class'])) {
			$msg .= sprintf(" Method: %s::%s()", $bt1['class'],
			  $bt1['function']);
		} else if(isset($bt1['function'])) {
			$msg .= sprintf(" Function: %s()", $entry['function']);
		}
		$msg .= sprintf(" Message: %s\n", vsprintf($fmt, $args));

		// Output message to stdout.
		//fwrite(STDOUT, $msg);
		file_put_contents("php://stdout", $msg);
		// Append message to debug log file.
		file_put_contents($GLOBALS['OMV_DEBUG_PHP_LOGFILE'], $msg,
		  FILE_APPEND);
	}
}
?>
