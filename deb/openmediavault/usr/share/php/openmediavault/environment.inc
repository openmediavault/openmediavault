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
namespace OMV;

require_once("openmediavault/globals.inc");
require_once("openmediavault/functions.inc");

/**
 * Access the environment variables defined in PHP and within the
 * file /etc/default/openmediavault.
 * @ingroup api
 */
class Environment {
	public static function get($name) {
		if (FALSE === array_key_exists($name, $GLOBALS)) {
			throw new \InvalidArgumentException(sprintf(
			  "The global variable '%s' does not exist.", $name));
		}
		return $GLOBALS[$name];
	}

	public static function set($name, $value) {
		$GLOBALS[$name] = $value;
	}

	public static function getString($name) {
		$value = self::get($name);
		return strval($value);
	}

	public static function getBoolean($name) {
		$value = self::get($name);
		return boolvalEx($value);
	}

	public static function getInteger($name) {
		$value = self::get($name);
		return intval($value);
	}

	public static function getDouble($name) {
		$value = self::get($name);
		return doubleval($value);
	}

	public static function getFloat($name) {
		$value = self::get($name);
		return floatval($value);
	}
}
