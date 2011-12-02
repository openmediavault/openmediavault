<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
require_once("php-json-schema/JsonSchema.php");

/**
 * Search in a multidimensional array.
 */
function array_search_ex($needle, $haystack, $key) {
	if (!is_array($haystack))
		return false;
	foreach ($haystack as $haystackval => $value) {
		$found = false;
		if (is_array($needle) && is_array($key)) {
			foreach ($needle as $n => $needlev) {
				$found = ($value[$key[$n]] === $needlev);
				if (false === $found) {
					break;
				}
			}
		} else {
			if (true === is_array($value[$key])) {
				$found = in_array($needle, $value[$key]);
			} else {
				$found = ($value[$key] === $needle);
			}
		}
		if (true === $found) {
			return $haystackval;
		}
	}
	return false;
}

/**
 * Sort an array by values using a user-defined key.
 * @param array The array to sort
 * @param key The key used as sort criteria
 * @return Returns TRUE on success or FALSE on failure.
 */
function array_sort_key(array &$array, $key) {
	global $_key;

	// Declare local compare function.
	if (!function_exists("_array_sort_key_cmp")) {
		function _array_sort_key_cmp($a, $b) {
			global $_key;
			return strnatcmp($a[$_key], $b[$_key]);
		}
	}

	// Sort array. Set key used my compare function. Finally unset useless global variable.
	$_key = $key;
	$result = uasort($array, "_array_sort_key_cmp");
	unset($_key);

	return $result;
}

/**
 * Filter an array by key and values.
 * @param array The array to sort
 * @param key The key used as filter criteria
 * @param value The value used as filter criteria
 * @return Returns the filtered array.
 */
function array_filter_ex($array, $key, $value) {
	$result = array();
	if (is_array($array) && count($array) > 0) {
		foreach (array_keys($array) as $keyv){
			$temp[$keyv] = $array[$keyv][$key];
			if ($temp[$keyv] == $value){
				$result[$keyv] = $array[$keyv];
			}
		}
	}
	return $result;
}

/**
 * Remove duplicate values by user-defined key.
 * @param array The array to be processed
 * @param key The key used as filter criteria
 */
function array_remove_duplicate($array, $key) {
	$result = array();
	if (!is_array($array))
		return false;
	$subarray = array();
	foreach ($array as $value) {
		$subarray[] = $value[$key];
	}
	foreach (array_unique($subarray) as $k => $value) {
		$result[] = $array[$k];
	}
	return $result;
}

/**
 * Convert an object to an array.
 * @param object The object to convert into an array
 */
function array_from_object($object) {
	if (is_object($object)) {
		$result = array();
		foreach (get_object_vars($object) as $key => $val) {
				$result[$key] = array_from_object($val);
		}
		// If result is empty then return a empty string instead
		return (!empty($result) > 0) ? $result : "";
	} else if (is_array($object)) {
		$result = array();
		foreach ($object as $key => $val) {
			$result[$key] = array_from_object($val);
		}
		// If result is empty then return a empty string instead
		return (!empty($result) > 0) ? $result : "";
	} else {
		return $object;
	}
}

/**
 * Checks if the given keys or index exists in the array.
 * @param keys An array containing the keys to check
 * @param search An array with keys to check
 * @param missing Contains the missing keys if function fails
 * @return Returns TRUE on success or FALSE on failure.
 */
function array_keys_exists($keys, $search, &$missing = NULL) {
	$missing = array_diff($keys, array_keys($search));
	return !(count($missing) > 0);
}

/**
 * Inserts an element in any position of the array.
 * @param array The array to be processed
 * @param value The value to be inserted
 * @param offset The position to insert the value
 * @return Returns the new number of elements in the array.
 */
function array_insert(&$array, $value, $offset) {
	if (is_array($array)) {
		$array  = array_values($array);
		$offset = intval($offset);
		if (($offset < 0) || ($offset >= count($array))) {
			array_push($array, $value);
		} elseif ($offset == 0) {
			array_unshift($array, $value);
		} else {
			$temp = array_slice($array, 0, $offset);
			array_push($temp, $value);
			$array = array_slice($array, $offset);
			$array = array_merge($temp, $array);
		}
	} else {
		$array = array($value);
	}
	return count($array);
}

/**
 * Get list of elements that are not unique.
 * @param array The array to be processed
 * @return Returns a list of elements that are not unique.
 */
function array_not_unique($array = array()) {
	return array_diff_key($array, array_unique($array));
}

/**
 * Convert an object to an array.
 * @param object The object instance.
 * @return Returns an associative array of defined object accessible
 * non-static properties for the specified object  in scope.
 */
function toArray($object) {
	if (is_object($object)) {
	  $object = get_object_vars($object);
	}
	return is_array($object) ? array_map(__FUNCTION__, $object) : $object;
}

/**
 * Convert the given data to an object.
 * @param data The data to convert to an object.
 * @return A object with the array fields as properties.
 */
