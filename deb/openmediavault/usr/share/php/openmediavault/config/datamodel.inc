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

/**
 * @ingroup api
 */
class DataModel extends \OMV\DataModel\DataModel {
	use \OMV\DebugTrait;

	private $propertiesSchema = null;

	public function __construct($model) {
		// Validate the data model definition.
		$schema = new \OMV\Json\Schema('{
			"type":"object",
			"properties":{
				"type":{"type":"string","enum":["config"],"required":true},
				"id":{"type":"string","required":true},
				"alias":{"type":"string","required":false},
				"persistent":{"type":"boolean","required":false},
				"title":{"type":"string","required":false},
				"description":{"type":"string","required":false},
				"notificationid":{"type":"string","required":false},
				"properties":{"type":"any","required":true},
				"queryinfo":{
					"type":"object",
					"required":false,
					"properties":{
						"xpath":{"type":"string","required":true},
						"iterable":{"type":"boolean","required":true},
						"idproperty":{"type":"string","required":false},
						"refproperty":{"type":"string","required":false}
					}
				}
			}
		}');
		try {
			// Convert the associative array into a JSON string
			// before validating the model definition.
			$schema->validate(json_encode_safe($model));
		} catch(\OMV\Json\SchemaValidationException $e) {
			throw new \OMV\Exception(
			  "The data model of the configuration object is invalid: %s",
			  $e->getMessage());
		}
		// Call the parent constructor.
		parent::__construct($model);
		// Create the schema describing the RPC parameters.
		$this->propertiesSchema = new \OMV\DataModel\Schema([
			"type" => "object",
			"properties" => $model['properties']
		]);
	}

	public function __clone() {
		$this->propertiesSchema = clone $this->propertiesSchema;
	}

	public function getPropertiesSchema() {
		return $this->propertiesSchema;
	}

	public function getQueryInfo() {
		$model = $this->getModel();
		return $model['queryinfo'];
	}

	public function isIdentifiable() {
		$qi = $this->getQueryInfo();
		return array_key_exists("idproperty", $qi);
	}

	public function getIdProperty() {
		$qi = $this->getQueryInfo();
		return $qi['idproperty'];
	}

	public function isIterable() {
		$qi = $this->getQueryInfo();
		if (!array_key_exists("iterable", $qi))
			return FALSE;
		return $qi['iterable'];
	}

	public function isReferenceable() {
		$qi = $this->getQueryInfo();
		return (array_key_exists("idproperty", $qi) && array_key_exists(
		  "refproperty", $qi));
	}

	public function getRefProperty() {
		$qi = $this->getQueryInfo();
		return $qi['refproperty'];
	}

	/**
	 * Tests whether the data model instance is persistent.
	 * @return TRUE if the data model instance is persistent. If the
	 *   property is not available in the data model definition, then
	 *   TRUE is assumed.
	 */
	public function isPersistent() {
		$model = $this->getModel();
		if (!array_key_exists("persistent", $model))
			return TRUE;
		return $model['persistent'];
	}

	/**
	 * Get the notification identifier. It is auto-generated based
	 * on the data model property named 'id':
	 * 'org.openmediavault.' + <id>
	 * The identifier can be overwritten using the property that is
	 * named 'notificationid'.
	 * @return The notification identifier string, e.g.
	 *   'org.openmediavault.x.y.z'.
	 */
	public function getNotificationId() {
		return array_value($this->getModel(), "notificationid",
		  sprintf("org.openmediavault.%s", $this->getId()));
	}

	/**
	 * Validate the specified data against the data model.
	 * @param data The JSON data to validate.
	 * @return void
	 * @throw \OMV\Json\SchemaException
	 * @throw \OMV\Json\SchemaValidationException
	 */
	final public function validate($data) {
		$this->getPropertiesSchema()->validate($data);
	}

	public function validateProperty($path, $value) {
		$this->getPropertiesSchema()->validate($value, $path);
	}

	/**
	 * Add a new property to the model.
	 * @param path The path of the property, e.g. "aaa.bbb.ccc".
	 * @param type The type of the property, e.g. 'string' or 'boolean'.
	 * @return void
	 */
	public function addProperty($path, $type, array $schema = array()) {
		$schema['type'] = $type;
		$schema = $this->getPropertiesSchema()->addProperty($path, $schema);
	}

	/**
	 * Copies a property from one item to another.
	 * @param path The path of the property, e.g. "aaa.bbb.ccc".
	 * @param targetPath The path of the new property, e.g. "xxx.yyy.zzz".
	 * @return void
	 */
	public function copyProperty($path, $targetPath) {
		$propSchema = $this->getPropertiesSchema()->getAssocByPath($path);
		$this->assertPropertyNotExists($targetPath);
		$this->addProperty($targetPath, $propSchema['type'], $propSchema);
	}

	/**
	 * Add a property from the model.
	 * @param path The path of the property, e.g. "aaa.bbb.ccc".
	 */
	public function removeProperty($path) {
		$this->getPropertiesSchema()->removeProperty($path);
	}

	/**
	 * Convert the given value into the type which is declared in the
	 * property schema at the specified path. The original value is returned
	 * when the property type can not be processed, e.g. 'any', 'array',
	 * 'object' or 'null'.
	 * @param name The path of the property, e.g. "aaa.bbb.ccc".
	 * @param value The value to convert.
	 * @return The converted value.
	 */
	public function convertProperty($path, $value) {
		$propSchema = $this->getPropertiesSchema()->getAssocByPath($path);
		if (is_array($propSchema['type'])) {
			throw new \OMV\Json\SchemaException(
			  "%s: The attribute 'type' must not be an array.",
			  $path);
		}
		switch ($propSchema['type']) {
		case "boolean":
			$value = boolvalEx($value);
			break;
		case "integer":
			$value = intval($value);
			break;
		case "number":
		case "double":
		case "float":
			$value = doubleval($value);
			break;
		case "string":
			$value = strval($value);
			break;
		}
		return $value;
	}

