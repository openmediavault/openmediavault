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
namespace OMV\System\Filesystem\Backend;

///////////////////////////////////////////////////////////////////////////////
// Register default filesystem backends.
///////////////////////////////////////////////////////////////////////////////
$backends = [ "Bcache", "Btrfs", "Ext", "Ext2", "Ext3", "Ext4", "Hfsplus",
  "Iso9660", "Jfs", "Msdos", "None", "Ntfs", "Reiserfs", "Udf", "Ufs",
  "Umsdos", "Exfat", "Vfat", "Xfs", "Fuseblk", "Swap", "F2fs",
  "Reiser4" ];
$mngr = Manager::getInstance();
foreach ($backends as $backendk => $backendv) {
	$className = sprintf("%s\%s", __NAMESPACE__, $backendv);
	$mngr->registerBackend(new $className());
}
if (TRUE === $debug) {
	$mngr->dump();
}
