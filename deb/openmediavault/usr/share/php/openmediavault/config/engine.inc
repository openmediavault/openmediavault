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
require_once("openmediavault/object.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/config/object.inc");

class Engine extends \OMVObject {
	/**
	 * Returns the configuration engine singleton.
	 * @return The singleton object.
	 */
	public static function &getInstance() {
		static $instance = NULL;
		if (!isset($instance))
			$instance = new Engine();
		return $instance;
	}

	/**
	 * Get the configuration object for the given XPath expression.
	 * @param xpath The XPath expression to execute.
	 * @param id The data model identifier. If \em id is not set, then
	 *   the xpath will be used as identifier instead.
	 * @return The configuration object, otherwise FALSE.
	 */
	public function get($xpath, $id = "") {
		global $xmlConfig;
		if (empty($id))
			$id = $xpath;
		$data = $xmlConfig->get($xpath);
		if (is_null($data))
			return FALSE;
		$object = new Object($id);
		$object->initialize();
		if (!$object->setAssoc($data, FALSE)) {
			$this->setLastError($object->getLastError());
			return FALSE;
		}
		return $object;
	}

	public function replace($xpath, $object) {
		global $xmlConfig;
		if (!$object instanceof Object)
			return FALSE;
		if (!$object->isInitialized())
			return FALSE;
		$data = $object->getAssoc();
		if (FALSE === $xmlConfig->replace($xpath, $data))
			return FALSE;
		return TRUE;
	}
}
