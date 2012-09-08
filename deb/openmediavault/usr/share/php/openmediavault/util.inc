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

class OMVUtil {
	/**
	* Create a UUID Version 4 (random).
	* @return A new UUID string if successful, otherwise FALSE.
	*/
	public static function uuid() {
		@OMVUtil::exec("export LANG=C; uuid -v 4 -F STR", $output, $result);
		if ($result !== 0)
			return FALSE;
		return $output[0];
	}

	/**
	* Execute the given command
	*/
	public static function exec($command, &$output = NULL, &$return_var = NULL) {
		putenv("PATH=/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin");
		return exec($command, $output, $return_var);
	}

	/**
	 * Write debug message to file '/tmp/phpdebug.log'.
	 * Take care about the permissions:
	 * touch /tmp/phpdebug.log
	 * chmod openmediavault:openmediavault /tmp/phpdebug.log
	 */
	public static function debug() {
		if (!file_exists("/tmp/phpdebug.log"))
			return;

		$args = func_get_args();
		$fmt = array_shift($args);

		// Get the backtrace
		$bt = debug_backtrace();
		$bt = $bt[1];

		// Prepare debug output
		$msg = sprintf("%s File: %s Line: %d", date(DATE_RFC822),
			basename($bt['file']), $bt['line']);
		if (isset($bt['class'])) {
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
		$cmd = sprintf("export LANG=C; sudo readlink -mn %s", escapeshellarg(
		  $path));
		@self::exec($cmd, $output, $result);
		if ($result !== 0)
			return FALSE;
		return $output[0];
	}
}
?>
