<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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
 * @ingroup api
 * @param array The array to sort
 * @param key The key used as sort criteria
 * @return Returns TRUE on success or FALSE on failure.
 */
function array_sort_key(array &$array, $key) {
	return uasort($array, create_function('$a, $b',
	  "return strnatcmp(strval(\$a['$key']), strval(\$b['$key']));"));
}

/**
 * Filter an array by key and values.
 * @ingroup api
 * @param array The array to sort
 * @param key The key used as filter criteria
 * @param value The value used as filter criteria
 * @return Returns the filtered array.
 */
function array_filter_ex($array, $key, $value) {
	$result = array();
	if(is_array($array) && count($array) > 0) {
		foreach(array_keys($array) as $keyv){
			$temp[$keyv] = $array[$keyv][$key];
			if($temp[$keyv] == $value){
				$result[$keyv] = $array[$keyv];
			}
		}
	}
	return $result;
}

/**
 * Checks if the given keys or index exists in the array.
 * @ingroup api
 * @param keys An array containing the keys to check
 * @param search An array with keys to check
 * @param missing Contains the missing keys if function fails
 * @return Returns TRUE on success or FALSE on failure.
 */
function array_keys_exists($keys, $search, &$missing = NULL) {
	$missing = array_values(array_diff($keys, array_keys($search)));
	return !(count($missing) > 0);
}

/**
 * Convert an object to an array.
 * @ingroup api
 * @param object The object instance.
 * @return Returns an associative array of defined object accessible
 * non-static properties for the specified object  in scope.
 */
function toArray($object) {
	if(is_object($object)) {
	  $object = get_object_vars($object);
	}
	return is_array($object) ? array_map(__FUNCTION__, $object) : $object;
}

/**
 * Get boolean value of an array element.
 * @ingroup api
 * @param array An array with keys.
 * @param key The key of the element.
 * Returns the boolean value of the given key.
 */
function array_boolval($array, $key) {
	if(!isset($array[$key]))
		return false;
	return boolval($array[$key]);
}

/**
 * Get boolean value of a variable.
 * @ingroup api
 * @param var An array with keys.
 * Returns the boolean value of the given variable.
 */
function boolval($var) {
	$result = false;
	// Boolean 'true' => '1'
	switch(strtolower(strval($var))) {
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
 * @ingroup api
 * @param var The variable being evaluated.
 * @return TRUE if variable is a UUID, otherwise FALSE.
 */
function is_uuid($var) {
	if(!is_string($var))
		return FALSE;
	return preg_match('/^[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-'.
	  '[a-f0-9]{12}$/i', $var) ? TRUE : FALSE;
}

/**
 * Finds out whether a variable describes a devicefile, e.g. /dev/sda1.
 * @ingroup api
 * @param var The variable being evaluated.
 * @return TRUE if variable describes a devicefile, otherwise FALSE.
 */
function is_devicefile($var) {
	return preg_match('/^\/dev\/.+$/i', $var) ? TRUE : FALSE;
}

/**
 * Tells whether the given file is a block device.
 * @ingroup api
 * @param filename Path to the file.
 * @return Returns TRUE if the file is a block device, FALSE otherwise.
 */
function is_block($filename) {
	if(FALSE === file_exists($filename))
		return FALSE;
	if(FALSE === ($stat = stat($filename)))
		return FALSE;
	// See http://www.kernel.org/doc/man-pages/online/pages/man2/stat.2.html
	return $stat['mode'] & 060000;
}

/**
 * Finds out whether a variable is JSON.
 * @ingroup api
 * @param var The variable being evaluated.
 * @return TRUE if variable is JSON, otherwise FALSE.
 */
function is_json($var) {
	return (is_string($var) && is_object(json_decode($var))) ?
	  TRUE : FALSE;
}

/**
 * Finds out whether a variable is an IP address.
 * @ingroup api
 * @param var The variable being evaluated.
 * @return TRUE if variable is an IP address, otherwise FALSE.
 */
function is_ipaddr($var) {
	if(!is_string($var))
		return FALSE;
	return filter_var($var, FILTER_VALIDATE_IP));
}

/**
 * Finds out whether a variable is an IPv4 address.
 * @ingroup api
 * @param var The variable being evaluated.
 * @return TRUE if variable is an IPv4 address, otherwise FALSE.
 */
function is_ipaddr4($var) {
	if(!is_string($var))
		return FALSE;
	return filter_var($var, FILTER_VALIDATE_IP, FILTER_FLAG_IPV4));
}

/**
 * Finds out whether a variable is an IPv6 address.
 * @ingroup api
 * @param var The variable being evaluated.
 * @return TRUE if variable is an IPv6 address, otherwise FALSE.
 */
