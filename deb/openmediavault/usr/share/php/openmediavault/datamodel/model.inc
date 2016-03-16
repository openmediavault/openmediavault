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
namespace OMV\DataModel;

require_once("openmediavault/functions.inc");

class Model extends \OMV\BaseObject {
	private $schema = NULL;

	/**
	 * @param schema The schema as in JSON schema syntax.
	 */
	public function __construct($schema) {
		if (!($schema instanceof Schema))
			$schema = new Schema($schema);
		$this->schema = $schema;
	}

	public function getSchema() {
		return $this->schema;
	}

	public function getId() {
		$schemaAssoc = $this->getSchema()->getAssoc();
		return array_value($schemaAssoc, "id", FALSE);
	}

	public function getAlias() {
		$schemaAssoc = $this->getSchema()->getAssoc();
		return array_value($schemaAssoc, "alias", FALSE);
	}

	public function getTitle() {
		$schemaAssoc = $this->getSchema()->getAssoc();
		return array_value($schemaAssoc, "title", "");
	}

	public function getDescription() {
		$schemaAssoc = $this->getSchema()->getAssoc();
		return array_value($schemaAssoc, "description", "");
	}

	/**
	 * Validate the specified data against the data model.
	 * @param data The JSON data to validate.
	 * @return None.
	 * @throw \OMV\Json\SchemaException
	 * @throw \OMV\Json\SchemaValidationException
	 */
	public function validate($data) {
		$this->getSchema()->validate($data);
	}

	public function validateProperty($path, $value) {
		$this->getSchema()->validate($value, $path);
	}

	/**
	 * Add a new property to the model.
	 * @param name The path of the property, e.g. "aaa.bbb.ccc".
	 * @param type The type of the property.
	 */
	public function addProperty($path, $type, array $properties = array()) {
		switch ($type) {
		case "boolean":
		case "integer":
		case "number":
		case "double":
		case "string":
			break;
		case "float":
			$type = "number";
			break;
		default:
			throw new \OMV\SchemaException("The type '%s' is not supported.",
			  $type);
			break;
		}
		$properties['type'] = $type;
		$schema = $this->getSchema()->addProperty($path, $properties);
	}

	public function convertProperty($path, $value) {
		$propSchema = $this->getSchema()->getAssocByPath($path);
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
	 * @return The default value as specified in the schema or by
	 *   the properties type.
	 */
	public function getPropertyDefault($path) {
		$propSchema = $this->getSchema()->getAssocByPath($path);
		$default = NULL;
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
				$default = array(); // Indexed array
				break;
			case "object":
				$default = array(); // Associative array
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
				break;
			case "any":
			case "null":
				$default = NULL;
				break;
			}
		}
		return $default;
	}

	public function propertyExists($path) {
		$exists = TRUE;
		try {
			$this->getSchema()->getAssocByPath($path);
		} catch(\OMV\Json\SchemaException $e) {
			$exists = FALSE;
		} catch(\OMV\Json\SchemaPathException $e) {
			$exists = FALSE;
		}
		return $exists;
	}

	/**
	 * Helper function.
	 */
	private function _walk($name, $path, $schema, $callback, &$userData = NULL) {
//		$this->debug(var_export(func_get_args(), TRUE));
		if (!array_key_exists("type", $schema)) {
			throw new SchemaException(
			  "No 'type' attribute defined at '%s'.", $path);
		}
		if ("object" == $schema['type']) {
			if (!array_key_exists("properties", $schema)) {
				throw new SchemaException(
				  "No 'properties' attribute defined at '%s'.", $path);
			}
			// Call the callback function. Abort if the callback function
			// returns FALSE.
			if (FALSE === call_user_func_array($callback, array($this, $name,
			  $path, $schema, &$userData)))
				return;
			foreach ($schema['properties'] as $propName => $propSchema) {
				// Build the property path. Take care that a valid path
				// is generated. To ensure this, empty parts are removed.
				$propPath = implode(".", array_filter(array($path, $propName)));
				// Call the callback function. Stop processing this property
				// if the callback function returns FALSE.
				if (FALSE === call_user_func_array($callback, array($this,
				  $propName, $propPath, $propSchema, &$userData)))
					continue;
				// Process the property node.
				$this->_walk($propName, $propPath, $propSchema, $callback,
				  $userData);
			}
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
		$propSchema = $this->getSchema()->getAssocByPath($path);
		$this->_walk("", $path, $propSchema, $callback, $userData);
	}
}
