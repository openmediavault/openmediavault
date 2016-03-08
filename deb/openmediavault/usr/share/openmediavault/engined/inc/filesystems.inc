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
require_once("openmediavault/system.inc");

///////////////////////////////////////////////////////////////////////////////
// Register default filesystem backends.
///////////////////////////////////////////////////////////////////////////////
OMVFilesystems::registerBackend(new OMVFilesystemBackendExt4);
OMVFilesystems::registerBackend(new OMVFilesystemBackendExt3);
OMVFilesystems::registerBackend(new OMVFilesystemBackendExt2);
OMVFilesystems::registerBackend(new OMVFilesystemBackendExt);
OMVFilesystems::registerBackend(new OMVFilesystemBackendXfs);
OMVFilesystems::registerBackend(new OMVFilesystemBackendJfs);
OMVFilesystems::registerBackend(new OMVFilesystemBackendVfat);
OMVFilesystems::registerBackend(new OMVFilesystemBackendNtfs);
OMVFilesystems::registerBackend(new OMVFilesystemBackendMsdos);
OMVFilesystems::registerBackend(new OMVFilesystemBackendHfsplus);
OMVFilesystems::registerBackend(new OMVFilesystemBackendBtrfs);
OMVFilesystems::registerBackend(new OMVFilesystemBackendIso9660);
OMVFilesystems::registerBackend(new OMVFilesystemBackendUdf);
OMVFilesystems::registerBackend(new OMVFilesystemBackendReiserfs);
OMVFilesystems::registerBackend(new OMVFilesystemBackendUmsdos);
OMVFilesystems::registerBackend(new OMVFilesystemBackendUfs);
OMVFilesystems::registerBackend(new OMVFilesystemBackendNone);
