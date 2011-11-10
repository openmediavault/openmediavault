<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
require_once("openmediavault/functions.inc");
require_once("openmediavault/util.inc");

/**
 * @class OMVLogFile
 * This class handles log file access and modifications.
 */
class OMVLogFile extends OMVObject {
	/**
	 * Preconfigured log file types. A type has the following structure:
	 * @param filename The name of the log file. This will be used when
	 * downloading the file via WebUI.
	 * @param filepath The path of the log file.
	 * @param regex The regular expression used to parse the log file.
	 * @param columns The column configuration.
	 * Example:
	 * $log = new OMVLog("syslog", "/var/log/syslog",
	 *   "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+(.*)$/", array(
	 *     "date" => 1,
	 *     "user" => 2,
	 *     "event" => 3
	 *   )
	 * );
	 * $log->getList(...);
	 */
	static $types = array(
		"syslog" => array(
			"filename" => "syslog",
			"filepath" => "/var/log/syslog",
			"regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+(.*)$/",
			"columns" => array(
				"date" => 1,
				"user" => 2,
				"event" => 3
			)
		),
		"daemon" => array(
			"filename" => "daemon.log",
			"filepath" => "/var/log/daemon.log",
			"regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+(.*)$/",
			"columns" => array(
				"date" => 1,
				"user" => 2,
				"event" => 3
			)
		),
		"auth" => array(
			"filename" => "auth.log",
			"filepath" => "/var/log/auth.log",
			"regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+(.*)$/",
			"columns" => array(
				"date" => 1,
				"user" => 2,
				"event" => 3
			)
		),
		"messages" => array(
			"filename" => "messages",
			"filepath" => "/var/log/messages",
			"regex" => "/^(\S+\s+\d+\s+\S+)\s+(\S+)\s+(.*)$/",
			"columns" => array(
				"date" => 1,
				"user" => 2,
				"event" => 3
			)
		)
	);

	private $id = null;

	/**
	 * Constructor
	 * @param id The identifier of the log file configuration.
	 */
	function __construct($id) {
		$this->id = $id;
	}

	/**
	 * Register a new log file type.
	 * @param id The identifier of the log file configuration, e.g.
	 * syslog or auth.
	 * @param config The type configuration.
	 * @return None
	 */
	public static function registerType($id, $config) {
		self::$types[$id] = $config;
	}

	/**
	 * Checks if a log file type is registered.
	 * @return TRUE if the log file type is registered, otherwise FALSE.
	 */
	public function isRegistered() {
		return array_key_exists($this->id, self::$types);
	}

	/**
	 * Get the file name of the log file, e.g. 'syslog'.
	 * @return The file name of the log file, otherwise FALSE.
	 */
	public function getFileName() {
		$type = self::$types[$this->id];
		return $type['filename'];
	}

	/**
	 * Get the file path of the log file, e.g. '/var/log/syslog'.
	 * @return The file path of the log file, otherwise FALSE.
	 */
	public function getFilePath() {
		$type = self::$types[$this->id];
		return $type['filepath'];
	}

	/**
	 * Get the regular expression used to parse a log file line.
	 */
	public function getRegex() {
		$type = self::$types[$this->id];
		return $type['regex'];
	}

	/**
	 * Get the column configuration.
	 */
	public function getColumns() {
		$type = self::$types[$this->id];
		return $type['columns'];
	}
}
?>
