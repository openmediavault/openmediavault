<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
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

/**
 * @ingroup api
 */
class ConfigObject {
	use \OMV\DebugTrait;

	private $model;
	private $properties;

	/**
	 * @param id The data model identifier.
	 */
	public function __construct($id) {
		// Set the data model.
		// !!! Note, we need to clone the data model, otherwise we are
		// working on the original object which will cause unexpected
		// behaviour !!!.
		$modelMngr = \OMV\DataModel\Manager::getInstance();
		$this->model = clone $modelMngr->getModel($id);
		// Set the default property values.
		$this->setDefaults();
	}

	public function __clone() {
		$this->model = clone $this->model;
		$this->properties = clone $this->properties;
	}

	/**
	 * Check if the specified property exists.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @return Returns TRUE if the property exists, otherwise FALSE.
	 */
	protected function exists($name) {
		return $this->getModel()->propertyExists($name);
	}

	/**
	 * Assert that the specified property exists.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @return void
	 * @throw \OMV\AssertException
	 */
	public function assertExists($name) {
		if (FALSE === $this->exists($name)) {
			throw new \OMV\AssertException(
			  "The property '%s' does not exist in the model '%s'.",
			  $name, $this->getModelId());
		}
	}

	/**
	 * Assert that the specified property has the given value.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @param value The value of the property.
	 * @param message The alternative error message.
	 * @return void
	 * @throw \OMV\AssertException
	 */
	public function assertValue($name, $value, $message = NULL) {
		if ($this->get($name) !== $value) {
			$message = is_null($message) ? sprintf(
				"The property '%s' does not match the value '%s'.",
				$name, strval($value)) : $error;
			throw new \OMV\AssertException($message);
		}
	}

	/**
	 * Get the data model of the configuration object.
	 * @return The data model object.
	 */
	public function getModel() {
		return $this->model;
	}

	/**
	 * Get the properties dictionary.
	 * @return The properties dictionary.
	 */
	protected function getProperties() {
		return $this->properties;
	}

	/**
	 * Get a property.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @return The property value.
	 */
	public function get($name) {
		$this->assertExists($name);
		return $this->getProperties()->get($name);
	}

	/**
	 * Get all properties as an associative array.
	 * Example:
	 * @code
	 * Array
	 * (
	 *     [timezone] => Europe/Berlin
	 *     [ntp] => Array
	 *         (
	 *             [enable] => 1
	 *             [timeservers] => pool.ntp.org,pool1.ntp.org;pool2.ntp.org,sss
	 *             [clients] => 10.0.0.0/16,192.168.1.0/24;10.1.0.1/32
	 *         )
	 * )
	 * @endcode
	 * @return The array of key/value pairs.
	 */
	public function getAssoc() {
		return $this->getProperties()->getData();
	}

	/**
	 * Get all properties as an array with the keys in dot notation.
	 * Example:
	 * @code
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
	 * @endcode
	 * @return The array of key/value pairs in dot notation.
	 */
	public function getIndexed() {
		return array_flatten($this->getAssoc());
	}

	/**
	 * Get the default properties as an indexed array. The property keys
	 * may look like 'a.b.c' or 'shares.share.0.uuid'.
	 * @return The array of key/value pairs with the default values as
	 *   described by the data model.
	 */
	public function getDefaultsIndexed() {
		return array_flatten($this->getDefaultsAssoc());
	}

	/**
	 * Get the default properties as an associative array.
	 * @return The array of key/value pairs with the default values as
	 *   described by the data model.
	 */
	public function getDefaultsAssoc() {
		$dict = new \OMV\Dictionary();
		$fn = function($model, $name, $path, $schema, &$userData) {
//			$this->debug(var_export(func_get_args(), TRUE));
			$userData->set($path, $model->getPropertyDefault($path));
		};
		$this->getModel()->walkRecursive("", $fn, $dict);
		return $dict->getData();
	}

	/**
	 * Set the default property values as defined in the data model.
	 * @return void
	 */
	public function setDefaults() {
		$this->properties = new \OMV\Dictionary($this->getDefaultsAssoc());
	}

