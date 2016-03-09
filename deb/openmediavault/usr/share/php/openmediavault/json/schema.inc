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
namespace OMV\Json;

/**
 * Implements a JSON schema validator.
 * @see https://tools.ietf.org/html/draft-zyp-json-schema-03
 * @see http://json-schema.org/latest/json-schema-core.html
 * @see http://json-schema.org/latest/json-schema-validation.html
 */
class Schema extends \OMV\BaseObject {
	private $schema = array();

	/**
	 * @param schema The schema as associative array in JSON schema syntax.
	 */
	public function __construct($schema) {
		$this->schema = $schema;
	}

	/**
	 * Navigate to the given JSONPath, e.g. 'a1.b2' will return
	 * '[type] => number'.
	 * Example:
	 * Array
	 * (
	 *     [type] => object
	 *     [properties] => Array
	 *         (
	 *             [a1] => Array
	 *                 (
	 *                     [type] => object
	 *                     [properties] => Array
	 *                         (
	 *                             [b1] => Array
	 *                                 (
	 *                                     [type] => string
	 *                                 )
	 *                             [b2] => Array
	 *                                 (
	 *                                     [type] => number
	 *                                 )
	 *                         )
	 *                 )
	 *             [a2] => Array
	 *                 (
	 *                     [type] => string
	 *                 )
	 *         )
	 * )
	 */
	private function _getAssocByPath($path, $schema) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Validate the path. Something like "aa.bb.cc." or ".xx.yy"
		// is invalid.
		if (("." == substr($path, 1, 1)) || ("." == substr($path, -1)))
			return FALSE;
		// Explode the path parts.
		$parts = explode(".", $path);
		// Do we have reached the end of the path?
		if ((1 >= count($parts)) && empty($parts[0]))
			return $schema;
		// Do we process an 'object' node? In this case step down to
		// the 'properties' leaf.
		if (array_key_exists("type", $schema) && ("object" == $schema['type']))
			return $this->_getAssocByPath($path, $schema['properties']);
		$key = array_shift($parts);
		// Check if the key/value pair exists.
		if (!array_key_exists($key, $schema))
			return FALSE;
		// Walk down the tree.
		return $this->_getAssocByPath(implode(".", $parts),
		  $schema[$key]);
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
		if (FALSE === ($schema = $this->_getAssocByPath(
		  $name, $schema))) {
			throw new SchemaException("The path '%s' is invalid.",
			  $name);
		}
		return $schema;
	}

	/**
	 * Validate the given value.
	 * @param value The value to validate.
	 * @param name The JSON path of the entity to validate, e.g. 'aa.bb.cc',
	 *   defaults to an empty string.
	 * @return None.
	 */
	final public function validate($value, $name = "") {
		if (is_string($value))
			$value = json_decode($value);
		if (FALSE === ($schema = $this->getAssocByPath($name))) {
			throw new SchemaException("The path '%s' is invalid.",
			  $name);
		}
		$this->validateType($value, $schema, $name);
	}

	protected function validateType($value, $schema, $name) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$types = "any";
		if (array_key_exists("type", $schema))
	        $types = $schema['type'];
		if (!is_array($types))
            $types = array($types);
		foreach ($types as $typek => $typev) {
			try {
	            switch ($typev) {
	            case "any":
	                $this->validateAny($value, $schema, $name);
	                break;
	            case "array":
	                $this->validateArray($value, $schema, $name);
	                break;
	            case "boolean":
	                $this->validateBoolean($value, $schema, $name);
	                break;
	            case "object":
	                $this->validateObject($value, $schema, $name);
	                break;
	            case "integer":
	                $this->validateInteger($value, $schema, $name);
	                break;
	            case "number":
	                $this->validateNumber($value, $schema, $name);
	                break;
	            case "string":
	                $this->validateString($value, $schema, $name);
	                break;
	            case "null":
	                $this->validateNull($value, $schema, $name);
	                break;
	            default:
					throw new SchemaException(
					  "%s: The type '%s' is not defined.",
					  $name, $typev);
					break;
	            }
				// Break the foreach loop here because one of the
				// defined types is successfully validated.
				break;
			} catch(SchemaValidationException $e) {
				// Continue with the next type.
			}
        }
	}

	protected function validateAny($value, $schema, $name) {
	}

	protected function validateBoolean($value, $schema, $name) {
		if (!is_bool($value)) {
			throw new SchemaValidationException(
			  "%s: The value '%s' is not a boolean.",
			  $name, $value);
		}
	}

	protected function validateInteger($value, $schema, $name) {
		if (!is_integer($value)) {
			throw new SchemaValidationException(
			  "%s: The value '%s' is not an integer.",
			  $name, $value);
		}
		$this->checkMinimum($value, $schema, $name);
		$this->checkExclusiveMinimum($value, $schema, $name);
		$this->checkMaximum($value, $schema, $name);
		$this->checkExclusiveMaximum($value, $schema, $name);
		$this->checkEnum($value, $schema, $name);
	}

	protected function validateNumber($value, $schema, $name) {
		if (!is_numeric($value)) {
			throw new SchemaValidationException(
			  "%s: The value '%s' is not a number.",
			  $name, $value);
		}
		$this->checkMinimum($value, $schema, $name);
		$this->checkExclusiveMinimum($value, $schema, $name);
		$this->checkMaximum($value, $schema, $name);
		$this->checkExclusiveMaximum($value, $schema, $name);
		$this->checkEnum($value, $schema, $name);
	}

	protected function validateString($value, $schema, $name) {
		if (!is_string($value)) {
			throw new SchemaValidationException(
			  "%s: The value '%s' is not a string.",
			  $name, $value);
		}
		$this->checkPattern($value, $schema, $name);
		$this->checkMinLength($value, $schema, $name);
		$this->checkMaxLength($value, $schema, $name);
		$this->checkFormat($value, $schema, $name);
		$this->checkEnum($value, $schema, $name);
	}

	protected function validateArray($value, $schema, $name) {
		if (!is_array($value)) {
			throw new SchemaValidationException(
			  "%s: The value is not an array.",
			  $name);
		}
		$this->checkMinItems($value, $schema, $name);
		$this->checkMaxItems($value, $schema, $name);
		$this->checkItems($value, $schema, $name);
	}

	protected function validateObject($value, $schema, $name) {
		if (!is_object($value)) {
			throw new SchemaValidationException(
			  "%s: The value is not an object.",
			  $name);
		}
		$this->checkProperties($value, $schema, $name);
	}

	protected function validateNull($value, $schema, $name) {
		if (!is_null($value)) {
			throw new SchemaValidationException(
			  "%s: The value is not NULL.",
			  $name);
		}
	}

	protected function checkMinimum($value, $schema, $name) {
		if (!isset($schema['minimum']))
			return;
		if ($schema['minimum'] > $value) {
			throw new SchemaValidationException(
			  "%s: The value %d is less than %d.",
			  $value, $schema, $name['minimum']);
		}
	}

	protected function checkMaximum($value, $schema, $name) {
		if (!isset($schema['maximum']))
			return;
		if ($schema['maximum'] < $value) {
			throw new SchemaValidationException(
			  "%s: The value %d is bigger than %d.",
			  $value, $schema, $name['maximum']);
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
			  "%s: Invalid value %d, must be greater than %d.",
			  $value, $schema, $name['minimum']);
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
			  "%s: Invalid value %d, must be less than %d.",
			  $value, $schema, $name['maximum']);
		}
	}

	protected function checkMinLength($value, $schema, $name) {
		if (!isset($schema['minLength']))
			return;
		$length = strlen($value);
		if ($schema['minLength'] > $length) {
			throw new SchemaValidationException(
			  "%s: The value '%s' is too short, minimum length is %d.",
			  $value, $schema, $name['minLength']);
		}
	}

	protected function checkMaxLength($value, $schema, $name) {
		if (!isset($schema['maxLength']))
			return;
		$length = strlen($value);
		if ($schema['maxLength'] < $length) {
			throw new SchemaValidationException(
			  "%s: The value '%s' is too long, maximum length is %d.",
			  $value, $schema, $name['maxLength']);
		}
	}

	protected function checkPattern($value, $schema, $name) {
		if (!isset($schema['pattern']))
			return;
		if (!preg_match($schema['pattern'], $value)) {
			throw new SchemaValidationException(
			  "%s: The value '%s' doesn't match the pattern '%s'.",
			  $name, $value, $schema['pattern']);
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
			// ToDo ...
			break;
		case "regex":
			// ToDo ...
			break;
		case "uri":
			if (!filter_var($value, FILTER_VALIDATE_URL,
			  FILTER_FLAG_QUERY_REQUIRED)) {
				throw new SchemaValidationException(
				  "%s: The value '%s' is not a URI.",
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
			  "%s: Enum property is not an array.",
			  $name);
		}
		if (!is_array($value))
			$value = array($value);
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
			  "%s: Not enough array items, minimum is %d",
			  $value, $schema, $name['minItems']);
		}
	}

	protected function checkMaxItems($value, $schema, $name) {
		if (!isset($schema['maxItems']))
			return;
		if (count($value) > $schema['maxItems']) {
			throw new SchemaValidationException(
			  "%s: Too many array items, maximum is %d",
			  $value, $schema, $name['maxItems']);
		}
	}

	protected function checkProperties($value, $schema, $name) {
		if (!isset($schema['properties'])) {
        	throw new SchemaException(
			  "%s: No properties defined.",
			  $name);
        }
		$valueProps = get_object_vars($value);
		foreach ($schema['properties'] as $propk => $propv) {
			// Build the new path. Strip empty parts.
			$parts = array($name, $propk);
			$parts = array_filter($parts, "strlen");
			$path = implode(".", $parts);
			// Check if the 'required' attribute is set.
			if (!array_key_exists($propk, $valueProps)) {
				if (isset($propv['required']) &&
				  (TRUE === $propv['required'])) {
					throw new SchemaValidationException(
					  "%s: Missing required property '%s'.",
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
			  "%s: No items defined.",
			  $name);
        }
        if (is_object($schema['items'])) {
			foreach ($value as $itemk => $itemv) {
                $path = sprintf("%s[%d]", $name, $itemk);
				$valid = TRUE;
                foreach ($schema['items'] as $itemSchema) {
					try {
	                    $this->validateType($itemv, $itemSchema, $path);
					} catch (SchemaValidationException $e) {
						$valid = FALSE;
					}
                }
                if (!$valid) {
                    $types = array_map(function($item) {
                        return $item['type'];
                    }, $schema['items']);
					throw new SchemaValidationException(
					  "%s: Invalid value, must be one of the following types '%s'.",
					  $path, implode(", ", $types));
                }
            }
        } else if (is_array($schema['items'])) {
            foreach ($value as $itemk => $itemv) {
				$path = sprintf("%s[%d]", $name, $itemk);
				$this->validateType($itemv, $schema['items'], $path);
			}
        } else {
			throw new SchemaValidationException(
			  "%s: Invalid items value.",
			  $name);
		}
	}
}
