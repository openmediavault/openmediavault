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
namespace OMV\System;

require_once("openmediavault/functions.inc");

/**
 * This class implements an interface to APT (Advanced Packaging Tool).
 * @ingroup api
 */
class Apt {
	/**
	 * Assert that the package database is not locked.
	 * @return void
	 * @throw \OMV\AssertException
	 */
	public static function assertNotLocked(): void {
		if (is_locked("/var/lib/dpkg/lock")) {
			throw new \OMV\AssertException(
				"The package database is held by another process. ".
				"Please try again later.");
		}
		if (is_locked("/var/lib/apt/lists/lock")) {
			throw new \OMV\AssertException(
				"The storage area for state information for each ".
				"package resource is held by another process. ".
				"Please try again later.");
		}
		if (is_locked("/var/cache/apt/archives/lock")) {
			throw new \OMV\AssertException(
				"The package archive is held by another process. ".
				"Please try again later.");
		}
	}
}
