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
require_once("openmediavault/autoloader.inc");

/**
 * Collapses a multi-dimensional array into a single dimension using
 * dot notation.
 * @ingroup api
 * @param array $array The array to flatten.
 * @param string $separator The character used as separator. Defaults to '.'.
 * @param string $parent_key The parent key. Defaults to empty.
 * @return A single-dimensional array.
 */
function array_flatten(array $array, $separator = ".", $parent_key = ""): ?array {
	if (!is_array($array))
		return FALSE;
	$result = [];
	foreach ($array as $key => $value) {
		$new_key = !empty($parent_key) ? $parent_key.$separator.$key : $key;
		if (is_array($value))
			$result = array_merge($result, array_flatten($value, $separator,
				$new_key));
		else
			$result[$new_key] = $value;
	}
	return $result;
}

/**
 * Expands a flat array to a multi-dimensional array.
 * @ingroup api
 * @param array $array The array to expand.
 * @param string $separator The character used as separator. Defaults to '.'.
 * @return A multi-dimensional array.
 */
function array_expand(array $array, $separator = "."): ?array {
	if (!is_array($array))
		return FALSE;
	$return = [];
	foreach ($array as $key => $value) {
		$parts = explode($separator, $key);
		$leafKey = array_pop($parts);
		$parent = &$return;
		foreach ($parts as $partk => $partv) {
			if (!isset($parent[$partv]))
				$parent[$partv] = [];
			else if (!is_array($parent[$partv]))
				$parent[$partv] = [];
			$parent = &$parent[$partv];
		}
		if (empty($parent[$leafKey]))
			$parent[$leafKey] = $value;
	}
	return $return;
}

/**
 * Sort an array by values using a user-defined key.
 * @ingroup api
 * @param array The array to sort.
 * @param key The key used as sort criteria.
 * @return Returns TRUE on success or FALSE on failure.
 */
function array_sort_key(array &$array, $key): bool {
	if (!is_multi_array($array)) {
		return FALSE;
	}
	// Sort the array.
	if (FALSE === uasort($array, function($a, $b) use($key) {
		return strnatcmp(strval($a[$key]), strval($b[$key]));
	})) {
		return FALSE;
	}
	// Re-index the array.
	$array = array_values($array);
	return TRUE;
}

/**
 * Remove an key from the given array.
 * @param array $array The array to remove the key from.
 * @param mixed $key The key to be removed.
 * @return Returns TRUE on success, otherwise FALSE.
 */
function array_remove_key(array &$array, $key): bool {
	if (FALSE === array_key_exists($key, $array))
		return FALSE;
	unset($array[$key]);
	return TRUE;
}

/**
 * Remove a value from the given array. The array will be re-indexed
 * automatically.
 * @param array $array The array to process.
 * @param mixed $value The value to be removed.
 * @return Returns TRUE on success, otherwise FALSE.
 */
function array_remove_value(array &$array, $value): bool {
	$index = array_search($value, $array, TRUE);
	if (FALSE === $index)
		return FALSE;
	// Remove value and re-index the array.
	$array = array_values(array_diff($array, [ $value ]));
	return TRUE;
}

/**
 * Filter a multi-dimensional array by key and value.
 * @ingroup api
 * @param array $array The array to process.
 * @param mixed $key The key used as filter criteria.
 * @param mixed $value The value used as filter criteria.
 * @param mixed $default The default value. Defaults to NULL.
 * @return Returns a new array containing only that elements that match
 *   the given filter arguments or the specified default on failure.
 */
function array_filter_ex($array, $key, $value, $default = NULL) {
	if (!is_multi_array($array)) {
		return $default;
	}
	// Re-index the filtered array.
	return array_values(
		array_filter($array, function($v) use($key, $value) {
			if (!array_key_exists($key, $v)) {
				return FALSE;
			}
			return $v[$key] == $value;
		}, ARRAY_FILTER_USE_BOTH));
}

/**
 * Iterates over elements of a multi-dimensional array, returning
 * the first element the key/value matches.
 * @param array $array The multi-dimensional array.
 * @param mixed $key The key of the element.
 * @param mixed $value The value of the element.
 * @param mixed $default The default value. Defaults to FALSE.
 * @return The element the specified key/value matches, otherwise
 *   the given default value.
 */
function array_search_ex(array $array, $key, $value, $default = FALSE) {
	if (!is_multi_array($array)) {
		return $default;
	}
	return array_value(
		array_filter_ex($array, $key, $value),
		0, $default);
}

/**
 * Checks if the given keys or index exists in the array.
 * @ingroup api
 * @param array $keys An array containing the keys to check.
 * @param array $search An array with keys to check.
 * @param mixed $missing An array that contains the missing keys if the
 *   function fails. Defaults to NULL.
 * @return Returns TRUE on success or FALSE on failure.
 */
