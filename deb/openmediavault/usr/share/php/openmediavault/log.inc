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
 * This class handles logfile access and modifications.
 */
class OMVLogFile extends OMVObject {
	/**
	 * Preconfigured logfile types. A type has the following structure:
	 * @param filename The name of the logfile. This will be used when
	 * downloading the file via WebUI.
	 * @param filepath The path of the logfile.
	 * @param regex The regular expression used to parse the logfile.
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
	 * @param id The identifier of the logfile configuration.
	 */
	function __construct($id) {
		$this->id = $id;
	}

	/**
	 * Register a new logfile type.
	 * @param id The identifier of the logfile configuration, e.g.
	 * syslog or auth.
	 * @param config The type configuration.
	 * @return None
	 */
	public static function registerType($id, $config) {
		self::$types[$id] = $config;
	}

	/**
	 * Get the parsed logfile content as array.
	 * @param start The index where to start.
	 * @param count The number of lines to process.
	 * @param sortField The name of the column used to sort. See logfile
	 * type configuration.
	 * @param sortDir The sort direction, ASC or DESC.
	 * @return An array containing the requested logfile lines. The
	 * field \em total contains the total number of lines, \em data contains
	 * the logfile lines as array with the configured columns. If an error
	 * occurs FALSE is returned.
	 */
	function get($start, $count, $sortField, $sortDir) {
//		$this->debug(var_export(func_get_args(), TRUE));
		if (!array_key_exists($this->id, self::$types))
			return FALSE;
		$type = self::$types[$this->id];
		// Get the logfile content
		$cmd = sprintf("sudo cat %s", $type['filepath']);
		OMVUtil::exec($cmd, $content);
		// Process content
		$data = array();
		foreach ($content as $contentk => $contentv) {
			// Skip invalid pattern matches
			$result = preg_match($type['regex'], $contentv, $matches);
			if ((FALSE === $result) || (0 == $result))
				continue;
			$record = array();
			foreach ($type['columns'] as $columnk => $columnv) {
				$record[$columnk] = $matches[$columnv];
			}
			$data[] = $record;
		}
		// Sort content
		if (!is_null($sortField) && $sortField !== "date") {
			array_sort_key($data, $sortField);
		}
		if (!is_null($sortDir) && $sortDir == "DESC") {
			$data = array_reverse($data);
		}
		// Prepare result
		$result = array(
			"total" => count($content),
			"data" => array_slice($data, $start, $count)
		);
		return $result;
	}

 	/**
	 * Clear a logfile.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	function clear() {
		if (!array_key_exists($this->id, self::$types))
			return FALSE;
		$type = self::$types[$this->id];
		// Execute command
		$cmd = sprintf("cat /dev/null | sudo tee %s", $type['filepath']);
		OMVUtil::exec($cmd, $output, $res);
		if ($res !== 0)
			return FALSE;
		return TRUE;
	}

 	/**
	 * Get the content of a logfile.
	 * @return An array with the field \em filename which is the name of
	 * the logfile and \em content which contains the logfile content. If an
	 * error occurs FALSE is returned.
	 */
	function getContent() {
		if (!array_key_exists($this->id, self::$types))
			return FALSE;
		$type = self::$types[$this->id];
		// Get the logfile content
		$cmd = sprintf("sudo cat %s", $type['filepath']);
		OMVUtil::exec($cmd, $content);
		return array(
			"filename" => $type['filename'],
			"content" => implode("\r\n", $content)
		);
	}
}
?>
