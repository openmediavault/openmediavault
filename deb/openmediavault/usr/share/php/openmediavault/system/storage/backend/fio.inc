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
 * Implements the storage device backend for FusionIO devices.
 * @ingroup api
 */
class FIO extends BackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_DISK;
	}

	function enumerate() {
		// major minor  #blocks  name
		//    8        0  312571224 sda
		//    8        1    1052226 sda1
		//    8        2    8393962 sda2
		//  252        0  157143584 fioa
		//  252        1  157139766 fioa1
		return $this->enumerateProcFs("fio[a-z]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/fioa
		// - /dev/fioa1
		$regex = "fio[a-z]+[0-9]*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new \OMV\System\Storage\StorageDeviceFIO($args);
	}
}