	/**
	 * Get the default value of the specified property as defined in
	 * the schema.
	 * @param path The path of the property, e.g. "aaa.bbb.ccc".
	 * @return The default value as specified in the data model
	 *   schema or by the type of the property.
	 */
	public function getPropertyDefault($path) {
		$default = NULL;
		// Get the JSON schema of the specified property.
		$propSchema = $this->getPropertiesSchema()->getAssocByPath($path);
		// Is a default value set in the data model? If not, then use
		// a generic default value which depends on the type and other
		// parameters defined in the data model.
		if (array_key_exists("default", $propSchema))
			$default = $propSchema['default'];
		else {
			if (is_array($propSchema['type'])) {
				throw new \OMV\Json\SchemaException(
				  "%s: The attribute 'type' must not be an array.",
				  $path);
			}
			$type = "any";
			if (array_key_exists("type", $propSchema))
				$type = $propSchema['type'];
			switch ($type) {
			case "array":
				$default = []; // Indexed array
				break;
			case "object":
				$default = []; // Associative array
				break;
			case "boolean":
				$default = FALSE;
				break;
			case "integer":
				$default = 0;
				break;
			case "number":
			case "double":
			case "float":
				$default = 0.0;
				break;
			case "string":
				$default = "";
				if (array_key_exists("format", $propSchema)) {
					switch ($propSchema['format']) {
					case "uuidv4":
						// If the property is the identifier of an iterable
						// data model, then set the UUID which is defined as
						// new configuration object ID.
						if ($this->isIdentifiable()) {
							if ($this->getIdProperty() == $path) {
								$default = \OMV\Environment::get(
								  "OMV_CONFIGOBJECT_NEW_UUID");
							}
						}
						break;
					}
				}
				break;
			case "any":
			case "null":
				$default = NULL;
				break;
			}
		}
		return $default;
	}

	/**
	 * Check if the specified property exists.
	 * @param path The path of the property, e.g. "aaa.bbb.ccc".
	 * @return TRUE if the property exists, otherwise FALSE.
	 */
	public function propertyExists($path) {
		$exists = TRUE;
		try {
			$this->getPropertiesSchema()->getAssocByPath($path);
		} catch(\OMV\Json\SchemaException $e) {
			$exists = FALSE;
		} catch(\OMV\Json\SchemaPathException $e) {
			$exists = FALSE;
		}
		return $exists;
	}

	/**
	 * Assert that the specified property exists.
	 * @param path The path of the property, e.g. "aaa.bbb.ccc".
	 * @return void
	 * @throw \OMV\AssertException
	 */
	public function assertPropertyExists($path) {
		if (!$this->propertyExists($path)) {
			throw new \OMV\AssertException("The property '%s' does not exist.",
			  $path);
		}
	}

	/**
	 * Assert that the specified property does not exist.
	 * @param path The path of the property, e.g. "aaa.bbb.ccc".
	 * @return void
	 * @throw \OMV\Json\SchemaPathException
	 */
	public function assertPropertyNotExists($path) {
		if ($this->propertyExists($path)) {
			throw new \OMV\AssertException("The property '%s' already exists.",
			  $path);
		}
	}

	/**
	 * Helper function.
	 */
	private function _walk($name, $path, $schema, $callback, &$userData = NULL) {
//		$this->debug(var_export(func_get_args(), TRUE));
		if (!array_key_exists("type", $schema)) {
			throw new \OMV\Json\SchemaException(
			  "No 'type' attribute defined at '%s'.", $path);
		}
		switch ($schema['type']) {
/*
		case "array":
			if (!array_key_exists("items", $schema)) {
				throw new \OMV\Json\SchemaException(
				  "No 'items' attribute defined at '%s'.", $path);
			}
			// Process the property node.
			return $this->_walk($name, $path, $schema['items'], $callback,
			  $userData);
			break;
*/
		case "object":
			if (!array_key_exists("properties", $schema)) {
				throw new \OMV\Json\SchemaException(
				  "No 'properties' attribute defined at '%s'.", $path);
			}
			foreach ($schema['properties'] as $propName => $propSchema) {
				// Build the property path. Take care that a valid path
				// is generated. To ensure this, empty parts are removed.
				$propPath = implode(".", array_filter(array($path, $propName)));
				// Process the property node.
				if (FALSE === $this->_walk($propName, $propPath, $propSchema,
				  $callback, $userData))
					continue;
			}
			break;
		default:
			// Call the callback function. Stop processing this property
			// if the callback function returns FALSE.
			return call_user_func_array($callback, array($this, $name, $path,
			  $schema, &$userData));
			break;
		}
	}

	/**
	 * Apply a user function recursively to every property of the schema.
	 * @param path The path of the property, e.g. "aaa.bbb.ccc".
	 * @param callback The callback function.
	 * @param userdata If the optional userdata parameter is supplied, it
	 *   will be passed to the callback function.
	 */
	public function walkRecursive($path, $callback, &$userData = NULL) {
		$propSchema = $this->getPropertiesSchema()->getAssocByPath($path);
		$this->_walk("", $path, $propSchema, $callback, $userData);
	}
}