function array_keys_exists(array $keys, array $search, &$missing = NULL): bool {
	$missing = array_values(array_diff($keys, array_keys($search)));
	return !(count($missing) > 0);
}

/**
 * Removes duplicate values from an multi-dimensional array.
 * Note, the first found duplicate will be inserted to the result.
 * @param array $array The array to process.
 * @param string $key The key used as filter criteria.
 * @return Returns a new array without duplicates or NULL on failure.
 */
function array_unique_key(array $array, $key): ?array {
	if (!is_multi_array($array)) {
		return NULL;
	}
	$result = [];
	$values = [];
	foreach ($array as $arrayk => $arrayv) {
		if (!in_array($arrayv[$key], $values)) {
			$values[] = $arrayv[$key];
			$result[] = $arrayv;
		}
	}
	return $result;
}

/**
 * Convert an object to an array.
 * @ingroup api
 * @param object $object The object instance.
 * @return Returns an associative array of defined object accessible
 *   non-static properties for the specified object in scope.
 */
function toArray($object): array {
	if (is_null($object)) {
		return [];
	}
	if (is_object($object)) {
		$object = get_object_vars($object);
	}
	return array_map(function($value) {
		return (is_array($value) || is_object($value)) ? toArray($value) : $value;
	}, $object);
}

/**
 * Get an array value.
 * @ingroup api
 * @param array $array The array.
 * @param string $key The key of the element.
 * @param mixed $default The default value. Defaults to NULL.
 * @return The value of the given key or the default value if the key
 *   does not exist.
 */
function array_value(array $array, $key, $default = NULL) {
	if (!is_array($array) || !array_key_exists($key, $array))
		return $default;
	return isset($array[$key]) ? $array[$key] : $default;
}

/**
 * Pick one or more random entries out of an array.
 * @ingroup api
 * @param array $array The input array.
 * @param int $num Specifies how many entries should be picked. Defaults to 1.
 * @return mixed When picking only one entry, the function returns the value
 *   for a random entry. Otherwise, an array of values for the random
 *   entries is returned. Trying to pick more elements than there are
 *   in the array NULL will be returned.
 */
function array_rand_value(array $array, $num = 1) {
	$keys = array_rand($array, $num);
	if (is_null($keys))
		return NULL;
	if (!is_array($keys))
		return $array[$keys];
	$result = [];
	foreach ($keys as $keyk => $keyv) {
		$result[] = $array[$keyv];
	}
	return $result;
/*
	// Alternative implementation.
	$count = count($array);
	if ($num > $count)
		return NULL;
	$result = (1 == $num) ? "" : array();
	for ($i = 0; $i < $num; $i++) {
		$index = mt_rand(0, $count - 1);
		if (1 == $num)
			$result = $array[$index];
		else
			$result[] = $array[$index];
	}
	return $result;
*/
}

/**
 * Get boolean value of an array element.
 * @ingroup api
 * @param array $array An array with keys.
 * @param string $key The key of the element.
 * @param string $default The default value returned if not found.
 * @return Returns the boolean value of the given key.
 */
function array_boolval($array, $key, $default = FALSE): bool {
	if (!is_array($array) || !array_key_exists($key, $array)) {
		return $default;
	}
	return boolvalEx($array[$key]);
}

/**
 * Get the boolean value of a variable. A boolean TRUE will be returned for
 * the values 1, '1', 'on', 'yes', 'y' and 'true'.
 * @ingroup api
 * @param mixed $var The scalar value being converted to a boolean.
 * @return Returns the boolean value of the given value.
 */
function boolvalEx($var): bool {
	$result = FALSE;
	// Boolean 'true' => '1'
	switch (mb_strtolower(strval($var))) {
		case "1":
		case "on":
		case "yes":
		case "y":
		case "true":
			$result = TRUE;
			break;
		default:
			break;
	}
	return $result;
}

/**
 * Finds out whether a variable is an UUID v4.
 * @ingroup api
 * @param string $var The variable being evaluated.
 * @return TRUE if the variable is an UUID, otherwise FALSE.
 */
function is_uuid($var): bool {
	return \OMV\Uuid::isUuid4($var);
}

/**
 * Finds out whether a variable is a filesystem UUID, e.g. <ul>
 * \li 78b669c1-9183-4ca3-a32c-80a4e2c61e2d (EXT2/3/4, JFS, XFS)
 * \li 7A48-BA97 (FAT)
 * \li 2ED43920D438EC29 (NTFS)
 * \li 2015-01-13-21-48-46-00 (ISO9660)
 * </ul>
 * @see http://wiki.ubuntuusers.de/UUID
 * @ingroup api
 * @param string $var The variable being evaluated.
 * @return TRUE if the variable is a filesystem UUID, otherwise FALSE.
 */
