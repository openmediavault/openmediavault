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
namespace OMV\System\Storage\Backend;

/**
 * Implements the storage device backend for harddisk devices.
 * @ingroup api
 */
class HDD extends BackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HDD;
	}

	function enumerate() {
		// Examples:
		// - /dev/hda
		// - /dev/sdb
		// - /dev/xvda
		return $this->enumerateProcFs("(h|s|xv)d[a-z]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/hdc2
		// - /dev/sdb
		// - /dev/sda1
		// - /dev/xvda
		// - /dev/xvdb2
		$regex = "(h|s|xv)d[a-z]+[0-9]*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new \OMV\System\Storage\StorageDeviceHDD($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(\d+)$/", "", $deviceFile);
	}
}
