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
 * Implements the storage device backend for Xen virtual disk devices.
 * @ingroup api
 */
class Xen extends BackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_DISK;
	}

	/**
	 * See interface definition.
	 */
	function enumerate() {
		// major minor  #blocks  name
		// 202        0    6145024 xvda
		// 202        1     104391 xvda1
		// 202        2    6040440 xvda2
		return $this->enumerateProcFs("xvd[a-z]+");
	}

	/**
	 * See interface definition.
	 */
	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/xvda
		// - /dev/xvda1
		// - /dev/xvdtq37
		$regex = "xvd[a-z]+[0-9]*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	/**
	 * See interface definition.
	 */
	function getImpl($args) {
		return new \OMV\System\Storage\StorageDeviceXen($args);
	}

	/**
	 * See interface definition.
	 */
	function baseDeviceFile($deviceFile) {
		return preg_replace("/(\d+)$/", "", $deviceFile);
	}
}
