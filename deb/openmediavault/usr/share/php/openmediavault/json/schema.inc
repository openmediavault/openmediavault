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

require_once("openmediavault/object.inc");

/**
 * Implements a JSON schema validator.
 * @see https://tools.ietf.org/html/draft-zyp-json-schema-03
 * @see http://json-schema.org/latest/json-schema-core.html
 * @see http://json-schema.org/latest/json-schema-validation.html
 */
class Schema extends \OMVObject {
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
	 * @return The JSON schema as associative array or FALSE on failure.
	 */
	final public function getAssoc() {
		if (is_string($this->schema)) {
			$this->schema = json_decode($this->schema, TRUE);
			if (is_null($this->schema)) {
				$this->setLastError(sprintf("Failed to decode schema: %s",
				  json_last_error_msg()));
				return FALSE;
			}
		}
		return $this->schema;
	}

	/**
	 * Returns the JSON schema by the given path.
	 * @return The JSON schema as associative array or FALSE on failure.
	 */
	final public function getAssocByPath($name) {
		// Get the schema.
		if (FALSE === ($schema = $this->getAssoc()))
			return FALSE;
		// Navigate down to the given path.
		if (FALSE === ($schema = $this->_getAssocByPath(
		  $name, $schema))) {
			$this->setLastError(sprintf("The path '%s' is invalid.", $name));
			return FALSE;
		}
		return $schema;
	}