function is_fs_uuid($var): bool {
	// Check if it is an UUID v4.
	if (is_uuid($var))
		return TRUE;
	// Check if it is a NTFS, FAT or ISO9660 filesystem identifier.
	$regex = '/^([a-f0-9]{4}-[a-f0-9]{4}|[a-f0-9]{16}|[0-9]{4}-[0-9]{2}-'.
	  '[0-9]{2}-[0-9]{2}-[0-9]{2}-[0-9]{2}-[0-9]{2})$/i';
	if (1 == preg_match($regex, $var))
		return TRUE;
	return FALSE;
}

/**
 * Finds out whether a variable describes a device file, e.g. <ul>
 * \li /dev/sda1.
 * </ul>
 * @ingroup api
 * @param string $var The variable being evaluated.
 * @return TRUE if the variable describes a device file, otherwise FALSE.
 */
function is_devicefile($var): bool {
	if (FALSE === is_string($var))
		return FALSE;
	return preg_match('/^\/dev\/.+$/i', $var) ? TRUE : FALSE;
}

/**
 * Finds out whether a variable describes a device file disk/by-xxx, e.g.
 * <ul>
 * \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
 * \li /dev/disk/by-id/usb-Kingston_DataTraveler_G2_001CC0EC21ADF011C6A20E35-0:0-part1
 * \li /dev/disk/by-label/data
 * </ul>
 * @ingroup api
 * @param string $var The variable being evaluated.
 * @return TRUE if the variable describes a device file, otherwise FALSE.
 */
function is_devicefile_by($var): bool {
	if (FALSE === is_string($var))
		return FALSE;
	return preg_match('/^\/dev\/disk\/by-\S+\/.+$/i', $var) ? TRUE : FALSE;
}

/**
 * Finds out whether a variable describes a device file, e.g. <ul>
 * \li /dev/disk/by-uuid/ad3ee177-777c-4ad3-8353-9562f85c0895
 * \li /dev/disk/by-uuid/2ED43920D438EC29 (NTFS)
 * </ul>
 * @ingroup api
 * @param string $var The variable being evaluated.
 * @return TRUE if the variable describes a device file, otherwise FALSE.
 */
function is_devicefile_by_uuid($var): bool {
	if (FALSE === is_string($var))
		return FALSE;
	return preg_match('/^\/dev\/disk\/by-uuid\/.+$/i', $var) ? TRUE : FALSE;
}

/**
 * Finds out whether a variable describes a device file, e.g. <ul>
 * \li /dev/disk/by-id/usb-Kingston_DataTraveler_G2_001CC0EC21ADF011C6A20E35-0:0-part1
 * </ul>
 * @ingroup api
 * @param string $var The variable being evaluated.
 * @return TRUE if the variable describes a device file, otherwise FALSE.
 */
function is_devicefile_by_id($var): bool {
	if (FALSE === is_string($var))
		return FALSE;
	return preg_match('/^\/dev\/disk\/by-id\/.+$/i', $var) ? TRUE : FALSE;
}

/**
 * Finds out whether a variable describes a device file, e.g. <ul>
 * \li /dev/disk/by-label/data
 * </ul>
 * @ingroup api
 * @param string $var The variable being evaluated.
 * @return TRUE if the variable describes a device file, otherwise FALSE.
 */
function is_devicefile_by_label($var): bool {
	if (FALSE === is_string($var))
		return FALSE;
	return preg_match('/^\/dev\/disk\/by-label\/.+$/i', $var) ? TRUE : FALSE;
}

/**
 * Finds out whether a variable describes a device file, e.g. <ul>
 * \li /dev/disk/by-path/pci-0000:00:17.0-ata-3
 * \li /dev/disk/by-path/pci-0000:00:17.0-ata-3-part2
 * </ul>
 * @ingroup api
 * @param string $var The variable being evaluated.
 * @return TRUE if the variable describes a device file, otherwise FALSE.
 */
function is_devicefile_by_path($var): bool {
	if (FALSE === is_string($var))
		return FALSE;
	return preg_match('/^\/dev\/disk\/by-path\/.+$/i', $var) ? TRUE : FALSE;
}

/**
 * Tells whether the given file is a block device.
 * @ingroup api
 * @param string $filename Path to the file.
 * @param bool $dereference Follow links. Defaults to TRUE.
 * @return Returns TRUE if the file is a block device, FALSE otherwise.
 */
function is_block_device($filename, $dereference = TRUE): bool {
	if (FALSE === is_string($filename))
		return FALSE;
	if (FALSE === file_exists($filename))
		return FALSE;
	clearstatcache(TRUE, $filename);
	if (TRUE === $dereference)
		$filename = realpath($filename);
	return ("block" == filetype($filename));
}

