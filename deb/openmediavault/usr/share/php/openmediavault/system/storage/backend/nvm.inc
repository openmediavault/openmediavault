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
 * Implements the storage device backend for Non Volatile Memory (NVM) devices.
 * @ingroup api
 */
class NVM extends BackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_DISK;
	}

	function enumerate() {
		// major minor  #blocks  name
		// 259        0  781412184 nvme0n1
		// 259        1  390705068 nvme0n1p1
		// 259        2  390706008 nvme0n1p2
		return $this->enumerateProcFs("nvme[0-9]+n[0-9]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/nvme<controller>n<namespace>p<partition>
		// - /dev/nvme0n1
		// - /dev/nvme0n1p1
		$regex = "nvme[0-9]+n[0-9]+(p[0-9]+)*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new \OMV\System\Storage\StorageDeviceNVM($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(p\d+)$/", "", $deviceFile);
	}

	function fsDeviceFile($deviceFile, $partition = 1) {
		// E.g. /dev/nvme0n1p1
		return sprintf("%sp%d", $deviceFile, $partition);
	}
}
