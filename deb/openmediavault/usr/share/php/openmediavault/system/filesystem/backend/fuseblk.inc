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

/**
 * This backend is required to support filesystems that are mounted
 * via FUSE.
 *
 * Such filesystems are listed as type 'fuseblk' in /proc/mounts:
 * /dev/sdj1 on /media/43C00A552F032479 type fuseblk (rw,relatime,user_id=0,group_id=0,allow_other,blksize=4096)
 * The blkid command has the following output:
 * /dev/sdj1: UUID="43C00A552F032479" TYPE="ntfs" PARTUUID="6eaa90f6-01"
 */
class Fuseblk extends BackendAbstract {
	public function __construct() {
		$this->type = "fuseblk";
	}
}
