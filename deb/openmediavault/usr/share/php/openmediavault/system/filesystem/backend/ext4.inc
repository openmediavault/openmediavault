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

class Ext4 extends Ext3 {
	public function __construct() {
		parent::__construct();
		$this->type = "ext4";
		$this->mkfsOptions = \OMV\Environment::get("OMV_MKFS_OPTIONS_EXT4");
		$this->mntOptions = explode(",", \OMV\Environment::get(
		  "OMV_FSTAB_MNTOPS_EXT4"));
	}

	/**
	 * See parent class definition.
	 */
	public function getMkfsOptions(
	  \OMV\System\Storage\StorageDevice $sd = null) {
		$options = parent::getMkfsOptions($sd);
		if (is_a($sd, "StorageDeviceMd")) {
			// Append extra options for MD (Multiple Device) software
			// RAID devices.
			// http://article.gmane.org/gmane.comp.file-systems.ext4/33938
			// http://serverfault.com/a/419628
			$options .= sprintf(" %s", \OMV\Environment::get(
				"OMV_MKFS_EXTRAOPTIONS_EXT4_MD", ""));
		}
		// Always use 64-bit support on 64bit systems.
		if (TRUE === is_64bits())
			$options .= " -O 64bit";
		return $options;
	}
}
