<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2020 Volker Theile
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
namespace OMV\System\Storage\Backend;

/**
 * Implements the storage device backend for LVM2.
 * @ingroup api
 */
class LVM extends DM {
	function enumerate() {
		$devs = parent::enumerate();
		if (FALSE === $devs)
			return FALSE;
		$result = [];
		foreach ($devs as $devk => $devv) {
			// Check if the given device is a LVM logical volume.
			$object = new \OMV\System\Storage\StorageDeviceLvm($devv);
			if (!$object->exists())
				continue;
			$result[] = $object->getDeviceFile();
		}
		return $result;
	}

	function enumerateSlaves() {
		$devs = $this->enumerate();
		if (FALSE === $devs)
			return FALSE;
		$result = [];
		foreach ($devs as $devk => $devv) {
			// Process LVM logical volumes.
			$object = $this->getImpl($devv);
			if (!$object->exists())
				continue;
			// Get the physical volumes.
			$slaves = $object->getSlaves();
			if (FALSE !== $slaves)
				$result = array_merge($result, $slaves);
			// Get the volume group.
			$result[] = sprintf("/dev/%s", $object->getVGName());
		}
		return $result;
	}

	function isTypeOf($deviceFile) {
		// Check if the given device is of type device mapper.
		if (FALSE === parent::isTypeOf($deviceFile))
			return FALSE;
		// Check if it is a LVM logical volume.
		// The DM_UUID prefix should be set to subsystem owning the device:
		// LVM, CRYPT, DMRAID, MPATH, PART
		// @see https://github.com/karelzak/util-linux/blob/master/misc-utils/lsblk.c#L389
		$sd = new \OMV\System\Storage\StorageDeviceDM($deviceFile);
		$uuid = $sd->getDeviceMapperUuid();
		if (is_null($uuid)) {
			return FALSE;
		}
		$parts = explode("-", $uuid);
		return "lvm" === mb_strtolower(array_shift($parts));
	}

	function getImpl($args) {
		return new \OMV\System\Storage\StorageDeviceLvm($args);
	}
}
