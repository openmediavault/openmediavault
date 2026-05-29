<?php

/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2026 Volker Theile
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
 * along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * Bootstrap for running OMV PHP unit tests directly from the repository
 * checkout where /etc/default/openmediavault and /usr/share/openmediavault
 * are not available.
 */
$shareDir = realpath(__DIR__ . "/../..");
$unitTestDataDir = realpath(__DIR__ . "/../data");

$GLOBALS["OMV_CONFIGOBJECT_NEW_UUID"] = "fa4b1c66-ef79-11e5-87a0-0002b3a176b4";
$GLOBALS["OMV_DATAMODELS_DIR"] = $shareDir . "/datamodels";
$GLOBALS["OMV_CONFIG_TEMPLATE_FILE"] = $shareDir . "/templates/config.xml";
$GLOBALS["OMV_PRODUCTINFO_FILE"] = $shareDir . "/productinfo.xml";
$GLOBALS["OMV_SCRIPTS_DIR"] = $shareDir . "/scripts";
$GLOBALS["OMV_I18N_LOCALE_DIR"] = $shareDir . "/locale";
$GLOBALS["OMV_CONFIG_FILE"] = $unitTestDataDir . "/config.xml";