function toObject($data) {
	return is_array($data) ? (object) array_map(__FUNCTION__, $data) : $data;
}

/**
 * Get boolean value of an array element.
 * @param array An array with keys.
 * @param key The key of the element.
 * Returns the boolean value of the given key.
 */
function array_boolval($array, $key) {
	if (!isset($array[$key]))
		return false;
	return boolval($array[$key]);
}

/**
 * Get boolean value of a variable.
 * @param var  * @param array An array with keys.
 * Returns the boolean value of the given variable.
 */
function boolval($var) {
	$result = false;
	// Boolean 'true' => '1'
	switch (strtolower(strval($var))) {
		case "1":
		case "on":
		case "yes":
		case "y":
		case "true":
			$result = true;
			break;
		default:
			break;
	}
	return $result;
}

/**
 * Finds out whether a variable is an UUID v4.
 * @param var The variable being evaluated.
 * @return TRUE if variable is a UUID, otherwise FALSE.
 */
function is_uuid($var) {
	return preg_match('/^[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-'.
	  '[a-f0-9]{12}$/i', $var) ? TRUE : FALSE;
}

/**
 * Finds out whether a variable describes a devicefile, e.g. /dev/sda1.
 * @param var The variable being evaluated.
 * @return TRUE if variable describes a devicefile, otherwise FALSE.
 */
function is_devicefile($var) {
	return preg_match('/^\/dev\/.+$/i', $var) ? TRUE : FALSE;
}

/**
 * Finds out whether a variable is JSON.
 * @param var The variable being evaluated.
 * @return TRUE if variable is JSON, otherwise FALSE.
 */
function is_json($var) {
	return (is_string($var) && is_object(json_decode($var))) ?
	  TRUE : FALSE;
}

/**
 * Validate JSON data via JSON schema.
 * @param json The JSON string to validate.
 * @param schema The JSON schema to use for validation.
 * @param errors Optional parameter which will contain the error messages
 * if validation fails.
 * @return TRUE if successful, otherwise FALSE.
 */
function json_validate($json, $schema, &$errors = NULL) {
	$jsonSchema = new JsonSchema($json);
	if (false === ($result = $jsonSchema->validate($schema))) {
		$errors = $jsonSchema->getErrors();
	}
	return $result;
}

/**
 * Converts a number of bytes into the highest possible unit.
 * @param value The value to convert.
 * @param options An array of additional options.
 * @return The converted string value including the unit or an indexed
 * array with the fields \em value and \em unit.
 */
function binary_format($number, $options = NULL) {
	$prefixes = array("B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB",
	  "ZiB", "YiB");
	$exp = 0;
	$maxExp = count($prefixes);
	$decimalPlaces = 2;
	$indexed = FALSE;

	// Process additional function options.
	if (is_array($options)) {
		if (isset($options['decimalPlaces']))
			$decimalPlaces = $options['decimalPlaces'];
		if (isset($options['indexed']))
			$indexed = $options['indexed'];
		if (isset($options['fromPrefix']))
			$exp = array_search($options['fromPrefix'], $prefixes);
		if (isset($options['maxPrefix']))
			$maxExp = array_search($options['maxPrefix'], $prefixes);
	}

	$number = strval($number);
	while ((-1 != bccomp($number, "1024")) && ($exp < $maxExp)) {
		$exp++;
		$number = bcdiv($number, "1024", $decimalPlaces);
	}

	$result = array(
		"value" => floatval($number),
		"unit" => $prefixes[$exp]
	);
	if (FALSE === $indexed) {
		$result = sprintf("%s %s", $number, $prefixes[$exp]);
	}

	return $result;
}

/**
 * Convert a number to bytes using binary multiples.
 * @param number The number to convert.
 * @param fromPrefix The binary prefix name \em number is in, e.g. 'KiB'.
 * @param toPrefix The binary prefix name to convert \em number to, e.g.
 * 'TiB'.
 * @return The converted number or FALSE on failure.
 */
function binary_convert($number, $fromPrefix, $toPrefix) {
	$prefixes = array(
		"B"   => 0,
		"KiB" => 10,
		"MiB" => 20,
		"GiB" => 30,
		"TiB" => 40,
		"PiB" => 50,
		"EiB" => 60,
		"ZiB" => 70,
		"YiB" => 80
	);

	if (!array_key_exists($fromPrefix, $prefixes) ||
	  !array_key_exists($toPrefix, $prefixes))
		return FALSE;

	return bcmul(strval($number), bcpow("2", strval($prefixes[$fromPrefix] -
	  $prefixes[$toPrefix])), 0);
}

/**
 * Convert any time/date into a Unix timestamp according to the specified
 * format.
 * @param date A date/time string to parse.
 * @param format The format used in date.
 * @return Returns a Unix timestamp on success, FALSE otherwise.
 */
function strpdate($date, $format) {
	if(FALSE === ($dt = DateTime::createFromFormat($format, $date)))
		return FALSE;
	return $dt->getTimestamp();
}
?>
