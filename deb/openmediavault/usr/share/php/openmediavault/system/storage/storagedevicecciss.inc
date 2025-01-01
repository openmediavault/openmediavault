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
 * Implements the storage device backend for HP Smart Array RAID
 * controllers using the cciss driver.
 * @ingroup api
 */
class StorageDeviceCCISS extends StorageDevice implements SmartInterface {
	use SmartTrait;

	public function getDeviceName($canonical = FALSE) {
		// Get the device name and convert '/' character to '!', e.g.
		// cciss/c0d0 => cciss!c0d0.
		return strtr(parent::getDeviceName($canonical), "/", "!");
	}

	public function isRaid() {
		return TRUE;
	}

	/**
	 * See \OMV\System\Storage\SmartInterface interface definition.
	 */
	public function getSmartDeviceType() {
		$regex = "/^cciss!c(\d)d(\d)$/";
		if (1 == preg_match($regex, $this->getDeviceName(), $matches)) {
			return sprintf("cciss,%s", $matches[2]);
		}
		return "";
	}
}
