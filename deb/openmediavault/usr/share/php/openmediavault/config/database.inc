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
namespace OMV\Config;

require_once("openmediavault/globals.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/config.inc");

class Database extends \OMV\BaseObject {
	/**
	 * Returns the configuration database singleton.
	 * @return The singleton object.
	 */
	public static function &getInstance() {
		static $instance = NULL;
		if (!isset($instance))
			$instance = new Database();
		return $instance;
	}

	private function isLoaded() {
		global $xmlConfig;
		if (!$xmlConfig instanceof \OMVConfigEngine)
			throw new DatabaseException("Database is not loaded.");
	}

	/**
	 * Get the configuration object at the specified XPath expression.
	 * @param xpath The XPath expression to execute.
	 * @param id The data model identifier. If \em id is not set, then
	 *   the xpath will be used as identifier instead.
	 * @return The configuration object.
	 */
	public function get($xpath, $id = "") {
		global $xmlConfig;
		if (empty($id))
			$id = $xpath;
		$this->isLoaded();
		$data = $xmlConfig->get($xpath);
		if (!is_array($data)) {
			throw new DatabaseException("Failed to execute query '%s'.",
			  $xpath);
		}
		$object = new ConfigObject($id);
		$object->setAssoc($data, FALSE);
		return $object;
	}

	/**
	 * Get the configuration objects at the specified XPath expression
	 * as an array.
	 * @param xpath The XPath expression to execute.
	 * @param id The data model identifier. If \em id is not set, then
	 *   the xpath will be used as identifier instead.
	 * @return An array containing the requested configuration objects,
	 *   otherwise an empty array.
	 */
	public function getList($xpath, $id = "") {
		global $xmlConfig;
		if (empty($id))
			$id = $xpath;
		$this->isLoaded();
		$data = $xmlConfig->getList($xpath);
		if (!is_array($data)) {
			throw new DatabaseException("Failed to execute query '%s'.",
			  $xpath);
		}
		$objects = array();
		foreach ($data as $datak => $datav) {
			$object = new ConfigObject($id);
			$object->setAssoc($datav, FALSE);
			$objects[] = $object;
		}
		return $objects;
	}

	/**
	 * Set the configuration object at the specified XPath expression.
	 * @param xpath The XPath expression to execute.
	 * @param object The configuration object to use.
	 * @return None.
	 */
	public function set($xpath, ConfigObject $object) {
		global $xmlConfig;
		$this->isLoaded();
		$data = $object->getAssoc();
		if (FALSE === $xmlConfig->set($xpath, $data))
			throw new DatabaseException("XPath '%s' does not exist.", $xpath);
	}

	/**
	 * Replace the configuration at the specified XPath expression.
	 * @param xpath The XPath expression to execute.
	 * @param object The configuration object to use.
	 * @return None.
	 */
	public function replace($xpath, ConfigObject $object) {
		global $xmlConfig;
		$this->isLoaded();
		$data = $object->getAssoc();
		if (FALSE === $xmlConfig->replace($xpath, $data))
			throw new DatabaseException("XPath '%s' does not exist.", $xpath);
	}

	/**
	 * Update the existing configuration at the specified XPath expression.
	 * The specified data will override the existing configuration. If the
	 * given XPath expression does not exist, then the method will exit
	 * immediatelly.
	 * @param xpath The XPath expression to execute.
	 * @param object The configuration object to use.
	 * @return None.
	 */
	public function update($xpath, ConfigObject $object) {
		global $xmlConfig;
		$this->isLoaded();
		$data = $object->getAssoc();
		if (FALSE === $xmlConfig->update($xpath, $data))
			throw new DatabaseException("XPath '%s' does not exist.", $xpath);
	}

	/**
	 * Delete the nodes matching the specified XPath expression.
	 * @param xpath The XPath expression to execute.
	 * @return None.
	 */
	public function delete($xpath) {
		$this->isLoaded();
		if (FALSE === $xmlConfig->delete($xpath))
			throw new DatabaseException("XPath '%s' does not exist.", $xpath);
	}
}
