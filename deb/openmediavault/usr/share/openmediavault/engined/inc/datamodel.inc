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
require_once("openmediavault/globals.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/datamodel/manager.inc");

$modelMgr = &\OMV\DataModel\Manager::getInstance();
if (FALSE === $modelMgr->load($GLOBALS['OMV_JSONSCHEMAS_DIR'])) {
	throw new OMVException(OMVErrorMsg::E_DATAMODEL_LOAD_FAILED,
	  $modelMgr->getLastError());
}
if (TRUE === $debug) {
	$modelMgr->dump();
}
