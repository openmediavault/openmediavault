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

require_once("openmediavault/functions.inc");
require_once("openmediavault/error.inc");

class ConfigObject extends \OMV\BaseObject {
	private $model = NULL;
	private $properties = array();

	/**
	 * @param id The data model identifier.
	 */
	public function __construct($id) {
		$modelMgr = &\OMV\DataModel\Manager::getInstance();
		$this->model = $modelMgr->getModel($id);
		$this->properties = $this->getDefaultsAssoc();
	}

	private function getModel() {
		return $this->model;
	}

	/**
	 * Array
	 * (
	 *     [enable] => 0
	 *     [timeservers] =>
	 *     [timezone] => Europe/Berlin
	 *     [ntp] => Array
	 *         (
	 *             [enable] => 1
	 *             [timeservers] => ntp1.webart.de
	 *         )
	 * )
	 */
	private function _getPropertyByPath($path, $node) {
		$parts = explode(".", $path);
		// Get the key of the node.
		$key = array_shift($parts);
		// Check if the key/value pair exists.
		if (!array_key_exists($key, $node))
			return FALSE;
		// Do we have found the requested node?
		if (0 == count($parts))
			return $node[$key];
		// If the path is not finally reached, then check if the node
		// is an array that we can walk down.
		if (!is_array($node[$key]))
			return FALSE;
		return $this->_getPropertyByPath(implode(".", $parts), $node[$key]);
	}

	/**
	 * Get a property.
	 * @param name The name of the property.
	 * @return The property value.
	 */
	public function get($name) {
		$model = $this->getModel();
		if (FALSE === $model->propertyExists($name)) {
			throw new \OMV\Exception(
			  "The property '%s' does not exist in the model.",
			  $name);
		}
		if (FALSE === ($value = $this->_getPropertyByPath($name,
		  $this->properties))) {
			throw new \OMV\Exception(
			  "The property '%s' does not exist.",
			  $name);
		}
		return $value;
	}

	/**
	 * Get all properties.
	 * @return The array of key/value pairs.
	 */
	public function getAssoc() {
		return $this->properties;
	}

	/**
	 * Get the default properties.
	 * @return The array of key/value pairs with the default values as
	 *   described by the data model, otherwise FALSE.
	 */
	public function getDefaultsAssoc() {
		$object = array();
		$fn = function($name, $schema, &$userData) {
			switch ($schema['type']) {
			case "object":
				$userData = array();
				break;
			default:
				$default = NULL;
				if (array_key_exists("default", $schema))
					$default = $schema['default'];
				$userData[$name] = $default;
				break;
			}
		};
		$this->getModel()->walk($fn, $object);
		return $object;
	}

	/**
	 * Set a property.
	 * @param name The name of the property.
	 * @param value The value of the property.
	 * @return None.
	 */
	public function set($name, $value, $validate = TRUE) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$model = $this->getModel();
		if (FALSE === $model->propertyExists($name)) {
			throw new \OMV\Exception(
			  "The property '%s' does not exist in the model.",
			  $name);
		}
		if (TRUE === $validate)
			$model->validateProperty($name, $value);
		$this->properties[$name] = $model->convertProperty($name, $value);
	}

	/**
	 * Set properties.
	 * @param data The array of key/value pairs.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function setAssoc($data, $validate = TRUE) {
//		$this->debug(var_export(func_get_args(), TRUE));
		foreach ($data as $datak => $datav) {
			if (FALSE === $this->set($datak, $datav, $validate))
				return FALSE;
		}
		return TRUE;
	}
}
