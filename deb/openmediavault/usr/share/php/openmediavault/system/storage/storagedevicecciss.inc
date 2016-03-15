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
namespace OMV\System\Storage;

/**
 * This class provides a simple interface to handle the CCISS hardware RAID
 * devices, e.g. HP's Smart Array hardware RAID controller.
 * @ingroup api
 */
class StorageDeviceCCISS extends StorageDeviceSMARTAbstract {
	public function getDeviceName($canonical = FALSE) {
		// Get the device name and convert '/' character to '!', e.g.
		// cciss/c0d0 => cciss!c0d0.
		return strtr(parent::getDeviceName($canonical), "/", "!");
	}

	public function isRaid() {
		return TRUE;
	}

	/**
	 * Identify the device type required by the smartctl command.
	 * @return Returns the identified device type or an empty string.
	 */
	public function getSMARTDeviceType() {
		$type = parent::getSMARTDeviceType();
		// Note, 'auto' attempts to guess the device type from the device
		// name or from controller type info provided by the operating system
		// or from a matching USB ID entry in the drive database. Thus we
		// have to do the job ourself for various hardware RAID devices.
		if (empty($type) || ("auto" === $type)) {
			if (1 == preg_match("/^cciss!c(\d)d(\d)$/", $this->getDeviceName(),
			  $matches)) {
				$type = $this->setSMARTDeviceType(sprintf("cciss,%s",
				  $matches[2]));
			}
		}
		return $type;
	}
}
