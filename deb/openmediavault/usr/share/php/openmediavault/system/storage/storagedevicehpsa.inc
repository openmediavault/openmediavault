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
namespace OMV\System\Storage;

/**
 * This class provides a simple interface to handle HP Smart Array RAID
 * controllers using the hpsa driver.
 * @ingroup api
 */
class StorageDeviceHPSA extends StorageDeviceSD {
	public function isRaid() {
		return "RAID" == substr($this->getRaidLevel(), 0, 4);
	}

	/**
	 * See \OMV\System\Storage\SmartInterface interface definition.
	 */
	public function getSmartDeviceType() {
		// Note, we need to distinguish between RAID and HBA mode here.
		if (TRUE === $this->isRaid()) {
			return parent::getSmartDeviceType();
		}
		//
		// $ hpssacli ctrl slot=0 pd all show detail
		// ...
		// physicaldrive 1I:1:5
		// Port: 1I
		// Box: 1
		// Bay: 5
		// Status: OK
		// Disk Name: /dev/sde
		// ...

		// The drive bay is encoded in the devpath of the device via the
		// SCSI address (HCTL), e.g.
		// /devices/pci0000:00/0000:00:02.2/0000:02:00.0/host1/scsi_host/host1/port-1:6/end_device-1:6/target1:0:5/1:0:5:0/block/sde

		// Return 'cciss,N'. The non-negative integer N (in the
		// range from 0 to 15 inclusive) denotes which disk on the
		// controller is monitored.
		$hctl = $this->getHCTL();
		return sprintf("cciss,%d", $hctl['target'] - 1);
	}

	/**
	 * Get the RAID level of the device. If it is a logical device,
	 * then 'N/A' will be returned, otherwise 'RAID 0', 'RAID 1(+0)'
	 * or 'RAID <N>'.
	 *
	 * @link https://www.kernel.org/doc/html/latest/scsi/hpsa.html#hpsa-specific-disk-attributes
	 * @link https://github.com/torvalds/linux/blob/master/drivers/scsi/hpsa.c#L654
	 * @link https://github.com/torvalds/linux/blob/master/drivers/scsi/hpsa.c#L694
	 * @link https://github.com/torvalds/linux/blob/master/drivers/scsi/hpsa.c#L702
	 *
	 * @return string Returns the RAID level.
	 */
	public function getRaidLevel(): string {
		$hctl = $this->getHCTL();
		$filePath = sprintf(
			"/sys/class/scsi_disk/%d:%d:%d:%d/device/raid_level",
			$hctl['host'], $hctl['channel'], $hctl['target'],
			$hctl['lun']);
		if (!file_exists($filePath))
			return "N/A";
		return trim(file_get_contents($filePath));
	}
}
