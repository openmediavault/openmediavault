<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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

abstract class OMVObject {
	public function __construct() {
	}

	protected function debug() {
		if (boolval($GLOBALS['OMV_DEBUG_PHP']) === FALSE)
			return;

		$args = func_get_args();
		$fmt = array_shift($args);

		// Get the backtrace
		$bt = debug_backtrace();
		$bt0 = $bt[0];
		$bt1 = $bt[1];

		// Prepare debug output
		$msg = sprintf("%s File: %s Line: %d", date(DATE_RFC822),
			basename($bt0['file']), $bt0['line']);
		if (isset($bt1['class'])) {
			$msg .= sprintf(" Method: %s::%s()", $bt1['class'],
				$bt1['function']);
		} else if(isset($bt1['function'])) {
			$msg .= sprintf(" Function: %s()", $entry['function']);
		}
		$msg .= sprintf(" Message: %s\n", vsprintf($fmt, $args));

		// Append debug output to log file
		file_put_contents($GLOBALS['OMV_DEBUG_PHP_LOGFILE'], $msg, FILE_APPEND);
	}
}
?>