/**
 * Tells whether the given file is a block device.
 * @ingroup api
 * @param string $filename Path to the file.
 * @param bool $dereference Follow links. Defaults to TRUE.
 * @return Returns TRUE if the file is a block device, FALSE otherwise.
 */
function is_char_device($filename, $dereference = TRUE): bool {
	if (FALSE === is_string($filename))
		return FALSE;
	if (FALSE === file_exists($filename))
		return FALSE;
	clearstatcache(TRUE, $filename);
	if (TRUE === $dereference)
		$filename = realpath($filename);
	return ("char" == filetype($filename));
}

/**
 * Finds out whether a variable is a JSON encoded string.
 * @ingroup api
 * @param mixed $var The variable being evaluated.
 * @return TRUE if the variable is JSON, otherwise FALSE.
 */
function is_json($var): bool {
	if (!is_string($var))
		return FALSE;
	// An empty string is not a valid JSON value.
	if (empty($var))
		return FALSE;
	// E.g. '1.1.1.1' is identified as JSON by json_decode.
	if (!preg_match("/^[\[\{]/", $var))
		return FALSE;
	if (is_null(json_decode_safe($var)))
		return FALSE;
	return (json_last_error() == JSON_ERROR_NONE);
}

/**
 * Finds out whether a variable is an IP address.
 * @ingroup api
 * @param string $var The variable being evaluated.
 * @return TRUE if the variable is an IP address, otherwise FALSE.
 */
function is_ipaddr($var): bool {
	if (!is_string($var))
		return FALSE;
	return filter_var($var, FILTER_VALIDATE_IP);
}

/**
 * Finds out whether a variable is an IPv4 address.
 * @ingroup api
 * @param string $var The variable being evaluated.
 * @return TRUE if the variable is an IPv4 address, otherwise FALSE.
 */
function is_ipaddr4($var): bool {
	if (!is_string($var))
		return FALSE;
	return filter_var($var, FILTER_VALIDATE_IP, FILTER_FLAG_IPV4);
}

/**
 * Finds out whether a variable is an IPv6 address.
 * @ingroup api
 * @param string $var The variable being evaluated.
 * @return TRUE if the variable is an IPv6 address, otherwise FALSE.
 */
function is_ipaddr6($var): bool {
	if (!is_string($var))
		return FALSE;
	return filter_var($var, FILTER_VALIDATE_IP, FILTER_FLAG_IPV6);
}

/**
 * Finds out whether a variable is a multi-dimensional array.
 * @ingroup api
 * @param mixed $var The variable being evaluated.
 * @return TRUE if the variable is a multi-dimensional array,
 *   otherwise FALSE.
 */
function is_multi_array($var): bool {
	if (FALSE === is_array($var)) {
		return FALSE;
	}
	if (empty($var)) {
		return TRUE;
	}
	if (count($var, COUNT_RECURSIVE) !== count($var)) {
		return TRUE;
	}
	return FALSE;
}

/**
 * Finds out whether a variable is an associative array, e.g.
 * ['foo' => 1, 'bar' => 'baz']
 * @ingroup api
 * @param mixed $var The variable being evaluated.
 * @return TRUE if the variable is an associative array, otherwise FALSE.
 */
function is_assoc_array($var): bool {
	if (FALSE === is_array($var)) {
		return FALSE;
	}
	if (empty($var)) {
		return TRUE;
	}
	$keys = array_keys($var);
	return array_keys($keys) !== $keys;
}

/**
 * Check if the platform is 64bit.
 * @ingroup api
 * @return TRUE if 64bit, otherwise FALSE.
 */
function is_64bits(): bool {
	return (PHP_INT_SIZE == 8);
}

/**
 * Verify that the variable is a closure.
 * @param mixed $var The value to check.
 * @return Returns TRUE if \em $var is a closure, FALSE otherwise.
 */
function is_closure($var): bool {
	try {
    	$reflection = new \ReflectionFunction($var);
    	$result = $reflection->isClosure();
	} catch (\Exception $e) {
		$result = FALSE;
	}
	return $result;
}

/**
 * Validate JSON data via JSON schema.
 * @ingroup api
 * @param string $data The JSON string to validate.
 * @param mixed $schema The JSON schema to use for validation. This can be a
 *   string or an associative array.
 * @param mixed $errors Optional parameter which will contain the error
 *   messages if validation fails. Defaults to NULL.
 * @return TRUE if successful, otherwise FALSE.
 */
function json_validate($data, $schema, &$errors = NULL): bool {
	$result = FALSE;
	try {
		$jsonSchema = new \OMV\Json\Schema($schema);
		$jsonSchema->validate($data);
		$result = TRUE;
	} catch(\Exception $e) {
		$errors = array($e->getMessage());
	}
	return $result;
}

