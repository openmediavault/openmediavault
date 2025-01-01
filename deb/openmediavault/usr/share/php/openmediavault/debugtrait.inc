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
 */
trait DebugTrait {
	/**
	 * Print debug message to stdout and file if debugging is enabled.
	 * @return void
	 */
	final protected function debug() {
		if (TRUE !== \OMV\Environment::getBoolean("OMV_DEBUG_PHP", FALSE))
			return;

		$args = func_get_args();
		$fmt = array_shift($args);

		// Get the backtrace.
		$bt = debug_backtrace();
		$bt0 = $bt[0];
		$bt1 = $bt[1];

		// Build the message.
		$msg = sprintf("DEBUG [%s File=%s Line=%d", date(DATE_RFC822),
		  basename($bt0['file']), $bt0['line']);
		if (isset($bt1['class'])) {
			$msg .= sprintf(" Method=%s::%s()", $bt1['class'],
			  $bt1['function']);
		} else if (isset($bt1['function'])) {
			$msg .= sprintf(" Function=%s()", $bt1['function']);
		}
		$msg .= sprintf("]: %s\n", vsprintf($fmt, $args));

		// Output the message to stdout.
		//fwrite(STDOUT, $msg);
		file_put_contents("php://stdout", $msg);

		// Append the message to the debug log file.
		file_put_contents(\OMV\Environment::get("OMV_DEBUG_PHP_LOGFILE"),
			$msg, FILE_APPEND);
	}
}