function is_ipaddr6($var) {
	if(!is_string($var))
		return FALSE;
	return filter_var($var, FILTER_VALIDATE_IP, FILTER_FLAG_IPV6));
}

/**
 * Validate JSON data via JSON schema.
 * @ingroup api
 * @param json The JSON string to validate.
 * @param schema The JSON schema to use for validation.
 * @param errors Optional parameter which will contain the error messages
 *   if validation fails.
 * @return TRUE if successful, otherwise FALSE.
 */
function json_validate($json, $schema, &$errors = NULL) {
	$jsonSchema = new JsonSchema($json);
	if(false === ($result = $jsonSchema->validate($schema))) {
		$errors = $jsonSchema->getErrors();
	}
	return $result;
}

/**
 * Returns the JSON representation of a value. All strings will be encoded
 * to UTF-8 before, thus json_encode should not throw an exception like
 * 'Invalid UTF-8 sequence in argument' (Mantis 0000355).
 * @ingroup api
 * @param value The value being encoded.
 * @param options Bitmask, see PHP json_encode manual pages.
 * @return Returns a string containing the JSON representation of \em value.
 */
function json_encode_safe($value, $options = 0) {
	if(is_array($value)) {
		array_walk_recursive($value, create_function('&$item, $key',
		  'if(is_string($item) && !mb_check_encoding($item, "UTF-8")) '.
		  '{ $item = utf8_encode($item); }'));
	} else if(is_string($value)) {
		$value = utf8_encode($value);
	}
	return json_encode($value, $options);
}

/**
 * Convert a number into the highest possible binary unit.
 * @ingroup api
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
	if(is_array($options)) {
		if(isset($options['decimalPlaces']))
			$decimalPlaces = $options['decimalPlaces'];
		if(isset($options['indexed']))
			$indexed = $options['indexed'];
		if(isset($options['fromPrefix']))
			$exp = array_search($options['fromPrefix'], $prefixes);
		if(isset($options['maxPrefix']))
			$maxExp = array_search($options['maxPrefix'], $prefixes);
	}

	$number = strval($number);
	while((-1 != bccomp($number, "1024")) && ($exp < $maxExp)) {
		$exp++;
		$number = bcdiv($number, "1024", $decimalPlaces);
	}

	$result = array(
		"value" => floatval($number),
		"unit" => $prefixes[$exp]
	);
	if(FALSE === $indexed) {
		$result = sprintf("%s %s", $number, $prefixes[$exp]);
	}

	return $result;
}

/**
 * Convert a number to bytes using binary multiples.
 * @ingroup api
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

	if((FALSE === $fromIndex) || (FALSE === $toIndex))
		return FALSE;

	// Convert number to string and ensure the correct number of decimal
	// numbers is set (workaround for strange bcmul behaviour).
	$number = bcadd(strval($number), "0", $decimalPlaces);
	// Convert the given number into the requested binary unit.
	$steps = ($toIndex > $fromIndex) ? $toIndex - $fromIndex :
	  $fromIndex - $toIndex;
	for($i = 0; $i < $steps; $i++) {
		if($fromIndex < $toIndex) {
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
 * @ingroup api
 * @param date A date/time string to parse.
 * @param format The format used in date.
 * @return Returns a Unix timestamp on success, FALSE otherwise.
 */
function strpdate($date, $format) {
	if(FALSE === ($dt = DateTime::createFromFormat($format, $date)))
		return FALSE;
	return $dt->getTimestamp();
}

/**
 * Create a directory with unique file name.
 * @ingroup api
 * @param prefix The prefix of the generated temporary directory.
 * @param mode The mode of the temporary directory. Defaults to 0700.
 * @return Returns the full path of the created directory, or FALSE on failure.
 */
function mkdtemp($prefix = "", $mode = 0700) {
	$pathname = sprintf("%s/%s", sys_get_temp_dir(), uniqid($prefix));
	if(is_dir($pathname))
		return FALSE;
	if(FALSE === mkdir($pathname, $mode))
		return FALSE;
	return $pathname;
}

/**
 * Build a netmask based on the given number of bits.
 * @ingroup api
 * @param bits The number of bits to be set in the desired netmask.
 * @param forceipv6 Set to TRUE to force an IPv6 netmask, even if bits <= 32.
 *   Defaults to FALSE which auto-detects the address family.
 * @return The netmask as string.
 */
function inet_getnetmask($bits, $forceipv6 = FALSE) {
	$af = (32 < $bits) ? AF_INET6 : (TRUE === $forceipv6) ? AF_INET6 : AF_INET;
	$binNetmask = str_pad(str_repeat("1", $bits), ($af == AF_INET6) ?
	  128 : 32, "0", STR_PAD_RIGHT);
	$inaddr = pack("H*", base_convert($binNetmask, 2, 16));
	return inet_ntop($inaddr);
}
?>