	/**
	 * Validate the given value.
	 * @param value The value to validate.
	 * @param name The JSON path of the entity to validate, e.g. 'aa.bb.cc',
	 *   defaults to an empty string.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	final public function validate($value, $name = "") {
		if (is_string($value))
			$value = json_decode($value);
		if (FALSE === ($schema = $this->getAssocByPath($name)))
			return FALSE;
		return $this->validateType($value, $schema, $name);
	}

	protected function validateType($value, $schema, $name) {
//		$this->debug(var_export(func_get_args(), TRUE));
		$types = "any";
		if (isset($schema['type']))
	        $types = $schema['type'];
		if (!is_array($types))
            $types = array($types);
		$valid = FALSE;
		foreach ($types as $typek => $typev) {
            switch ($typev) {
            case "any":
                $valid = $this->validateAny($value, $schema, $name);
                break;
            case "array":
                $valid = $this->validateArray($value, $schema, $name);
                break;
            case "boolean":
                $valid = $this->validateBoolean($value, $schema, $name);
                break;
            case "object":
                $valid = $this->validateObject($value, $schema, $name);
                break;
            case "integer":
                $valid = $this->validateInteger($value, $schema, $name);
                break;
            case "number":
                $valid = $this->validateNumber($value, $schema, $name);
                break;
            case "string":
                $valid = $this->validateString($value, $schema, $name);
                break;
            case "null":
                $valid = $this->validateNull($value, $schema, $name);
                break;
            default:
				$this->setLastError(
				  sprintf("%s: The type '%s' is not defined.",
				  $name, $typev));
				return FALSE;
            }
			if (TRUE === $valid)
				break;
        }
		return $valid;
	}

	protected function validateAny($value, $schema, $name) {
		return TRUE;
	}

	protected function validateBoolean($value, $schema, $name) {
		if (!is_bool($value)) {
			$this->setLastError(
			  sprintf("%s: The value '%s' is not a boolean.",
			  $name, $value));
			return FALSE;
		}
		return TRUE;
	}

	protected function validateInteger($value, $schema, $name) {
		if (!is_integer($value)) {
			$this->setLastError(
			  sprintf("%s: The value '%s' is not an integer.",
			  $name, $value));
			return FALSE;
		}
		if (!$this->checkMinimum($value, $schema, $name))
			return FALSE;
		if (!$this->checkExclusiveMinimum($value, $schema, $name))
			return FALSE;
		if (!$this->checkMaximum($value, $schema, $name))
			return FALSE;
		if (!$this->checkExclusiveMaximum($value, $schema, $name))
			return FALSE;
		if (!$this->checkEnum($value, $schema, $name))
			return FALSE;
		return TRUE;
	}

	protected function validateNumber($value, $schema, $name) {
		if (!is_numeric($value)) {
			$this->setLastError(
			  sprintf("%s: The value '%s' is not a number.",
			  $name, $value));
			return FALSE;
		}
		if (!$this->checkMinimum($value, $schema, $name))
			return FALSE;
		if (!$this->checkExclusiveMinimum($value, $schema, $name))
			return FALSE;
		if (!$this->checkMaximum($value, $schema, $name))
			return FALSE;
		if (!$this->checkExclusiveMaximum($value, $schema, $name))
			return FALSE;
		if (!$this->checkEnum($value, $schema, $name))
			return FALSE;
		return TRUE;
	}

	protected function validateString($value, $schema, $name) {
		if (!is_string($value)) {
			$this->setLastError(
			  sprintf("%s: The value '%s' is not a string.",
			  $name, $value));
			return FALSE;
		}
		if (!$this->checkPattern($value, $schema, $name))
			return FALSE;
		if (!$this->checkMinLength($value, $schema, $name))
			return FALSE;
		if (!$this->checkMaxLength($value, $schema, $name))
			return FALSE;
		if (!$this->checkFormat($value, $schema, $name))
			return FALSE;
		if (!$this->checkEnum($value, $schema, $name))
			return FALSE;
		return TRUE;
	}

	protected function validateArray($value, $schema, $name) {
		if (!is_array($value)) {
			$this->setLastError(
			  sprintf("%s: The value is not an array.",
			  $name));
			return FALSE;
		}
		if (!$this->checkMinItems($value, $schema, $name))
			return FALSE;
		if (!$this->checkMaxItems($value, $schema, $name))
			return FALSE;
		if (!$this->checkItems($value, $schema, $name))
			return FALSE;
		return TRUE;
	}

	protected function validateObject($value, $schema, $name) {
		if (!is_object($value)) {
			$this->setLastError(
			  sprintf("%s: The value is not an object.",
			  $name));
			return FALSE;
		}
		if (!$this->checkProperties($value, $schema, $name))
			return FALSE;
		return TRUE;
	}

	protected function validateNull($value, $schema, $name) {
		if (!is_null($value)) {
			$this->setLastError(
			  sprintf("%s: The value is not NULL.",
			  $name));
			return FALSE;
		}
		return TRUE;
	}

	protected function checkMinimum($value, $schema, $name) {
		if (!isset($schema['minimum']))
			return TRUE;
		if ($schema['minimum'] > $value) {
			$this->setLastError(
			  sprintf("%s: The value %d is less than %d.",
			  $value, $schema, $name['minimum']));
			return FALSE;
		}
		return TRUE;
	}

	protected function checkMaximum($value, $schema, $name) {
		if (!isset($schema['maximum']))
			return TRUE;
		if ($schema['maximum'] < $value) {
			$this->setLastError(
			  sprintf("%s: The value %d is bigger than %d.",
			  $value, $schema, $name['maximum']));
			return FALSE;
		}
		return TRUE;
	}

	protected function checkExclusiveMinimum($value, $schema, $name) {
		if (!isset($schema['minimum']))
			return TRUE;
		if (!(isset($schema['exclusiveMinimum']) &&
		  (TRUE === $schema['exclusiveMinimum'])))
			return TRUE;
		if ($value == $schema['minimum']) {
			$this->setLastError(
			  sprintf("%s: Invalid value %d, must be greater than %d.",
			  $value, $schema, $name['minimum']));
			return FALSE;
		}
		return TRUE;
	}

	protected function checkExclusiveMaximum($value, $schema, $name) {
		if (!isset($schema['maximum']))
			return TRUE;
		if (!(isset($schema['exclusiveMaximum']) &&
		  (TRUE === $schema['exclusiveMaximum'])))
			return TRUE;
		if ($value == $schema['maximum']) {
			$this->setLastError(
			  sprintf("%s: Invalid value %d, must be less than %d.",
			  $value, $schema, $name['maximum']));
			return FALSE;
		}
		return TRUE;
	}

	protected function checkMinLength($value, $schema, $name) {
		if (!isset($schema['minLength']))
			return TRUE;
		$length = strlen($value);
		if ($schema['minLength'] > $length) {
			$this->setLastError(
			  sprintf("%s: The value '%s' is too short, minimum length is %d.",
			  $value, $schema, $name['minLength']));
			return FALSE;
		}
		return TRUE;
	}

	protected function checkMaxLength($value, $schema, $name) {
		if (!isset($schema['maxLength']))
			return TRUE;
		$length = strlen($value);
		if ($schema['maxLength'] < $length) {
			$this->setLastError(
			  sprintf("%s: The value '%s' is too long, maximum length is %d.",
			  $value, $schema, $name['maxLength']));
			return FALSE;
		}
		return TRUE;
	}

	protected function checkPattern($value, $schema, $name) {
		if (!isset($schema['pattern']))
			return TRUE;
		if (!preg_match($schema['pattern'], $value)) {
			$this->setLastError(
			  sprintf("%s: The value '%s' doesn't match the pattern '%s'.",
			  $name, $value, $schema['pattern']));
			return FALSE;
		}
		return $this;
	}

	/**
	 * @see https://tools.ietf.org/html/draft-zyp-json-schema-03#section-5.23
	 */
	protected function checkFormat($value, $schema, $name) {
		if (!isset($schema['format']))
			return TRUE;
		$valid = TRUE;
		switch ($schema['format']) {
		case "date-time":
			if (!preg_match('/^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$/', $value)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' does not match to ISO 8601.",
				  $name, $value));
			}
			break;
		case "date":
			if (!preg_match('/^\d{4}-\d{2}-\d{2}$/', $value)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' does not match to YYYY-MM-DD.",
				  $name, $value));
			}
			break;
		case "time":
			if (!preg_match('/^\d{2}:\d{2}:\d{2}$/', $value)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' does not match to hh:mm:ss.",
				  $name, $value));
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
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' is not a URI.",
				  $name, $value));
			}
			break;
		case "email":
			if (!filter_var($value, FILTER_VALIDATE_EMAIL)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' is not an email.",
				  $name, $value));
			}
			break;
		case "ip-address":
			if (!filter_var($value, FILTER_VALIDATE_IP, FILTER_FLAG_IPV4)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' is not an IPv4 address.",
				  $name, $value));
			}
			break;
		case "ipv6":
			if (filter_var($value, FILTER_VALIDATE_IP, FILTER_FLAG_IPV6)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' is not an IPv6 address.",
				  $name, $value));
			}
			break;
		default:
			$valid = FALSE;
			$this->setLastError(
			  sprintf("%s: The format '%s' is not defined.",
			  $name, $schema['format']));
			break;
		}
		return $valid;
	}

	protected function checkEnum($value, $schema, $name) {
		if (!isset($schema['enum']))
			return TRUE;
		if (!is_array($schema['enum'])) {
			$this->setLastError(
			  sprintf("%s: Enum property is not an array.",
			  $name));
			return FALSE;
		}
		if (!is_array($value))
			$value = array($value);
		foreach ($value as $valuek => $valuev) {
			if (!in_array($valuev, $schema['enum'])) {
				$this->setLastError(
				  sprintf("%s: Invalid value '%s', allowed values are '%s'.",
				  $name, $valuev, implode(',', $schema['enum'])));
				return FALSE;
			}
		}
		return TRUE;
	}

	protected function checkMinItems($value, $schema, $name) {
		if (!isset($schema['minItems']))
			return TRUE;
		if (count($value) < $schema['minItems']) {
			$this->setLastError(
			  sprintf("%s: Not enough array items, minimum is %d",
			  $value, $schema, $name['minItems']));
			return FALSE;
		}
		return TRUE;
	}

	protected function checkMaxItems($value, $schema, $name) {
		if (!isset($schema['maxItems']))
			return TRUE;
		if (count($value) > $schema['maxItems']) {
			$this->setLastError(
			  sprintf("%s: Too many array items, maximum is %d",
			  $value, $schema, $name['maxItems']));
			return FALSE;
		}
		return TRUE;
	}

	protected function checkProperties($value, $schema, $name) {
		if (!isset($schema['properties'])) {
        	$this->setLastError(
			  sprintf("%s: No properties defined.",
			  $name));
			return FALSE;
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
					$this->setLastError(
					  sprintf("%s: Missing required property '%s'.",
					  $name, $path));
					return FALSE;
                }
				continue;
			}
			if (!$this->validateType($valueProps[$propk], $propv, $path))
				return FALSE;
		}
		return TRUE;
	}

	protected function checkItems($value, $schema, $name) {
		if (!isset($schema['items'])) {
        	$this->setLastError(
			  sprintf("%s: No items defined.",
			  $name));
			return FALSE;
        }
        if (is_array($schema['items'])) {
			foreach ($value as $itemk => $itemv) {
                $path = sprintf("%s[%k]", $name, $itemk);
				$valid = FALSE;
                foreach ($schema['items'] as $itemSchema) {
                    if ($this->validateType($path, $itemv, $itemSchema)) {
						$valid = TRUE;
						break;
					}
                }
                if (!$valid) {
                    $types = array_map(function($item) {
                        return $item['type'];
                    }, $schema['items']);
					$this->setLastError(
					  sprintf("%s: Invalid value, must be one of the following types '%s'.",
					  $path, implode(", ", $types)));
					return FALSE;
                }
            }
        } else if (is_object($schema['items'])) {
            foreach ($value as $itemk => $itemv) {
				$path = sprintf("%s[%k]", $name, $itemk);
				if (!$this->validateType($path, $itemv, $schema['items']))
					return FALSE;
			}
        } else {
			$this->setLastError(
			  sprintf("%s: Invalid items value.",
			  $name));
			return FALSE;
		}
		return TRUE;
	}
}
