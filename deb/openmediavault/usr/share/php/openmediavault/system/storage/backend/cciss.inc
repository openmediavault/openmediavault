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
 * Implements the storage device backend for HP Smart Array RAID
 * controllers using the cciss driver.
 * @ingroup api
 */
class CCISS extends BackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_DISK;
	}

	function enumerate() {
		// major minor  #blocks  name
		// 104     0  292935982 cciss/c0d0
		// 104     1     265041 cciss/c0d0p1
		// 104     2  292664137 cciss/c0d0p2
		// 104    16  586029016 cciss/c0d1
		return $this->enumerateProcFs("cciss\/c[0-9]+d[0-9]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/cciss/c0d0
		// - /dev/cciss/c0d0p2
		$regex = "cciss\/c[0-9]+d[0-9]+(p[0-9]+)?";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		return new \OMV\System\Storage\StorageDeviceCCISS($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(p\d+)$/", "", $deviceFile);
	}

	function fsDeviceFile($deviceFile, $partition = 1) {
		// E.g. /dev/cciss/c0d0p1
		return sprintf("%sp%d", $deviceFile, $partition);
	}
}
