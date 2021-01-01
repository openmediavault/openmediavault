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
namespace OMV\System\Storage;

/**
 * This class provides a simple interface to handle SCSI disk devices.
 * @ingroup api
 */
class StorageDeviceSD extends StorageDevice implements SmartInterface {
	use SmartTrait;

	/**
	 * Get the current power mode status.
	 * @return string The current power mode status which can be
	 *   <ul>
	 *   \li unknown
	 *   \li active/idle
	 *   \li standby
	 *   \li sleeping
	 *   </ul>
	 * @throw \OMV\ExecException
	 */
	public function getPowerModeState() {
		$result = "unknown";
		// Get the current power mode status.
		$cmdArgs = [];
		$cmdArgs[] = "-C";
		$cmdArgs[] = escapeshellarg($this->getDeviceFile());
		$cmd = new \OMV\System\Process("hdparm", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		// Parse command output:
		// /dev/sda:
		//  drive state is:  active/idle
		//
		// /dev/sdb:
		//  drive state is:  standby
		foreach ($output as $outputk => $outputv) {
			$regex = "/^\s+drive state is:\s+(\S+)$/i";
			if (1 == preg_match($regex, $outputv, $matches)) {
				$result = $matches[1];
				break;
			}
		}
		return $result;
	}

	/**
	 * See \OMV\System\Storage\SmartInterface interface definition.
	 */
	public function getSmartDeviceType() {
		return (TRUE === $this->isUsb()) ? "sat" : "";
	}
}
