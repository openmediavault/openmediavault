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

// Set locale information.
setlocale(LC_CTYPE, "");
setlocale(LC_COLLATE, "");
// Set timezone.
try {
	// Make sure the data models are loaded.
	$modelMngr = \OMV\DataModel\Manager::getInstance();
	$modelMngr->load();
	// Get session timeout from the configuration.
	$db = &\OMV\Config\Database::getInstance();
	$object = $db->get("conf.system.time");
	if (in_array($object->get("timezone"), timezone_identifiers_list())) {
		date_default_timezone_set($object->get("timezone"));
	} else {
		// See http://php.net/manual/en/timezones.others.php for more
		// information.
		syslog(LOG_ERR, sprintf("Unknown or unsupported timezone [tz=%s]",
		  $object->get("timezone")));
	}
} catch(\Exception $e) {
	// Nothing to do here.
}
