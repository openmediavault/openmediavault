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

	/**
	 * Check if the specified property exists.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @return Returns TRUE if the property exists, otherwise FALSE.
	 */
	protected function exists($name) {
		$model = $this->getModel();
		return $model->propertyExists($name);
	}

	/**
	 * Assert that the specified property exists.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @return None.
	 * @throw \OMV\Exception
	 */
	protected function assertExists($name) {
		if (FALSE === $this->exists($name)) {
			throw new \OMV\Exception(
			  "The property '%s' does not exist in the model '%s'.",
			  $name, $this->getModel()->getId());
		}
	}

	/**
	 * Get the model of the configuration object.
	 * @return The data model object.
	 */
	public function getModel() {
		return $this->model;
	}

	/**
	 * Get a property.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @return The property value.
	 */
	public function get($name) {
		$this->assertExists($name);
		$propSchema = $this->getModel()->getPropertiesSchema();
		$subPropSchema = $propSchema->getAssocByPath($name);
		switch ($subPropSchema['type']) {
		case "array":
		case "object":
			$propertiesAssoc = $this->getAssoc();

			break;
		default:
			$result = $this->properties[$name];
			break;
		}
		return $result;
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
		$this->assertExists($name);
		$model = $this->getModel();
		if (TRUE === $validate)
			$model->validateProperty($name, $value);
		$this->properties[$name] = $model->convertProperty($name, $value);
	}

	/**
	 * Set properties.
	 * @param data The associative array of key/value pairs.
	 * @param validate Set to FALSE to do not validate the property values
	 *   against the property schema defined in the model.
	 *   Defaults to TRUE.
	 * @param ignore Set to TRUE to ignore and skip unknown properties.
	 *   Defaults to FALSE.
	 * @return None.
	 */
	public function setAssoc($data, $validate = TRUE, $ignore = FALSE) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Create a flat representation of the data.
		$this->setIndexed(array_flatten($data), $validate, $ignore);
	}

	/**
	 * Set properties. Properties that are not defined in the model are
	 * ignored.
	 * @param data The single dimension array of path/value pairs.
	 * @param validate Set to FALSE to do not validate the property values
	 *   against the property schema defined in the model.
	 *   Defaults to TRUE.
	 * @param ignore Set to TRUE to ignore and skip unknown properties.
	 *   Defaults to FALSE.
	 * @return None.
	 */
	public function setIndexed($data, $validate = TRUE, $ignore = FALSE) {
//		$this->debug(var_export(func_get_args(), TRUE));
		foreach ($data as $propk => $propv) {
			if ((TRUE === $ignore) && (FALSE === $this->exists($propk)))
				continue;
			$this->set($propk, $propv, $validate);
		}
	}

	/**
	 * Remove/unset the specified property. Note, this will modify the data
	 * model of this object.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 */
	public function remove($name) {
		$this->assertExists($name);
		// Remove the property from the data model schema.
		$this->getModel()->removeProperty($name);
		// Unset the property. Note, take care with properties that have
		// child nodes.
		//
		// Indexed:
		// trap.enable
		// trap.community
		//
		// Assoc:
		// array(
		//     trap => array(
		//         enable => TRUE,
		//         community => "public",
		//     )
		// )
		foreach ($this->properties as $propk => $propv) {
			$regex = sprintf("/^(%s)([.].+)?$/i", $name);
			if (1 !== preg_match($regex, $propk))
				continue;
			unset($this->properties[$propk]);
		}
	}

	/**
	* Add a new property. This can be accessed by the get/set methods.
	* Note, this will modify the data model of this object.
	* @param name The name of the property in dot notation, e.g. 'a.b.c'.
	* @param type The type of the property, e.g. 'string' or 'boolean'.
	* @param default An optional default value.
	* @return None.
	*/
	public function add($name, $type, $default = NULL) {
		// Add the property to the data model schema.
		$this->getModel()->addProperty($name, $type);
		// Set the default value of the property.
		if (!is_null($default))
			$this->set($name, $default);
	}

	/**
	 * Map an existing property to another one. the target property will
	 * be created if necessary.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @param targetName The name of the target property in dot notation,
	 *   e.g. 'x.y'.
	 * @return None.
	 */
	public function map($name, $targetName) {
		$this->assertExists($name);
		$model = $this->getModel();
		if (!$model->propertyExists($targetName))
			$model->copyProperty($name, $targetName);
		$value = $this->get($name);
		$this->set($targetName, $value);
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
	 * the configuration object has an 'uuid' property.
	 * @return TRUE if the cconfiguration object is identified as new,
	 *   otherwise FALSE.
	 * @throw \OMV\Exception
	 */
	public function isNew() {
		if ((!$this->isIterable()) || (!$this->isIdentifiable())) {
			throw new \OMV\Exception(
			  "The configuration object '%s' is not identifiable.",
			  $this->getModel()->getId());
		}
		$value = $this->get($this->getModel()->getIdProperty());
		if (FALSE === \OMV\Uuid::isUuid4($value))
			return FALSE;
		return ($GLOBALS['OMV_UUID_UNDEFINED'] == $value);
	}

	/**
	 * Mark the configuration object as new. The 'uuid' property is
	 * modified in this case.
	 * @return None.
	 * @throw \OMV\Exception
	 */
	public function setNew() {
		if ((!$this->isIterable()) || (!$this->isIdentifiable())) {
			throw new \OMV\Exception(
			  "The configuration object '%s' is not identifiable.",
			  $this->getModel()->getId());
		}
		$value = $GLOBALS['OMV_UUID_UNDEFINED'];
		$this->set($this->getModel()->getIdProperty(), $value);
	}

	/**
	 * Create a new UUID. Use this method only if the configuration object
	 * has an 'uuid' property.
	 * @return None.
	 * @throw \OMV\Exception
	 */
	public function createIdentifier() {
		if ((!$this->isIterable()) || (!$this->isIdentifiable())) {
			throw new \OMV\Exception(
			  "The configuration object '%s' is not identifiable.",
			  $this->getModel()->getId());
		}
		$value = \OMV\Uuid::uuid4();
		$this->set($this->getModel()->getIdProperty(), $value);
	}

	public function isIdentifiable() {
		return $this->getModel()->isIterable();
	}

	public function isIterable() {
		return $this->getModel()->isIterable();
	}

	public function isReferenceable() {
		return $this->getModel()->isReferenceable();
	}
}
