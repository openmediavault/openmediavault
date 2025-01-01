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
 * https://www.kernel.org/doc/Documentation/filesystems/f2fs.txt
 */
class F2fs extends BackendAbstract {
	public function __construct() {
		$this->type = "f2fs";
		$this->properties = self::PROP_MNTENT | self::PROP_POSIX_ACL |
			self::PROP_DISCARD | self::PROP_MNTENT_DISCARD |
			self::PROP_QUOTA | self::PROP_RESIZE;
		$this->mkfsOptions = \OMV\Environment::get("OMV_MKFS_OPTIONS_F2FS");
		$this->mntOptions = explode(",", \OMV\Environment::get(
			"OMV_FSTAB_MNTOPS_F2FS"));
	}
}