/**
 * Encode the specified expression to UTF-8.
 * @param expression The variable you want to convert.
 * @return Returns the UTF-8 encoded value.
 */
function var_utf8_encode($expression) {
	if (is_array($expression)) {
		foreach ($expression as $exprk => $exprv)
			$expression[$exprk] = var_utf8_encode($exprv);
	} else if (is_object($expression)) {
		foreach ($expression as $exprk => $exprv)
			$expression->$exprk = var_utf8_encode($exprv);
	} else if (is_string($expression)) {
		if (FALSE === mb_detect_encoding($expression, "UTF-8", TRUE))
			$expression = utf8_encode($expression);
	}
	return $expression;
}

/**
 * Returns the JSON representation of a value. All strings will be encoded
 * to UTF-8 before, thus json_encode should not throw an exception like
 * 'Invalid UTF-8 sequence in argument' (Mantis 0000355).
 * @ingroup api
 * @param mixed $value The value being encoded.
 * @param int $options Bitmask, see PHP json_encode manual pages.
 *   Defaults to 0.
 * @return Returns a string containing the JSON representation of \em value.
 */
function json_encode_safe($value, $options = 0) {
	return json_encode(var_utf8_encode($value), $options);
}

/**
* Decodes a JSON string. The JSON string being decoded will be encoded
* to UTF-8 before.
* @ingroup api
* @param string $json The JSON string being decoded.
* @param bool $assoc When TRUE, returned objects will be converted into
*   associative arrays. Defaults to FALSE.
* @param int $depth User specified recursion depth. Defaults to 512.
* @param int $options Bitmask of JSON decode options. Defaults to 0.
* @return Returns the \em value encoded in json in appropriate PHP type.
*   NULL is returned if the json cannot be decoded or if the encoded data
*   is deeper than the recursion limit.
*/
function json_decode_safe($json, $assoc = FALSE, $depth = 512, $options = 0) {
	return json_decode(var_utf8_encode($json), $assoc, $depth, $options);
}

/**
 * Returns the error string of the last JSON API call.
 * @ingroup api
 * @return Returns the error message on success or NULL with wrong parameters.
 */
if (!function_exists('json_last_error_msg')) {
    function json_last_error_msg(): string {
        static $errorMsg = [
			JSON_ERROR_NONE	=> NULL,
			JSON_ERROR_DEPTH =>	"The maximum stack depth has been exceeded",
			JSON_ERROR_STATE_MISMATCH => "Invalid or malformed JSON",
			JSON_ERROR_CTRL_CHAR => "Control character error, possibly incorrectly encoded",
			JSON_ERROR_SYNTAX => "Syntax error",
			JSON_ERROR_UTF8 => "Malformed UTF-8 characters, possibly incorrectly encoded"
        ];
        $errno = json_last_error();
        return array_key_exists($errno, $errorMsg) ? $errorMsg[$errno] :
        	sprintf("Unknown error (%d)", $errno);
    }
}

/**
 * Get the message of the last occurred error.
 * @ingroup api
 * @return Returns the message of the last error if possible,
 *   otherwise `Unknown error`.
 */
function last_error_msg(): string {
	$error = error_get_last();
	return isset($error['message']) ? $error['message'] : "Unknown error";
}

/**
 * Convert a number into the highest possible binary unit.
 * @ingroup api
 * @param mixed $number The number to convert (per default this is in Bytes).
 * @param mixed $options An array of additional options. Defaults to NULL.
 * @return The converted string value including the unit or an indexed
 *   array with the fields \em value and \em unit.
 */
function binary_format($number, $options = NULL) {
	$prefixes = [ "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB" ];
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

	$number = number_format($number, 0, "", "");
	while ((-1 != bccomp($number, "1024")) && ($exp < $maxExp)) {
		$exp++;
		$number = bcdiv($number, "1024", $decimalPlaces);
	}

	$result = [
		"value" => floatval($number),
		"unit" => $prefixes[$exp]
	];
	if (FALSE === $indexed) {
		$result = sprintf("%s %s", $number, $prefixes[$exp]);
	}

	return $result;
}

/**
 * Convert a number to bytes using binary multiples.
 * @ingroup api
 * @param mixed $number The number to convert.
 * @param string $fromPrefix The binary prefix name \em number is in,
 *   e.g. 'KiB'.
 * @param string $toPrefix The binary prefix name to convert \em number to,
 *   e.g. 'TiB'.
 * @param int $decimalPlaces The number of decimal places. Defaults to 0.
 * @return The converted number as string or FALSE on failure.
 */
