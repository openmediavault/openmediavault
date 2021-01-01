<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2021 Volker Theile
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
 * Implements the storage device backend for SCSI disk (block) devices.
 * @see https://linux.die.net/man/4/sd
 * @ingroup api
 */
class SD extends BackendAbstract {
	function getType() {
		return OMV_STORAGE_DEVICE_TYPE_DISK;
	}

	function enumerate() {
		// Examples:
		// - /dev/sdb
		return $this->enumerateProcFs("sd[a-z]+");
	}

	function isTypeOf($deviceFile) {
		// Examples:
		// - /dev/sdb
		// - /dev/sda1
		$regex = "sd[a-z]+[0-9]*";
		return $this->isTypeOfByName($deviceFile, $regex);
	}

	function getImpl($args) {
		$result = NULL;
		switch ($this->getHostDriver($args)) {
		case "arcmsr":
			// Areca RAID controller
			$result = new \OMV\System\Storage\StorageDeviceARCMSR($args);
			break;
		case "hpsa":
			// Logical drives from HP Smart Array RAID controllers
			// are accessed via the SCSI disk driver, so we need to
			// identify such drives because these must be handled
			// different in some cases (e.g. S.M.A.R.T.).
			// @see https://linux.die.net/man/4/hpsa
			$result = new \OMV\System\Storage\StorageDeviceHPSA($args);
			break;
		default:
			$result = new \OMV\System\Storage\StorageDeviceSD($args);
			break;
		}
		return $result;
	}

	function baseDeviceFile($deviceFile) {
		return preg_replace("/(\d+)$/", "", $deviceFile);
	}

	private function getHostDriver($deviceFile) {
		$sd = new \OMV\System\Storage\StorageDevice($deviceFile);
		$sd->assertExists();
		// Try to get the driver via 'driver'.
		$hostPath = sprintf("/sys/block/%s/device/../..",
			$sd->getDeviceName(TRUE));
		$driverPath = realpath(sprintf("%s/../driver", $hostPath));
		if (file_exists($driverPath)) {
			return basename($driverPath);
		}
		// Try to get the driver via 'proc_name'.
		// 'proc_name' is the "name of proc directory" of a driver, if
		// the driver maintained one.
		$realDevicePath = realpath(sprintf("/sys/block/%s",
			$sd->getDeviceName(TRUE)));
		$hostName = "";
		foreach (explode("/", $realDevicePath) as $partk => $partv) {
			if (1 == preg_match("^host\d+$", $partv)) {
				$hostName = $partv;
				break;
			}
		}
		$procNamePath = sprintf("/sys/class/scsi_host/%s/proc_name",
			$hostName);
		if (file_exists($procNamePath)) {
			return trim(file_get_contents($procNamePath));
		}
		return "";
	}
}
