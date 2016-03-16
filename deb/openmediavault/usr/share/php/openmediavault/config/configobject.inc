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
		$modelMngr = &\OMV\DataModel\Manager::getInstance();
		$this->model = $modelMngr->getModel($id);
		$this->properties = $this->getDefaults();
	}

	private function getModel() {
		return $this->model;
	}

	private function exists($name) {
		$model = $this->getModel();
		if (FALSE === $model->propertyExists($name)) {
			throw new \OMV\Exception(
			  "The property '%s' does not exist in the model '%s'.",
			  $name, $model->getId());
		}
	}

	/**
	 * Get a property.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @return The property value.
	 */
	public function get($name) {
		$this->exists($name);
		return $this->properties[$name];
	}

	/**
	 * Get all properties as an associative array.
	 * Example:
	 * Array
	 * (
	 *     [timezone] => Europe/Berlin
	 *     [ntp] => Array
	 *         (
	 *             [enable] => 1
	 *             [timeservers] => pool.ntp.org,pool1.ntp.org;pool2.ntp.org,sss
	 *         )
	 * )
	 * @return The array of key/value pairs.
	 */
	public function getAssoc() {
		return array_expand($this->getIndexed());
	}

	/**
	 * Get all properties as an array with the keys in dot notation.
	 * Example:
	 * [winssupport] =>
	 * [winsserver] =>
	 * [homesenable] =>
	 * [homesbrowseable] => 1
	 * [extraoptions] =>
	 * [shares.share.0.uuid] => 1837f560-2589-47ff-98ba-287dc3b73c3f
	 * [shares.share.0.enable] => 1
	 * [shares.share.0.sharedfolderref] => e03ee0fc-4c82-4bbe-ad2e-2d7f76774be0
	 * [shares.share.0.name] => dfsdfs
	 * [shares.share.0.comment] =>
	 * [shares.share.0.guest] => no
	 * @return The array of key/value pairs in dot notation.
	 */
	public function getIndexed() {
		return $this->properties;
	}

	public function getDefaults() {
		// Create a flat representation of the data.
		$data = array();
		$fn = function($model, $name, $path, $schema, &$userData) {
			switch ($schema['type']) {
			case "array":
				$userData[$path] = $model->getPropertyDefault($path);
				// Do not process the array items.
				return FALSE;
				break;
			case "object":
				// Nothing to do here.
				break;
			default:
				$userData[$path] = $model->getPropertyDefault($path);
				break;
			}
		};
		$this->getModel()->walkRecursive("", $fn, $data);
		return $data;
	}

	/**
	 * Get the default properties.
	 * @return The array of key/value pairs with the default values as
	 *   described by the data model, otherwise FALSE.
	 */
	public function getDefaultsAssoc() {
		return array_expand($this->getDefaults());
	}

	/**
	 * Set a property.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @param value The value of the property.
	 * @return None.
	 */
	public function set($name, $value, $validate = TRUE) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$this->exists($name);
		$model = $this->getModel();
		if (TRUE === $validate)
			$model->validateProperty($name, $value);
		$this->properties[$name] = $model->convertProperty($name, $value);
	}

	/**
	 * Set properties.
	 * @param data The associative array of key/value pairs.
	 * @return None.
	 */
	public function setAssoc($data, $validate = TRUE) {
//		$this->debug(var_export(func_get_args(), TRUE));
		if (TRUE === $validate)
			$this->getModel()->validate($data);
		// Create a flat representation of the data.
		$this->setIndexed(array_flatten($data), $validate);
	}

	/**
	 * Set properties.
	 * @param data The single dimension array of path/value pairs.
	 * @return None.
	 */
	public function setIndexed($data, $validate = TRUE) {
//		$this->debug(var_export(func_get_args(), TRUE));
		foreach ($data as $path => $value)
			$this->set($path, $value, $validate);
	}

	/**
	 * Remove/unset a given property.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 */
	public function remove($name) {
		$this->exists($name);
		unset($this->properties[$name]);
	}
}
