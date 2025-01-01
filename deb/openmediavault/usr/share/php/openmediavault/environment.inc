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
namespace OMV;

require_once("openmediavault/globals.inc");
require_once("openmediavault/functions.inc");

/**
 * Access the environment variables defined in PHP and within the
 * file /etc/default/openmediavault.
 * @ingroup api
 */
class Environment {
	/**
	 * Get the specified environment variable. If the variable does not exist
	 * and \em default is not NULL, then the specified default value will be
	 * returned, otherwise an exception is thrown.
	 * @param name The name of the environment variable.
	 * @param default The default value. Defaults to NULL.
	 * @return The value of the specified environment variable, or the
	 *   specified default value if the variable does not exist.
	 * @throw \InvalidArgumentException
	 */
	public static function get($name, $default = NULL) {
		if (TRUE === array_key_exists($name, $GLOBALS))
			return $GLOBALS[$name];
		if (TRUE === is_null($default)) {
			throw new \InvalidArgumentException(sprintf(
			  "The environment variable '%s' does not exist.", $name));
		}
		return $default;
	}

	public static function set($name, $value) {
		$GLOBALS[$name] = $value;
	}

	public static function getString($name, $default = NULL) {
		$value = self::get($name, $default);
		return strval($value);
	}

	public static function getBoolean($name, $default = NULL) {
		$value = self::get($name, $default);
		return boolvalEx($value);
	}

	public static function getInteger($name, $default = NULL) {
		$value = self::get($name, $default);
		return intval($value);
	}

	public static function getDouble($name, $default = NULL) {
		$value = self::get($name, $default);
		return doubleval($value);
	}

	public static function getFloat($name, $default = NULL) {
		$value = self::get($name, $default);
		return floatval($value);
	}
}
