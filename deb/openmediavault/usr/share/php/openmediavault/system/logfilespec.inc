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
namespace OMV\System;

require_once("openmediavault/functions.inc");

/**
 * This class that handles the log file specification.
 * @ingroup api
 */
class LogFileSpec {
	/**
	 * Preconfigured log file types. A type has the following structure:
	 * @param filename The name of the log file. This will be used when
	 *   downloading the file via WebUI.
	 * @param filepath The path of the log file.
	 * @param regex The regular expression used to parse the log file.
	 * @param columns The column configuration. It contains key/value pairs,
	 *   the key is the name of the column, the value is the index of the
	 *   regex match. If the value is an array, then the field \em index
	 *   contains the index of the regex match and \em func contains an user
	 *   defined function that is called while processing. It can be used
	 *   to convert a time/date string to an Unix timestamp for example.
	 */
	static private $specs = [
/* Example:
		"syslog" => array(
			"filename" => "syslog",
			"filepath" => "/var/log/syslog",
			"regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+(.*)$/",
			"columns" => array(
				"date" => array(
					"index" => 1,
					"func" => create_function('$v',
					  'return strpdate($v,"M j G:i:s");')
				),
				"user" => 2,
				"event" => 3
			)
		)
*/
	];
	private $id = null;

	/**
	 * Constructor
	 * @param id The identifier of the log file specification.
	 */
	function __construct($id) {
		self::assertIsRegistered($id);
		$this->id = $id;
	}

	/**
	 * Register a new log file specification.
	 * @param id The identifier of the log file specification, e.g.
	 *   'syslog' or 'auth'.
	 * @param config The log file specification.
	 * @return None
	 */
	public static function registerSpecification($id, array $config) {
		self::$specs[$id] = $config;
	}

	/**
	 * Checks if a log file specification is registered.
	 * @param id The identifier of the log file specification.
	 * @return None.
	 * @throw \OMV\AssertException
	 */
	public static function assertIsRegistered($id) {
		if (FALSE === array_key_exists($id, self::$specs)) {
			throw new \OMV\AssertException(
			  "Unknown log file specification (id=%s).", $id);
		}
	}

	/**
	 * Get the file name of the log file, e.g. 'syslog'.
	 * @return The file name of the log file, otherwise FALSE.
	 */
	public function getFileName() {
		$spec = self::$specs[$this->id];
		return $spec['filename'];
	}

	/**
	 * Get the file path of the log file, e.g. '/var/log/syslog'.
	 * @return The file path of the log file, otherwise NULL.
	 */
	public function getFilePath() {
		$spec = self::$specs[$this->id];
		return array_value($spec, "filepath", NULL);
	}

	/**
	 * Get the command line to be executed to get the log file content.
	 * @return The command line, otherwise NULL.
	 */
	public function getCommand() {
		$spec = self::$specs[$this->id];
		return array_value($spec, "command", NULL);
	}

	/**
	 * Get the regular expression used to parse a log file line.
	 */
	public function getRegex() {
		$spec = self::$specs[$this->id];
		return $spec['regex'];
	}

	/**
	 * Get the column configuration.
	 */
	public function getColumns() {
		$spec = self::$specs[$this->id];
		return $spec['columns'];
	}
}
