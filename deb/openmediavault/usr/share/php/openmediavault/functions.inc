<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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
			return strnatcmp(strval($a[$_key]), strval($b[$_key])); 
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
 * Convert a number into the highest possible binary unit.
 * @param value The number to convert (per default this is in Bytes).
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
 * @param decimalPlaces The number of decimal places. Defaults to 0.
 * @return The converted number as string or FALSE on failure.
 */
function binary_convert($number, $fromPrefix, $toPrefix, $decimalPlaces = 0) {
	$prefixes = array("B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB",
	  "ZiB", "YiB");
	$fromIndex = array_search($fromPrefix, $prefixes);
	$toIndex = array_search($toPrefix, $prefixes);

	if ((FALSE === $fromIndex) || (FALSE === $toIndex))
		return FALSE;

	// Convert number to string and ensure the correct number of decimal
	// numbers is set (workaround for strange bcmul behaviour).
	$number = bcadd(strval($number), "0", $decimalPlaces);
	// Convert the given number into the requested binary unit.
	$steps = ($toIndex > $fromIndex) ? $toIndex - $fromIndex :
	  $fromIndex - $toIndex;
	for ($i = 0; $i < $steps; $i++) {
		if ($fromIndex < $toIndex) {
			$number = bcdiv($number, "1024", $decimalPlaces);
		} else {
			$number = bcmul($number, "1024", $decimalPlaces);
		}
	}

	return $number;
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
