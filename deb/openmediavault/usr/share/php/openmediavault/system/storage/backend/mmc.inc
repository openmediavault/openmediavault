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
namespace OMV\System\Storage\Backend;

/**
 * Implements the storage device backend for Multimedia Card (MMC) devices.
 * @ingroup api
 */
class MMC extends BackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_DISK;
	}

	function enumerate() {
		// major minor  #blocks  name
		// 179        0   30948352 mmcblk0
		// 179        1   30947328 mmcblk0p1
		return $this->enumerateProcFs("mmcblk[0-9]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/mmcblk0
		// - /dev/mmcblk0p1
		$regex = "mmcblk[0-9]+(p[0-9]+)*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new \OMV\System\Storage\StorageDeviceMMC($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(p\d+)$/", "", $deviceFile);
	}

	function fsDeviceFile($deviceFile, $partition = 1) {
		// E.g. /dev/mmcblk0p1
		return sprintf("%sp%d", $deviceFile, $partition);
	}
}
