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
require_once("openmediavault/object.inc");
require_once("openmediavault/error.inc");

class ConfigObject extends \OMVObject {
	private $model = NULL;
	private $properties = array();

	/**
	 * @param id The data model identifier.
	 */
	public function __construct($id) {
		$this->model = $id;
	}

	public function initialize() {
		if (TRUE === $this->isInitialized())
			return TRUE;
		$modelMgr = &\OMV\DataModel\Manager::getInstance();
		if (FALSE === ($model = $modelMgr->getModel($this->model))) {
			$this->setLastError(
			  sprintf("The model '%s' does not exist.",
			  $this->model));
			return FALSE;
		}
		$this->model = $model;
		$this->properties = $this->getDefaultsAssoc();
		return TRUE;
	}

	public function isInitialized() {
		if (!($this->getModel() instanceof \OMV\DataModel\Model)) {
			$this->setLastError("Object is not initialized.");
			return FALSE;
		}
		return TRUE;
	}

	private function getModel() {
		return $this->model;
	}

	/**
	 * Get a property.
	 * @param name The name of the property.
	 * @return The property value, otherwise NULL.
	 */
	public function get($name) {
		if (FALSE === $this->isInitialized())
			return FALSE;
		$model = $this->getModel();
		if (FALSE === $model->propertyExists($name)) {
			$this->setLastError($model->getLastError());
			return NULL;
		}
		return $this->properties[$name];
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
		if (FALSE === $this->isInitialized())
			return FALSE;
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
	 * @return TRUE if successful, otherwise FALSE.
	 */
	public function set($name, $value, $validate = TRUE) {
//		$this->debug(var_export(func_get_args(), TRUE));
		if (FALSE === $this->isInitialized())
			return FALSE;
		$model = $this->getModel();
		if (FALSE === $model->propertyExists($name)) {
			$this->setLastError($model->getLastError());
			return FALSE;
		}
		if (TRUE === $validate) {
			if (FALSE === $model->validateProperty($name, $value)) {
				$this->setLastError($model->getLastError());
				return FALSE;
			}
		}
		$this->properties[$name] = $model->convertProperty($name, $value);
		return TRUE;
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
