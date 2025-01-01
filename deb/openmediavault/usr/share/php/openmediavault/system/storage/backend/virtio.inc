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
 * Implements the storage device backend for KVM VirtIO devices.
 * @ingroup api
 */
class VirtIO extends BackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_DISK;
	}

	/**
	 * See interface definition.
	 */
	function enumerate() {
		// major minor  #blocks  name
		//  254        0  976762584 vda
		//  254       16  976762584 vdb
		//   11        0     355328 sr0
		//    8        0    8388608 sda
		//    8        1    7993344 sda1
		//    8        2          1 sda2
		//    8        5     392192 sda5
		return $this->enumerateProcFs("vd[a-z]+");
	}

	/**
	 * See interface definition.
	 */
	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/vda
		// - /dev/vda1
		$regex = "vd[a-z]+[0-9]*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	/**
	 * See interface definition.
	 */
	function getImpl($args) {
		return new \OMV\System\Storage\StorageDeviceVirtIO($args);
	}

	/**
	 * See interface definition.
	 */
	function baseDeviceFile($deviceFile) {
		return preg_replace("/(\d+)$/", "", $deviceFile);
	}
}
