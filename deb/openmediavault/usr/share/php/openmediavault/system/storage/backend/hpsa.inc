<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2019 Volker Theile
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
 * controllers using the hpsa driver.
 * @see https://linux.die.net/man/4/hpsa
 * @ingroup api
 */
class HPSA extends BackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_HBA;
	}

	function enumerate() {
		$result = [];
		// Examples:
		// - /dev/sda
		// - /dev/sdb
		foreach ($this->enumerateProcFs("sd[a-z]+") as $devk => $devv) {
			if ($this->isTypeOf($devv)) {
				$result[] = $devv;
			}
		}
		return $result;
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/sdb
		// - /dev/sda1
		$regex = "sd[a-z]+[0-9]*";
		if (FALSE === $this->isTypeOfByName($deviceFile, $regex)) {
			return FALSE;
		}
		// Check the name of the driver.
		$sd = new \OMV\System\Storage\StorageDevice(
			$this->baseDeviceFile($deviceFile));
		$sd->assertExists();
		return $sd->getDriver() === "hpsa";
	}

	function getImpl($args) {
		return new \OMV\System\Storage\StorageDeviceHPSA($args);
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(\d+)$/", "", $deviceFile);
	}
}
