<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2017 Volker Theile
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

class Btrfs extends BackendAbstract {
	public function __construct() {
		$this->type = "btrfs";
		$this->properties = self::PROP_MNTENT | self::PROP_POSIX_ACL |
		  self::PROP_DISCARD | self::PROP_COMPRESS | self::PROP_AUTO_DEFRAG;
		$this->mkfsOptions = \OMV\Environment::get("OMV_MKFS_OPTIONS_BTRFS");
		$this->mntOptions = explode(",", \OMV\Environment::get(
		  "OMV_FSTAB_MNTOPS_BTRFS"));
	}

	/**
	 * See parent class definition.
	 */
	public function enumerateByBlkid(array $enums) {
		// Sort filesystems by device names.
		array_sort_key($enums, "devicefile");
		// Filter filesystems with duplicate UUID, e.g. when they are
		// used in a RAID. Because of the previous sorting always the
		// lower device is used.
		// The following example of a RAID1 will return /dev/sdk.
		// /dev/sdk: LABEL="TestFS" UUID="b7adba40-b980-4067-b33c-dd5c8ee17044" UUID_SUB="7b19094b-e7df-4a3e-a3db-57e2d4c548b3" TYPE="btrfs"
		// /dev/sdl: LABEL="TestFS" UUID="b7adba40-b980-4067-b33c-dd5c8ee17044" UUID_SUB="ba1e2b16-5022-4cbd-ad35-6d609e9560e9" TYPE="btrfs"
		return array_unique_key($enums, "uuid");
	}

	/**
	 * See parent class definition.
	 */
	public function getFstabMntOptions(
	  \OMV\System\Storage\StorageDevice $sd = null) {
		$options = parent::getFstabMntOptions($sd);
		if (!is_null($sd)) {
			if (!$sd->isRotational())
				$options[] = "ssd";
		}
		return $options;
	}

	/**
	 * See parent class definition.
	 */
	function getImpl($args) {
		$object = new \OMV\System\Filesystem\Btrfs($args);
		$object->setBackend($this);
		return $object;
	}
}
