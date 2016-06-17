<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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
namespace OMV\System;

/**
 * @ingroup api
 */
trait UdevDevice {
	private $udevData = NULL;

	/**
	 * Get the device file, e.g. /dev/sda.
	 * @return The device file.
	 */
	abstract public function getDeviceFile();

	private function getUdevData() {
		// Do we need to get the udev information?
		if (is_null($this->udevData)) {
			$cmdArgs = [];
			$cmdArgs[] = "info";
			$cmdArgs[] = "--query=property";
			$cmdArgs[] = sprintf("--name=%s", escapeshellarg(
			  $this->getDeviceFile()));
			$cmd = new \OMV\System\Process("udevadm", $cmdArgs);
			$cmd->setRedirect2to1();
			$cmd->execute($output);
			// Parse output:
			// UDEV_LOG=3
			// DEVPATH=/devices/pci0000:00/0000:00:10.0/host2/target2:0:1/2:0:1:0/block/sdb
			// MAJOR=8
			// MINOR=16
			// DEVNAME=/dev/sdb
			// DEVTYPE=disk
			// SUBSYSTEM=block
			// ID_SCSI=1
			// ID_VENDOR=VMware_
			// ID_VENDOR_ENC=VMware\x2c\x20
			// ID_MODEL=VMware_Virtual_S
			// ID_MODEL_ENC=VMware\x20Virtual\x20S
			//
			// DEVLINKS=/dev/disk/by-id/ata-IBM-DTTA-341050_WF0WABH1579 /dev/disk/by-id/scsi-SATA_IBM-DTTA-341050_WF0WABH1579 /dev/disk/by-path/pci-0000:00:02.5-scsi-0:0:0:0
			// DEVNAME=/dev/sda
			// DEVPATH=/devices/pci0000:00/0000:00:02.5/host0/target0:0:0/0:0:0:0/block/sda
			// DEVTYPE=disk
			// ID_ATA=1
			// ID_ATA_FEATURE_SET_HPA=1
			// ID_ATA_FEATURE_SET_HPA_ENABLED=1
			// ID_ATA_FEATURE_SET_PM=1
			// ID_ATA_FEATURE_SET_PM_ENABLED=1
			// ID_ATA_FEATURE_SET_SECURITY=1
			// ID_ATA_FEATURE_SET_SECURITY_ENABLED=0
			// ID_ATA_FEATURE_SET_SECURITY_ERASE_UNIT_MIN=22
			// ID_ATA_FEATURE_SET_SMART=1
			// ID_ATA_FEATURE_SET_SMART_ENABLED=0
			// ID_ATA_WRITE_CACHE=1
			// ID_ATA_WRITE_CACHE_ENABLED=1
			// ID_BUS=ata
			// ID_MODEL=IBM-DTTA-351010
			// ID_MODEL_ENC=IBM-DTTA-351010\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20
			// ID_PART_TABLE_TYPE=dos
			// ID_PATH=pci-0000:00:02.5-scsi-0:0:0:0
			// ID_PATH_TAG=pci-0000_00_02_5-scsi-0_0_0_0
			// ID_REVISION=T56OA7A3
			// ID_SCSI_COMPAT=SATA_IBM-DTTA-341050_WF0WABH1579
			// ID_SERIAL=IBM-DTTA-341050_WF0WABH1579
			// ID_SERIAL_SHORT=WF0WFJH1486
			// ID_TYPE=disk
			// MAJOR=8
			// MINOR=0
			// SUBSYSTEM=block
			// UDEV_LOG=3
			// USEC_INITIALIZED=16872806
			$this->udevData = [];
			foreach ($output as $row) {
				$data = explode("=", $row);
				if (empty($data))
					continue;
				$key = trim($data[0]);
				$this->udevData[$key] = $data[1];
			}
		}
	}

	/**
	 * Checks if a udev property exists.
	 * @param id The name of the property, e.g. ID_VENDOR, ID_MODEL or
	 *   ID_SERIAL_SHORT.
	 * @return Returns TRUE if the property exists, otherwise FALSE.
	 */
	public function hasUdevProperty($id) {
		$this->getUdevData();
		return array_key_exists($id, $this->udevData);
	}

	/**
	 * Assert that a udev property exists.
	 * @param id The name of the property, e.g. ID_VENDOR, ID_MODEL or
	 *   ID_SERIAL_SHORT.
	 * @return None.
	 * @throw \OMV\AssertException
	 */
	public function assertUdevProperty($id) {
		if (FALSE === $this->hasUdevProperty($id)) {
			throw new \OMV\AssertException(
			  "The udev property '%s' does not exist.", $id);
		}
	}

	/**
	 * Queries the udev database for device information stored in the udev
	 * database.
	 * @param id The name of the property, e.g. ID_VENDOR, ID_MODEL or
	 *   ID_SERIAL_SHORT.
	 * @return The requested property, otherwise FALSE.
	 */
	public function getUdevProperty($id) {
		$this->getUdevData();
		$this->assertUdevProperty($id);
		return $this->udevData[$id];
	}
}