function binary_convert($number, $fromPrefix, $toPrefix, $decimalPlaces = 0): ?string {
	$prefixes = [ "B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB" ];
	$fromIndex = array_search($fromPrefix, $prefixes);
	$toIndex = array_search($toPrefix, $prefixes);
	if ((FALSE === $fromIndex) || (FALSE === $toIndex))
		return FALSE;
	if (preg_match("/E[\+-]\d+$/", strval($number))) {
		// Convert number to string and ensure the correct number of decimal
		// numbers is set (workaround for float numbers that are otherwise
		// converted to exponential notation, which bcmul cannot handle).
		$number = number_format($number, 32, ".", "");
	}
	// Convert the given number into the requested binary unit.
	$steps = abs($toIndex - $fromIndex);
	$factor = bcpow("1024", $steps);
	if ($fromIndex < $toIndex) {
		$number = bcdiv($number, $factor, $decimalPlaces);
	} else {
		$number = bcmul($number, $factor, $decimalPlaces);
	}
	return $number;
}

/**
 * Convert any time/date into a Unix timestamp according to the specified
 * format.
 * @ingroup api
 * @param string $date A date/time string to parse.
 * @param string $format The format used in date.
 * @return Returns a Unix timestamp on success, FALSE otherwise.
 */
function strpdate($date, $format): ?int {
	if (FALSE === ($dt = DateTime::createFromFormat($format, $date)))
		return FALSE;
	return $dt->getTimestamp();
}

/**
 * Creates a path from a series of parts using separator as the separator
 * between parts. At the boundary between two parts, any trailing occurrences
 * of separator in the first part, or leading occurrences of separator in the
 * second part are removed and exactly one copy of the separator is inserted.
 * @ingroup api
 * @param string $separator The directory seperator, e.g. DIRECTORY_SEPARATOR.
 * @param string $parts The path elements to be concatenated.
 * @return A string path built from the given parts.
 */
function build_path($separator, ...$parts) {
	$parts = array_filter($parts, "strlen");
	$path = implode($separator, $parts);
	$regex = sprintf("/%s+/", preg_quote($separator, "/"));
	return preg_replace($regex, $separator, $path);
}

/**
 * List files inside the specified path.
 * @ingroup api
 * @param string $directory The directory that will be scanned.
 * @param string $extension The file extension of the files to search for.
 * @param int $sortFlags Modify the sorting behavior. Defaults to SORT_NATURAL.
 * @return Returns an array of files from the directory.
 */
function listdir($directory, $extension = "", $sortFlags = SORT_NATURAL): array {
	$files = [];
	if (file_exists($directory)) {
		foreach (new \DirectoryIterator($directory) as $item) {
			if ($item->isDot())
				continue;
			if (!$item->isFile())
				continue;
			if (!empty($extension) && ($extension !== mb_strtolower(
					$item->getExtension())))
				continue;
			$files[] = $item->getPathname();
		}
		sort($files, $sortFlags);
	}
	return $files;
}

/**
 * Create a directory with unique file name.
 * @ingroup api
 * @param string $prefix The prefix of the generated temporary directory.
 *   Defaults to empty.
 * @param int $mode The mode of the temporary directory. Defaults to 0700.
 * @return Returns the full path of the created directory, or FALSE on failure.
 */
function mkdtemp($prefix = "", $mode = 0700): ?string {
	$directory = build_path(DIRECTORY_SEPARATOR, sys_get_temp_dir(),
	  uniqid($prefix));
	if (is_dir($directory))
		return FALSE;
	if (FALSE === mkdir($directory, $mode))
		return FALSE;
	return $directory;
}

/**
 * Build a netmask based on the given number of bits.
 * @ingroup api
 * @param int $bits The number of bits to be set in the desired netmask.
 * @param bool $forceipv6 Set to TRUE to force an IPv6 netmask, even if
 *   bits <= 32. Defaults to FALSE which auto-detects the address family.
 * @return The netmask as string.
 */
function inet_getnetmask($bits, $forceipv6 = FALSE) {
	$af = (32 < $bits) ? AF_INET6 : ((TRUE === $forceipv6) ? AF_INET6 : AF_INET);
	$binNetmask = str_pad(str_repeat("1", $bits), ($af == AF_INET6) ?
	  128 : 32, "0", STR_PAD_RIGHT);
	$inaddr = pack("H*", base_convert($binNetmask, 2, 16));
	return inet_ntop($inaddr);
}

/**
 * Check if the given integer value is in the range min <= var <= max.
 * @ingroup api
 * @param mixed $var The value to check.
 * @param int $min The minimum value of the sequence.
 * @param int $max The maximum value of the sequence.
 * @return TRUE if the value is within the range, otherwise FALSE.
 */
function in_range($var, $min, $max): bool {
	// See http://php.net/manual/en/filter.filters.validate.php.
	return (FALSE !== filter_var($var, FILTER_VALIDATE_INT, [
	  'options' => [
		  'min_range' => $min,
		  'max_range' => $max
	  ]]));
}

