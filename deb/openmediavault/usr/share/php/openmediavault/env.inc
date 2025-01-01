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

// Set locale information.
// Use the system locale for string handling.
setlocale(LC_CTYPE, "");
setlocale(LC_COLLATE, "");
// Use the system locale for formatting date and time values.
setlocale(LC_TIME, "");
// Use the C/POSIX standard for formatting numeric values.
// This is necessary to store numeric values in a locale independent
// format in the database.
// https://www.gnu.org/software/libc/manual/html_node/Standard-Locales.html
// https://en.wikipedia.org/wiki/C_localization_functions
setlocale(LC_NUMERIC, "C");
// Set timezone.
try {
	// Make sure the data models are loaded.
	$modelMngr = \OMV\DataModel\Manager::getInstance();
	$modelMngr->load();
	// Get the configured timezone.
	$db = &\OMV\Config\Database::getInstance();
	$object = $db->get("conf.system.time");
	// According to http://php.net/manual/en/timezones.others.php
	// some of the time zones that the Debian installer supports are
	// deprecated. To prevent errors we support them to (ALL_WITH_BC).
	if (in_array($object->get("timezone"), \DateTimeZone::listIdentifiers(
			\DateTimeZone::ALL_WITH_BC))) {
		date_default_timezone_set($object->get("timezone"));
	} else {
		// See http://php.net/manual/en/timezones.others.php for more
		// information.
		syslog(LOG_ERR, sprintf("Unknown or deprecated time zone [tz=%s]. ".
			"Please reconfigure your time zone settings.",
			$object->get("timezone")));
	}
} catch(\Exception $e) {
	// Nothing to do here.
}
