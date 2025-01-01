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
 * Implements the storage device backend for MFM/IDE hard disk devices.
 * @see https://linux.die.net/man/4/hd
 * @ingroup api
 */
class HD extends BackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_DISK;
	}

	function enumerate() {
		// Examples:
		// - /dev/hda
		return $this->enumerateProcFs("hd[a-z]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/hdc2
		$regex = "hd[a-z]+[0-9]*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new \OMV\System\Storage\StorageDeviceHD($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(\d+)$/", "", $deviceFile);
	}
}
