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

/**
 * @ingroup api
 */
class Uuid {
	private static function uuid($version) {
		$cmdArgs = [];
		$cmdArgs[] = sprintf("-v %d", $version);
		$cmd = new \OMV\System\Process("uuid", $cmdArgs);
		return $cmd->execute();
	}

	/**
	 * Generate a UUID from a host ID, sequence number, and the current time.
	 * @return The UUID string.
	 */
	final public static function uuid1() {
		return self::uuid(1);
	}

	/**
	* Create a UUID Version 4 (random).
	* @return The UUID string.
	*/
	final public static function uuid4() {
		return self::uuid(4);
	}

	final public static function isUuid4($var) {
		if (!is_string($var))
			return FALSE;
		return preg_match('/^[a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-'.
		  '[a-f0-9]{12}$/i', $var) ? TRUE : FALSE;
	}
}
