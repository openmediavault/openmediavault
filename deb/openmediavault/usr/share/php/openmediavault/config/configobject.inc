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

class ConfigObject {
	use \OMV\Debugable;

	private $model = NULL;
	private $properties = array();

	/**
	 * @param id The data model identifier.
	 */
	public function __construct($id) {
		$modelMngr = \OMV\DataModel\Manager::getInstance();
		$this->model = $modelMngr->getModel($id);
		$this->properties = $this->getDefaults();
	}

	private function getModel() {
		return $this->model;
	}

	private function exists($name, $quiet = FALSE) {
		$model = $this->getModel();
		$exists = $model->propertyExists($name);
		if ((FALSE === $quiet) && (FALSE === $exists)) {
			throw new \OMV\Exception(
			  "The property '%s' does not exist in the model '%s'.",
			  $name, $model->getId());
		}
		return $exists;
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

	/**
	 * Get the default properties as an indexed array. The property keys
	 * may look like 'a.b.c' or 'shares.share.0.uuid'.
	 * @return The array of key/value pairs with the default values as
	 *   described by the data model.
	 */
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
	 * Get the default properties as an associative array.
	 * @return The array of key/value pairs with the default values as
	 *   described by the data model.
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
	 * Remove/unset the specified property. Note, this will modify the data
	 * model of this object.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 */
	public function remove($name) {
		$this->exists($name);
		// Remove the property from the data model schema.
		$this->getModel()->removeProperty($name);
		// Unset the property.
		unset($this->properties[$name]);
	}

	/**
	* Add a new property. This can be accessed by the get/set methods.
	* Note, this will modify the data model of this object.
	* @param name The name of the property in dot notation, e.g. 'a.b.c'.
	* @param type The type of the property, e.g. 'string' or 'boolean'.
	* @param default An optional default value.
	*/
	public function add($name, $type, $default = NULL) {
		// Add the property to the data model schema.
		$this->getModel()->addProperty($name, $type);
		// Set the default value of the property.
		if (!is_null($default))
			$this->set($name, $default);
	}

	/**
	 * Determine whether a property value is empty.
	 * @return Returns FALSE if the property exists and has a non-empty,
	 *   non-zero value. Otherwise returns TRUE. If the property does not
	 *   exist an exception is thrown.
	 * @throw \OMV\Exception
	 */
	public function isEmpty($name) {
		$value = $this->get($name);
		return empty($value);
	}

	/**
	 * Check if the configuration object is new. Use this method only if
	 * the object has an 'uuid' property.
	 */
	public function isNew() {
		$model = $this->getModel();
		if (FALSE === $model->isIdentifiable()) {
			throw new \OMV\Exception(
			  "The configuration object is not identifiable.");
		}
		$uuid = $this->get($model->getIdProperty());
		return \OMV\Uuid::isUuid4($uuid);
	}

	public function isIterable() {
		return $this->getModel()->isIterable();
	}

	public function isReferenceable() {
		return $this->getModel()->isReferenceable();
	}
}