/**
 * Compare two UUID v4 strings.
 * @ingroup api
 * @param string $uuid1 The first UUID v4.
 * @param string $uuid2 The second UUID v4.
 * @return Returns TRUE when the two strings are equal, FALSE otherwise.
 */
function uuid_equals($uuid1, $uuid2): bool {
	if (!is_uuid($uuid1) || !is_uuid($uuid2))
		return FALSE;
	return ($uuid1 == $uuid2);
}

/**
 * Compare two file system UUID strings.
 * @ingroup api
 * @see http://wiki.ubuntuusers.de/UUID
 * @param string $uuid1 The first file system UUID.
 * @param string $uuid2 The second file system UUID.
 * @return Returns TRUE when the two strings are equal, FALSE otherwise.
 */
function fs_uuid_equals($uuid1, $uuid2): bool {
	if (!is_fs_uuid($uuid1) || !is_fs_uuid($uuid2))
		return FALSE;
	return ($uuid1 == $uuid2);
}

/**
 * Returns an array with the name of the defined classes that are
 * a subclass of the specified class.
 * @ingroup api
 * @param string $class The class name being checked against.
 * @return Returns an array of the names of the declared classes in the
 *   current script that match the specified criteria.
 */
function get_declared_subclasses($class): array {
	$classNames = [];
	foreach (get_declared_classes() as $className) {
		$wanted = TRUE;
		$classObject = new \ReflectionClass($className);
		if (!$classObject->isSubclassOf($class))
			$wanted = FALSE;
		if ($classObject->isAbstract())
			$wanted = FALSE;
		unset($classObject);
		if (!$wanted)
			continue;
		$classNames[] = $className;
	}
	return $classNames;
}

/**
 * Send a signal to a process and its children.
 * @ingroup api
 * @param pid The process identifier.
 * @param sig One of the PCNTL signals constants.
 * @return Returns TRUE on success or FALSE on failure.
 */
function posix_kill_ex($pid, $sig): bool {
	exec("ps --no-headers -e -o pid,ppid", $output, $exitStatus);
	if ($exitStatus)
		return FALSE;
	foreach ($output as $rowk => $rowv) {
		if (1 !== preg_match('/^\s*(\d+)\s+(\d+)\s*$/', $rowv, $matches))
			continue;
		if (intval($matches[2]) !== $pid)
			continue;
		posix_kill_ex(intval($matches[1]), $sig);
	}
	return posix_kill($pid, $sig);
}

/**
 * Escape a string to be used in a shell environment. Various characters will
 * be replaced with their hexadecimal/octal representation, e.g. a whitespace
 * will be replaced by \x20 or \040.
 * Example:
 * /srv/dev-disk-by-label-xx yy => /srv/dev-disk-by-label-xx\x20yy
 * /srv/dev-disk-by-label-xx yy => /srv/dev-disk-by-label-xx\040yy
 * @param string $var The variable that will be escaped.
 * @param bool $octal If TRUE, convert octal values, otherwise hexadecimal.
 *   Defaults to FALSE.
 * @return string|array The escaped string.
 */
function escape_blank($var, $octal = FALSE) {
	return str_replace(" ", $octal ? "\\040" : "\\x20", $var);
}

/**
 * Escape a path to be used in a shell environment. Various characters will
 * be replaced with their hexadecimal/octal representation, e.g. a whitespace
 * will be replaced by \x20 or \040.
 * Example:
 * /srv/dev-disk-by-label-xx yy => /srv/dev-disk-by-label-xx\x20yy
 * /srv/dev-disk-by-label-xx yy => /srv/dev-disk-by-label-xx\040yy
 * @param string $var The variable that will be escaped.
 * @param bool $octal If TRUE, convert octal values, otherwise hexadecimal.
 *   Defaults to FALSE.
 * @return string|array The escaped string.
 */
function escape_path($var, $octal = FALSE) {
	return escape_blank($var, $octal);
}

/**
 * Escape all percent characters in the given string. This is useful
 * to ensure that the printf, sprintf or vsprintf functions will not
 * interpret this as format directive.
 * @param string $var The variable that will be escaped.
 * @return string|array The escaped string.
 */
function escape_percent($var) {
	return str_replace("%", "%%", $var);
}

/**
 * Unescape a string. Various hexadecimal/octal characters will be replaced
 * with their ASCII representation, e.g. \x20 or \040 will be replaced by
 * a whitespace.
 * Example:
 * /srv/dev-disk-by-label-xx\x20yy => /srv/dev-disk-by-label-xx yy
 * /srv/dev-disk-by-label-xx\040yy => /srv/dev-disk-by-label-xx yy
 * @param string $var The variable that will be unescaped.
 * @param bool $octal If TRUE, convert octal values, otherwise hexadecimal.
 *   Defaults to FALSE.
 * @return string|array The unescaped string.
 */
