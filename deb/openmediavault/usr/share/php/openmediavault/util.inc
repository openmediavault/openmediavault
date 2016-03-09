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
require_once("openmediavault/autoloader.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/functions.inc");

/**
 * This class provides various helper methods.
 * @ingroup api
 * @deprecated
 */
class OMVUtil {
	/**
	* Create a UUID Version 4 (random).
	* @return A new UUID string if successful, otherwise an exception
	*   is thrown.
	* @deprecated
	*/
	public static function uuid() {
		return \OMV\Uuid::uuid4();
	}

	/**
	* Execute the given command.
	* @deprecated
	*/
	public static function exec($command, &$output = NULL, &$exitStatus = NULL) {
		$cmd = new \OMV\System\Process($command);
		$cmd->setQuiet(TRUE);
		return $cmd->execute($output, $exitStatus);
	}
}