	/**
	 * Set a property.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @param value The value of the property.
	 * @return void
	 */
	public function set($name, $value, $validate = TRUE) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$this->assertExists($name);
		$model = $this->getModel();
		if (TRUE === $validate)
			$model->validateProperty($name, $value);
		// Convert the value into the proper type.
		$value = $model->convertProperty($name, $value);
		// Set the property value in the dictionary.
		$this->getProperties()->set($name, $value);
	}

	/**
	 * Set properties.
	 * Example:
	 * @code
	 * array(
	 *   "c" => "d",
	 *   "a" => [
	 *     "b" => [
	 *       "c" => 1
	 *	   ]
	 *	 ],
	 *   "x" => [
	 *     "x" => "z",
	 *     "y" => [
	 *       0 => [
	 *         a => "xxx",
	 *         b => "yyy"
	 *       ],
	 *       1 => [
	 *         a => "aaa",
	 *         b => "bbb"
	 *       ]
	 *	   ]
	 *	 ]
	 * )
	 * @endcode
	 * @param array $data The associative array of key/value pairs.
	 * @param bool $validate Set to FALSE to do not validate the property
	 *   values against the property schema defined in the model.
	 *   Defaults to TRUE.
	 * @param bool $ignore Set to TRUE to ignore and skip unknown properties.
	 *   Defaults to FALSE.
	 * @return void
	 */
	public function setAssoc(array $data, $validate = TRUE, $ignore = FALSE) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Create a flat representation of the data.
		$this->setFlatAssoc(array_flatten($data), $validate, $ignore);
	}

	/**
	 * Set properties.
	 * Example:
	 * @code
	 * array(
	 *   "c" => "d",
	 *   "x.x" => "z",
	 *   "a.b.c" => 1,
	 *   "x.y.0.a" => "xxx",
	 *   "x.y.0.b" => "yyy",
	 *   "x.y.1.a" => "aaa",
	 *   "x.y.1.b" => "bbb"
	 * )
	 * @endcode
	 * @param array $data The single dimension array of key/value pairs.
	 *   The key must be in dot notation, e.g. 'a.b.c' or 'a.b.0.c.1'
	 * @param bool $validate Set to FALSE to do not validate the property
	 *   values against the property schema defined in the model.
	 *   Defaults to TRUE.
	 * @param bool $ignore Set to TRUE to ignore and skip unknown properties.
	 *   Defaults to FALSE.
	 * @return void
	 */
	public function setFlatAssoc(array $data, $validate = TRUE,
	  $ignore = FALSE) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Make sure that the given array is not a multi-dimensional
		// associative array.
		if (TRUE === is_multi_array($data)) {
			throw new \InvalidArgumentException(
			  "The data must not be a multi-dimensional associative array.");
		}
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
	 * @return void
	 */
	public function remove($name) {
		$this->assertExists($name);
		// Remove the property from the data model schema.
		$this->getModel()->removeProperty($name);
		// Remove the property.
		$this->getProperties()->remove($name);
	}

	/**
	 * Add a new property. This can be accessed by the get/set methods.
	 * Note, this will modify the data model of this object. In common this
	 * is only useful for scalar types like string, boolean, integer, ...
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @param type The type of the property, e.g. 'string' or 'boolean'.
	 * @param default An optional default value.
	 * @return void
	 */
	public function add($name, $type, $default = NULL) {
		// Add the property to the data model schema.
		$this->getModel()->addProperty($name, $type);
		// Set the default value of the property.
		if (!is_null($default))
			$this->set($name, $default);
	}

	/**
	 * Copy an existing property to another one. The target property will
	 * be created if necessary.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @param targetName The name of the target property in dot notation,
	 *   e.g. 'x.y'.
	 * @return void
	 */
	public function copy($name, $targetName) {
		$this->assertExists($name);
		$model = $this->getModel();
		if (!$model->propertyExists($targetName)) {
			// Copy the data model of the property.
			$model->copyProperty($name, $targetName);
			// Set the property default value.
			$this->reset($targetName);
		}
		// Copy the propert value.
		$value = $this->get($name);
		$this->set($targetName, $value);
	}

	/**
	 * Move an existing property to another one. The target property will
	 * be created if necessary. The source property will be removed.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @param targetName The name of the target property in dot notation,
	 *   e.g. 'x.y'.
	 * @return void
	 */
	public function move($name, $targetName) {
		// Relocate the source property to a temporary property. This is
		// the easiest way to handle movements like 'users.user' -> 'users'.
		$tmpName = uniqid("_tmp");
		$this->copy($name, $tmpName);
		// Remove the source and target properties.
		$this->remove($name);
		if ($this->exists($targetName))
			$this->remove($targetName);
		// Finally move the temporary property to the target path.
		$this->copy($tmpName, $targetName);
		// Remove the temporary property.
		$this->remove($tmpName);
	}

	/**
	 * Reset a property to its default value as defined in the data model.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @return void
	 */
	public function reset($name) {
		$defaults = new \OMV\Dictionary($this->getDefaultsAssoc());
		$this->set($name, $defaults->get($name));
	}

	/**
	 * Determine whether a property value is empty.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @return Returns FALSE if the property exists and has a non-empty,
	 *   non-zero value, otherwise returns TRUE. If the property does not
	 *   exist an exception is thrown.
	 * @throw \OMV\Exception
	 */
	public function isEmpty($name) {
		$value = $this->get($name);
		return empty($value);
	}

	/**
	 * Determine whether a property value and the specified one are equal.
	 * @param name The name of the property in dot notation, e.g. 'a.b.c'.
	 * @param value The value to compare.
	 * @return Returns TRUE if the property value and the specified one
	 *   are equal, otherwise FALSE. If the property does not
	 *   exist an exception is thrown.
	 * @throw \OMV\Exception
	 */
	public function isEqual($name, $value) {
		$propValue = $this->get($name);
		return ($propValue === $value);
	}

	/**
	 * Check if the configuration object is new. Use this method only if
	 * the configuration object has an 'uuid' property.
	 * @return TRUE if the configuration object is identified as new,
	 *   otherwise FALSE.
	 * @throw \OMV\Exception
	 */
	public function isNew() {
		$uuid = $this->getIdentifier();
		if (FALSE === \OMV\Uuid::isUuid4($uuid))
			return FALSE;
		return (\OMV\Environment::get("OMV_CONFIGOBJECT_NEW_UUID") == $uuid);
	}

	/**
	 * Mark the configuration object as new. The 'uuid' property is
	 * modified in this case.
	 * @return void
	 * @throw \OMV\Exception
	 */
	public function setNew() {
		if ((!$this->isIterable()) || (!$this->isIdentifiable())) {
			throw new \OMV\Exception(
			  "The configuration object '%s' is not identifiable.",
			  $this->getModelId());
		}
		$uuid = \OMV\Environment::get("OMV_CONFIGOBJECT_NEW_UUID");
		$this->set($this->getModel()->getIdProperty(), $uuid);
	}

	/**
	 * Create a new object identifier (UUID). Use this method only if the
	 * configuration object has an 'uuid' property.
	 * @return The new object identifier.
	 * @throw \OMV\Exception
	 */
	public function createIdentifier() {
		if ((!$this->isIterable()) || (!$this->isIdentifiable())) {
			throw new \OMV\Exception(
			  "The configuration object '%s' is not identifiable.",
			  $this->getModelId());
		}
		$uuid = \OMV\Uuid::uuid4();
		$this->set($this->getModel()->getIdProperty(), $uuid);
		return $uuid;
	}

	/**
	 * Get the object identifier.
	 * @return The object identifier.
	 */
	public function getIdentifier() {
		if ((!$this->isIterable()) || (!$this->isIdentifiable())) {
			throw new \OMV\Exception(
			  "The configuration object '%s' is not identifiable.",
			  $this->getModelId());
		}
		return $this->get($this->getModel()->getIdProperty());
	}

	/**
	 * Verify that the configuration object is identifiable.
	 * @return Returns TRUE ifthe configuration object is identifiable,
	 *   FALSE otherwise.
	 */
	public function isIdentifiable() {
		return $this->getModel()->isIterable();
	}

	/**
	 * Verify that the configuration object is iterable.
	 * @return Returns TRUE ifthe configuration object is iterable,
	 *   FALSE otherwise.
	 */
	public function isIterable() {
		return $this->getModel()->isIterable();
	}

	/**
	 * Verify that the configuration object is referenceable.
	 * @return Returns TRUE ifthe configuration object is referenceable,
	 *   FALSE otherwise.
	 */
	public function isReferenceable() {
		return $this->getModel()->isReferenceable();
	}

	/**
	 * Get the data model identifier of this configuration object, e.g.
	 * <ul>
	 * \li conf.service.rsyncd.module
	 * \li conf.service.ftp.share
	 * \li conf.system.notification.notification
	 * \li conf.system.sharedfolder
	 * </ul>
	 * @return The data model identifier.
	 */
	public function getModelId() {
		return $this->getModel()->getId();
	}

	/**
	 * Validate the configuration object using the given data model.
	 * @return void
	 * @throw \OMV\Json\SchemaException
	 * @throw \OMV\Json\SchemaValidationException
	 */
	public function validate() {
		$this->getModel()->validate($this->getProperties());
	}
}