function unescape_blank($var, $octal = FALSE) {
	return str_replace($octal ? "\\040" : "\\x20", " ", $var);
}

/**
 * Unescape a path. Various hexadecimal/octal characters will be replaced
 * with their ASCII representation, e.g. \x20 or \040 will be replaced by
 * a whitespace.
 * Example:
 * /srv/dev-disk-by-label-xx\x20yy => /srv/dev-disk-by-label-xx yy
 * /srv/dev-disk-by-label-xx\040yy => /srv/dev-disk-by-label-xx yy
 * @param string $var The variable that will be unescaped.
 * @param bool $octal If TRUE, convert octal values, otherwise hexadecimal.
 *   Defaults to FALSE.
 * @return string|array The unescaped string.
 */
function unescape_path($var, $octal = FALSE) {
	return unescape_blank($var, $octal);
}

/**
 * Split a string by a string. If the string is empty, then an empty
 * array is returned.
 * @param string $delimiter The boundary string.
 * @param string $string The input string.
 * @return Returns an array of strings created by splitting the string
 *   parameter on boundaries formed by the delimiter.
 */
function explode_safe($delimiter, $string): array {
	if (empty($string)) {
		return [];
	}
	return explode($delimiter, $string);
}

/**
 * Split a string into an array.
 * @param string $string The string to parse.
 * @param string $seperator The boundary string(s). Defaults to `,;`.
 * @param bool $autoTrim Automatically strip whitespace and other
 *   characters (check the `trim` function for more details) from the
 *   beginning and end of the substrings. Defaults to TRUE.
 * @return Returns an array of strings created by splitting the string
 *   parameter on boundaries formed by the separator or FALSE on
 *   failure.
 */
function explode_csv($string, $separator = ",;", $autoTrim = TRUE): ?array {
    $parts = preg_split("/[".$separator."]+/", $string);
    if (FALSE === $parts) {
        return FALSE;
    }
    if ($autoTrim) {
        $parts = array_map("trim", $parts);
    }
    return $parts;
}

/**
 * Get the major number of the device number.
 * @param int dev The device number.
 * @return Returns the major number of the device number.
 */
function major_dev($dev): int {
	// https://github.com/torvalds/linux/blob/v5.16/include/linux/kdev_t.h
	return ($dev & 0xfff00) >> 8;
}

/**
 * Get the minor number of the device number.
 * @param int dev The device number.
 * @return Returns the minor number of the device number.
 */
function minor_dev($dev): int {
	// https://github.com/torvalds/linux/blob/v5.16/include/linux/kdev_t.h
	return ($dev & 0xff) | (($dev >> 12) & 0xfff00);
}

/**
 * Tells whether the filename has a file lock.
 * @param string $filename Path to the file.
 * @return Returns TRUE if the file is locked, otherwise FALSE.
 */
function is_locked($filename): bool {
	$locked = FALSE;
	if (file_exists($filename)) {
		$stat = stat($filename);
		// https://www.baeldung.com/linux/file-locking#2-proclocks
		$search = sprintf("%02x:%02x:%d", major_dev($stat['dev']),
			minor_dev($stat['dev']), $stat['ino']);
		$locks = file("/proc/locks");
		foreach ($locks as $linek => $linev) {
			if (FALSE !== mb_strpos($linev, $search)) {
				$locked = TRUE;
				break;
			}
		}
	}
	return $locked;
}

class __Defer {
    private $callback;

    public function __construct(callable $callback) {
        $this->callback = $callback;
    }

    public function __destruct() {
        call_user_func($this->callback);
    }
}

/**
 * Schedule a function call to be executed when the surrounding function
 * exits.
 * Note, the function result MUST be assigned to a variable, otherwise
 * the callback function is immediately executed. You may use $_ for that.
 * @param callable $callback The function to be called.
 * @return The instance of the wrapper class that is executing the
 *   callback function when the variable that it is assigned to goes
 *   out of scope.
 */
function defer(callable $callback) {
    return new __Defer($callback);
}

/**
 * Wait until the given condition is met or the timeout is reached.
 * @param int $timeout The maximum time to wait in seconds.
 * @param callable $callback The function to be called.
 * @param string $message The optional message to be thrown if the
 *   timeout is reached.
 * @throws RuntimeException If the timeout is reached.
 */
function waitUntil($timeout, callable $callback, $message = NULL): void {
	for ($i = 0; $i < $timeout; $i++) {
		sleep(1);
		if (TRUE === $callback()) {
			return;
		}
	}
	throw new \RuntimeException(is_null($message) ? sprintf(
		"Timeout reached after %d seconds.", $timeout) : $message);
}
