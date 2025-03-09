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

class Bcachefs extends BackendAbstract {
	public function __construct() {
		$this->type = "bcachefs";
		$this->properties = self::PROP_MNTENT | self::PROP_POSIX_ACL |
			self::PROP_DISCARD | self::PROP_COMPRESS |
			self::PROP_RESIZE;
		$this->mkfsOptions = \OMV\Environment::get("OMV_MKFS_OPTIONS_BCACHEFS");
		$this->mntOptions = explode(",", \OMV\Environment::get(
		  "OMV_FSTAB_MNTOPS_BCACHEFS"));
	}

	/**
	 * See parent class definition.
	 */
	public function enumerateByBlkid(array $enums) {
		$result = [];
		// !!! Attention !!!
		// We need to keep the following in mind when processing BCACHEFS
		// filesystems:
		// * BCACHEFS will report multiple devices for a RAID.
		// * BCACHEFS seems to randomly use a device file for the canonical
		//   path from a device of the RAID.
		//
		// For RAID devices we need to find out the correct device file
		// and return ONLY that device.
		//
		// To achieve this, we first filter for unique UUIDs and use
		// the /dev/disk/by-uuid/<UUID> device file to get the
		// canonical one. After that we only need to find that device
		// in the enum list given as input argument.

		// First we need to convert all device files to their canonical
		// equivalent, otherwise devices like '/dev/mapper/xxx' won't get
		// found in the enum list given as input argument later.
		foreach ($enums as $enumk => &$enumv) {
			$enumv['devicefile'] = realpath($enumv['devicefile']);
		}
		$uniqueEnums = array_unique_key($enums, "uuid");
		foreach ($uniqueEnums as $uniqueEnumk => $uniqueEnumv) {
			$canonicalDeviceFile = realpath(sprintf("/dev/disk/by-uuid/%s",
				$uniqueEnumv['uuid']));
			$filtered = array_filter_ex($enums, "devicefile",
				$canonicalDeviceFile);
			if (empty($filtered)) {
				// It seems there are situations where the symlink from
				// /dev/disk/by-uuid/<UUID> -> /dev/xxx does not exist.
				//
				// Use findfs as a fallback method to get the device file.
				$cmdArgs = [];
				$cmdArgs[] = sprintf("UUID=%s", $uniqueEnumv['uuid']);
				$cmd = new \OMV\System\Process("findfs", $cmdArgs);
				$cmd->setRedirect2to1();
				$filtered[] = array_merge($uniqueEnumv, [
					"devicefile" => $cmd->execute()
				]);
			}
			$result[] = $filtered[0];
		}
		return $result;
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
		$object = new \OMV\System\Filesystem\Bcachefs($args);
		$object->setBackend($this);
		return $object;
	}
}
