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
namespace OMV\Json;

require_once("openmediavault/functions.inc");

/**
 * Implements a JSON schema validator.
 * @see https://tools.ietf.org/html/draft-zyp-json-schema-03
 * @see http://json-schema.org/latest/json-schema-core.html
 * @see http://json-schema.org/latest/json-schema-validation.html
 */
class Schema {
	use \OMV\DebugTrait;

	private $schema = [];

	/**
	 * @param schema The schema as associative array in JSON schema syntax.
	 */
	public function __construct($schema) {
		$this->schema = $schema;
	}

	/**
	 * Navigate to the given JSON path, e.g. 'a1.b2' or 'ntp.enable'.
	 * Example:
	 * @code
	 * array (
	 * 	'type' => 'object',
	 * 	'id' => 'conf.system.time',
	 * 	'alias' => '//system/time',
	 * 	'properties' =>
	 * 	array (
	 * 	  'timezone' =>
	 * 	  array (
	 * 	    'type' => 'string',
	 * 	    'default' => 'Etc/UTC',
	 * 	  ),
	 * 	  'ntp' =>
	 * 	  array (
	 * 	    'type' => 'object',
	 * 	    'properties' =>
	 * 	    array (
	 * 	      'enable' =>
	 * 	      array (
	 * 	        'type' => 'boolean',
	 * 	        'default' => 0,
	 * 	      ),
	 * 	      'timeservers' =>
	 * 	      array (
	 * 	        'type' => 'string',
	 * 	      ),
	 * 	      'clients' =>
	 * 	      array (
	 * 	        'type' => 'string',
	 * 	      ),
	 * 	  ),
	 * 	  ),
	 * 	),
	 * )
	 *
	 * Example:
	 * 'extraoptions' =>
	 *  array (
	 *     'type' => 'string',
	 *  ),
	 *  'shares' =>
	 *  array (
	 *     'type' => 'object',
	 *     'properties' =>
	 *     array (
	 *       'share' =>
	 *       array (
	 *         'type' => 'array',
	 *         'items' =>
	 *         array (
	 *           'type' => 'object',
	 *           'properties' =>
	 *           array (
	 *             'uuid' =>
	 *             array (
	 *               'type' => 'string',
	 *               'format' => 'uuidv4',
	 *             ),
	 *             'enable' =>
	 *             array (
	 *               'type' => 'boolean',
	 *               'default' => 0,
	 *                 ),
	 * ...
	 * @endcode
	 */
	private function &_getSchemaByPath($path, &$schema) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Do we have reached the end of the requested path (path is empty)?
		if (empty($path))
			return $schema;
		// Explode the path in dot notation into its parts.
		$parts = explode(".", $path);
		// Filter array indices from the path in dot notation.
		// Example: shares.share.0.uuid
		// To access the schema of an array we do not need them.
		$parts = array_filter($parts, function($v) { return !is_numeric($v); });
		// Validate the path. Something like "aa.bb.cc." "a..b.c", or ".xx.yy"
		// is invalid.
		if (0 < count(array_filter($parts, function($v) { return empty($v); })))
			throw new SchemaPathException("Invalid path '%s' syntax.", $path);
		// Do we process an 'object' or 'array' node?
		// !!! Note, the 'type' can be an array. How to handles them here?
		// ToDo: Handle types like '{ "type": [ "string", "object" ] }'
		if (array_key_exists("type", $schema) && is_string($schema['type'])) {
			switch ($schema['type']) {
			case "array":
				// Does 'items' exist?
				if (!array_key_exists("items", $schema)) {
					throw new SchemaPathException(
					  "No 'items' attribute defined for path '%s'.",
					  $path);
				}
				return $this->_getSchemaByPath($path, $schema['items']);
				break;
			case "object":
				// Does 'properties' exist and is it an associative array?
				if (!(array_key_exists("properties", $schema) && is_assoc_array(
				  $schema['properties']))) {
					throw new SchemaPathException(
					  "No 'properties' attribute defined for path '%s'.",
					  $path);
				}
				return $this->_getSchemaByPath($path, $schema['properties']);
				break;
			}
		}
		$key = array_shift($parts);
		// Check if the node has the requested key/value pair.
		if (!array_key_exists($key, $schema))
			throw new SchemaPathException("Invalid path '%s'.", $path);
		// Continue to walk down the tree.
		return $this->_getSchemaByPath(implode(".", $parts), $schema[$key]);
	}

	/**
	 * Add a new property to the schema at runtime.
	 * @param path The path of the property, e.g. "aaa.bbb.ccc", where 'ccc'
	 *   is the name of the property to add.
	 * @param schema The schema of the property to add, e.g.
	 *   array("type" => "integer", "default" => 100)
	 */
	public function addProperty($path, array $schema = array()) {
		// Add the property.
		$parts = explode(".", $path);
		$name = array_pop($parts);
		// Get the property schema.
		$parentSchema = &$this->_getSchemaByPath(implode(".", $parts),
		  $this->schema);
		switch ($parentSchema['type']) {
		case "array":
			$parentSchema = &$parentSchema['items'];
			break;
		case "object":
			$parentSchema = &$parentSchema['properties'];
			break;
		default:
			throw new SchemaPathException(
			  "Can not add property at path '%s'.",
			  $path);
			break;
		}
		$parentSchema[$name] = $schema;
	}

	/**
	 * Remove a property from the schema at runtime.
	 * @param path The path of the property to remove.
	 */
	public function removeProperty($path) {
		// PHP doesn't allow to delete the array entry by unsetting the
		// reference. So we need to get the parent property to delete
		// the specified property.
		$parts = explode(".", $path);
		$propName = array_pop($parts);
		$path = implode(".", $parts);
		// Get the parent property schema.
		$parentPropSchema = &$this->_getSchemaByPath($path, $this->schema);
		// Unset the specified property.
		switch ($parentPropSchema['type']) {
		case "array":
			unset($parentPropSchema['items'][$propName]);
			break;
		case "object":
			unset($parentPropSchema['properties'][$propName]);
			break;
		}
	}

	/**
	 * Returns the JSON schema.
	 * @return The JSON schema as associative array.
	 */
	final public function getAssoc() {
		if (is_string($this->schema)) {
			$this->schema = json_decode($this->schema, TRUE);
			if (is_null($this->schema)) {
				throw new SchemaException("Failed to decode schema: %s",
				  json_last_error_msg());
			}
		}
		return $this->schema;
	}

	/**
	 * Returns the JSON schema by the given path.
	 * @return The JSON schema as associative array.
	 */
	final public function getAssocByPath($name) {
		// Get the schema.
		$schema = $this->getAssoc();
		// Navigate down to the given path.
		return $this->_getSchemaByPath($name, $schema);
	}

	/**
	 * Validate the given value.
	 * @param value The value to validate.
	 * @param name The JSON path of the entity to validate, e.g. 'aa.bb.cc',
	 *   defaults to an empty string. Use an empty value for the root.
	 * @return void
	 * @throw \OMV\Json\SchemaException
	 * @throw \OMV\Json\SchemaValidationException
	 */
	final public function validate($value, $name = "") {
		// Convert all arrays (indexed and associative) to JSON string to
		// ensure that they are converted into the required array format.
		if (is_array($value))
			$value = json_encode_safe($value);
		// Convert JSON string to array.
		// !!! Note, this is no associative array. !!!
		if (is_json($value))
			$value = json_decode($value);
		$schema = $this->getAssocByPath($name);
		$this->validateType($value, $schema, $name);
	}

	protected function validateType($value, $schema, $name) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$types = "any";
		if (array_key_exists("type", $schema))
			$types = $schema['type'];
		if (!is_array($types))
			$types = [ $types ];
		$valid = FALSE;
		$lastException = NULL;
		foreach ($types as $typek => $typev) {
			try {
				switch ($typev) {
				case "any":
					$this->validateAny($value, $schema, $name);
					$valid = TRUE;
					break;
				case "array":
					$this->validateArray($value, $schema, $name);
					$valid = TRUE;
					break;
				case "boolean":
					$this->validateBoolean($value, $schema, $name);
					$valid = TRUE;
					break;
				case "object":
					$this->validateObject($value, $schema, $name);
					$valid = TRUE;
					break;
				case "integer":
					$this->validateInteger($value, $schema, $name);
					$valid = TRUE;
					break;
				case "number":
					$this->validateNumber($value, $schema, $name);
					$valid = TRUE;
					break;
				case "string":
					$this->validateString($value, $schema, $name);
					$valid = TRUE;
					break;
				case "null":
					$this->validateNull($value, $schema, $name);
					$valid = TRUE;
					break;
				default:
					throw new SchemaException(
					  "%s: The type '%s' is not defined.",
					  $name, $typev);
					break;
				}
			} catch(SchemaValidationException $e) {
				// Continue with the next type but remember the exception.
				$lastException = $e;
			}
			// Break the foreach loop here because one of the defined types
			// is successfully validated.
			if (TRUE === $valid)
				break;
		}
		// If the validation is not successful, then trow the
		// last exception.
		if (FALSE === $valid)
			throw $lastException;
	}

	protected function validateAny($value, $schema, $name) {
	}

	protected function validateBoolean($value, $schema, $name) {
		if (!is_bool($value)) {
			throw new SchemaValidationException(
				"%s: The value '%s' is not a boolean.",
				$name, json_encode_safe($value));
		}
	}

	protected function validateInteger($value, $schema, $name) {
		if (!is_integer($value)) {
			throw new SchemaValidationException(
				"%s: The value %s is not an integer.",
				$name, json_encode_safe($value));
		}
		$this->checkMinimum($value, $schema, $name);
		$this->checkExclusiveMinimum($value, $schema, $name);
		$this->checkMaximum($value, $schema, $name);
		$this->checkExclusiveMaximum($value, $schema, $name);
		$this->checkEnum($value, $schema, $name);
		$this->checkOneOf($value, $schema, $name);
	}

	protected function validateNumber($value, $schema, $name) {
		if (!is_numeric($value)) {
			throw new SchemaValidationException(
				"%s: The value %s is not a number.",
				$name, json_encode_safe($value));
		}
		$this->checkMinimum($value, $schema, $name);
		$this->checkExclusiveMinimum($value, $schema, $name);
		$this->checkMaximum($value, $schema, $name);
		$this->checkExclusiveMaximum($value, $schema, $name);
		$this->checkEnum($value, $schema, $name);
		$this->checkOneOf($value, $schema, $name);
	}

	protected function validateString($value, $schema, $name) {
		if (!is_string($value)) {
			throw new SchemaValidationException(
				"%s: The value %s is not a string.",
				$name, json_encode_safe($value));
		}
		$this->checkPattern($value, $schema, $name);
		$this->checkMinLength($value, $schema, $name);
		$this->checkMaxLength($value, $schema, $name);
		$this->checkFormat($value, $schema, $name);
		$this->checkEnum($value, $schema, $name);
		$this->checkOneOf($value, $schema, $name);
	}

	protected function validateArray($value, $schema, $name) {
		if (!is_array($value)) {
			throw new SchemaValidationException(
				"%s: The value %s is not an array.",
				$name, json_encode_safe($value));
		}
		$this->checkMinItems($value, $schema, $name);
		$this->checkMaxItems($value, $schema, $name);
		$this->checkItems($value, $schema, $name);
	}

	protected function validateObject($value, $schema, $name) {
		if (!empty($value) && !is_object($value)) {
			throw new SchemaValidationException(
				"%s: The value %s is not an object.",
				$name, json_encode_safe($value));
		}
		$this->checkProperties($value, $schema, $name);
	}

	protected function validateNull($value, $schema, $name) {
		if (!is_null($value)) {
			throw new SchemaValidationException(
				"%s: The value %s is not NULL.",
				$name, json_encode_safe($value));
		}
	}

	protected function checkMinimum($value, $schema, $name) {
		if (!isset($schema['minimum']))
			return;
		if ($schema['minimum'] > $value) {
			throw new SchemaValidationException(
			  "%s: The value '%s' is less than %s.",
			  $name, strval($value), strval($schema['minimum']));
		}
	}

	protected function checkMaximum($value, $schema, $name) {
		if (!isset($schema['maximum']))
			return;
		if ($schema['maximum'] < $value) {
			throw new SchemaValidationException(
			  "%s: The value '%s' is bigger than %s.",
			  $name, strval($value), strval($schema['maximum']));
		}
	}

	protected function checkExclusiveMinimum($value, $schema, $name) {
		if (!isset($schema['minimum']))
			return;
		if (!(isset($schema['exclusiveMinimum']) &&
		  (TRUE === $schema['exclusiveMinimum'])))
			return;
		if ($value == $schema['minimum']) {
			throw new SchemaValidationException(
			  "%s: Invalid value '%s', must be greater than %s.",
			  $name, strval($value), strval($schema['minimum']));
		}
	}

	protected function checkExclusiveMaximum($value, $schema, $name) {
		if (!isset($schema['maximum']))
			return;
		if (!(isset($schema['exclusiveMaximum']) &&
		  (TRUE === $schema['exclusiveMaximum'])))
			return;
		if ($value == $schema['maximum']) {
			throw new SchemaValidationException(
			  "%s: Invalid value '%s', must be less than %s.",
			  $name, strval($value), strval($schema['maximum']));
		}
	}

	protected function checkMinLength($value, $schema, $name) {
		if (!isset($schema['minLength']))
			return;
		$length = strlen($value);
		if ($schema['minLength'] > $length) {
			throw new SchemaValidationException(
			  "%s: The value '%s' is too short, minimum length is %d.",
			  $name, $value, $schema['minLength']);
		}
	}

	protected function checkMaxLength($value, $schema, $name) {
		if (!isset($schema['maxLength']))
			return;
		$length = strlen($value);
		if ($schema['maxLength'] < $length) {
			throw new SchemaValidationException(
			  "%s: The value '%s' is too long, maximum length is %d.",
			  $name, $value, $schema['maxLength']);
		}
	}

	protected function checkPattern($value, $schema, $name) {
		if (!isset($schema['pattern']))
			return;
		// Note, the pattern is defined according to the ECMA 262 regular
		// expression dialect. So it must be adapted to the PHP syntax.
		$regex = sprintf("/%s/u", $schema['pattern']);
		if (!preg_match($regex, $value)) {
			throw new SchemaValidationException(
			  "%s: The value '%s' doesn't match the pattern '%s' (code=%d).",
			  $name, $value, $schema['pattern'], preg_last_error());
		}
	}

	/**
	 * @see https://tools.ietf.org/html/draft-zyp-json-schema-03#section-5.23
	 */
	protected function checkFormat($value, $schema, $name) {
		if (!isset($schema['format']))
			return;
		switch ($schema['format']) {
		case "date-time":
		case "datetime":
			if (!preg_match('/^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$/', $value)) {
				throw new SchemaValidationException(
				  "%s: The value '%s' does not match to ISO 8601.",
				  $name, $value);
			}
			break;
		case "date":
			if (!preg_match('/^\d{4}-\d{2}-\d{2}$/', $value)) {
				throw new SchemaValidationException(
				  "%s: The value '%s' does not match to YYYY-MM-DD.",
				  $name, $value);
			}
			break;
		case "time":
			if (!preg_match('/^\d{2}:\d{2}:\d{2}$/', $value)) {
				throw new SchemaValidationException(
				  "%s: The value '%s' does not match to hh:mm:ss.",
				  $name, $value);
			}
			break;
		case "host-name":
		case "hostname":
			// See https://datatracker.ietf.org/doc/html/rfc1123#section-2
			if (!preg_match('/^[a-zA-Z0-9]([-a-zA-Z0-9]{0,61}[a-zA-Z0-9])'.
			  '{0,1}$/u', $value)) {
				throw new SchemaValidationException(
				  "%s: The value '%s' is not a valid hostname.",
				  $name, $value);
			}
			break;
		case "regex":
			// ToDo ...
			break;
		case "uri":
			if (!filter_var($value, FILTER_VALIDATE_URL,
			  FILTER_FLAG_QUERY_REQUIRED)) {
				throw new SchemaValidationException(
				  "%s: The value '%s' is not an URI.",
				  $name, $value);
			}
			break;
		case "email":
			if (!filter_var($value, FILTER_VALIDATE_EMAIL)) {
				throw new SchemaValidationException(
				  "%s: The value '%s' is not an email.",
				  $name, $value);
			}
			break;
		case "ip-address":
		case "ipv4":
			if (!filter_var($value, FILTER_VALIDATE_IP, FILTER_FLAG_IPV4)) {
				throw new SchemaValidationException(
				  "%s: The value '%s' is not an IPv4 address.",
				  $name, $value);
			}
			break;
		case "ipv6":
			if (filter_var($value, FILTER_VALIDATE_IP, FILTER_FLAG_IPV6)) {
				throw new SchemaValidationException(
				  "%s: The value '%s' is not an IPv6 address.",
				  $name, $value);
			}
			break;
		default:
			throw new SchemaException(
			  "%s: The format '%s' is not defined.",
			  $name, $schema['format']);
			break;
		}
	}

	protected function checkEnum($value, $schema, $name) {
		if (!isset($schema['enum']))
			return;
		if (!is_array($schema['enum'])) {
			throw new SchemaException(
			  "%s: The attribute 'enum' is not an array.",
			  $name);
		}
		if (!is_array($value))
			$value = [ $value ];
		foreach ($value as $valuek => $valuev) {
			if (!in_array($valuev, $schema['enum'])) {
				throw new SchemaValidationException(
				  "%s: Invalid value '%s', allowed values are '%s'.",
				  $name, $valuev, implode(',', $schema['enum']));
			}
		}
	}

	protected function checkMinItems($value, $schema, $name) {
		if (!isset($schema['minItems']))
			return;
		if (count($value) < $schema['minItems']) {
			throw new SchemaValidationException(
			  "%s: Not enough array items, minimum is %d.",
			  $name, $schema['minItems']);
		}
	}

	protected function checkMaxItems($value, $schema, $name) {
		if (!isset($schema['maxItems']))
			return;
		if (count($value) > $schema['maxItems']) {
			throw new SchemaValidationException(
			  "%s: Too many array items, maximum is %d.",
			  $name, $schema['maxItems']);
		}
	}

	protected function checkProperties($value, $schema, $name) {
		if (!isset($schema['properties'])) {
			throw new SchemaException(
			  "%s: No 'properties' attribute defined.",
			  $name);
		}
		// Note, this is a workaround to process empty objects
		// correctly that are submitted as arrays because of
		// historical reasons.
		$valueProps = !empty($value) ? get_object_vars($value) : [];
		foreach ($schema['properties'] as $propk => $propv) {
			// Build the new path. Strip empty parts.
			$parts = [ $name, $propk ];
			$parts = array_filter($parts, "strlen");
			$path = implode(".", $parts);
			// Check if the 'required' attribute is set.
			if (!array_key_exists($propk, $valueProps)) {
				if (isset($propv['required']) &&
				  (TRUE === $propv['required'])) {
					throw new SchemaValidationException(
					  "%s: Missing 'required' attribute '%s'.",
					  $name, $path);
				}
				continue;
			}
			$this->validateType($valueProps[$propk], $propv, $path);
		}
	}

	protected function checkItems($value, $schema, $name) {
		if (!isset($schema['items'])) {
			throw new SchemaException(
			  "%s: No 'items' attribute defined.",
			  $name);
		}
		if (is_assoc_array($schema['items'])) {
			foreach ($value as $itemk => $itemv) {
				$path = sprintf("%s[%d]", $name, $itemk);
				$this->validateType($itemv, $schema['items'], $path);
			}
		} else if (is_array($schema['items'])) {
			foreach ($value as $itemk => $itemv) {
				$path = sprintf("%s[%d]", $name, $itemk);
				$valid = FALSE;
				foreach ($schema['items'] as $itemSchema) {
					try {
						$this->validateType($itemv, $itemSchema, $path);
						$valid = TRUE;
						break;
					} catch (SchemaValidationException $e) {
						// Nothing to do here.
					}
				}
				if (!$valid) {
					$types = array_map(function($item) {
						return $item['type'];
					}, $schema['items']);
					throw new SchemaValidationException(
					  "%s: Invalid 'items' value, must be one of the ".
					  "following types '%s'.",
					  $path, implode(", ", $types));
				}
			}
		} else {
			throw new SchemaValidationException(
			  "%s: Invalid 'items' value.",
			  $name);
		}
	}

	protected function checkOneOf($value, $schema, $name) {
		if (!isset($schema['oneOf']))
			return;
		if (!is_array($schema['oneOf'])) {
			throw new SchemaException(
				"%s: The 'oneOf' attribute is not an array.",
				$name);
		}
		$valid = FALSE;
		foreach ($schema['oneOf'] as $subSchemak => $subSchemav) {
			try {
				$this->validateType($value, $subSchemav, $name);
				// If validation succeeds for one of the schema, then we
				// can exit immediately.
				$valid = TRUE;
				break;
			} catch (SchemaValidationException $e) {
				// Nothing to do here.
			}
		}
		if (!$valid) {
			throw new SchemaValidationException(
				"%s: The value %s does not match exactly one schema of %s.",
				$name, json_encode_safe($value),
				json_encode_safe($schema['oneOf']));
		}
	}
}
