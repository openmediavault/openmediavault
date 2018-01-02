<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2018 Volker Theile
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
namespace OMV\System\Net;

/**
 * This class provides a interface to handle Linux bond network interfaces.
 * @ingroup api
 */
class NetworkInterfaceBond extends NetworkInterface {
	/**
	 * Get the slave devices.
	 * @return A list of device names used in the array, otherwise FALSE.
	 */
	public function getSlaves() {
		$filename = sprintf("/sys/class/net/%s/bonding/slaves",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return file($filename);
	}

	/**
	 * Delete a bond interface.
	 * @return Returns TRUE on success, otherwise FALSE.
	 */
	public function delete() {
		$filename = "/sys/class/net/bonding_masters";
		if (!file_exists($filename))
			return FALSE;
		// See http://backdrift.org/manage-linux-bonding-without-ifenslave-using-sysfs
		$content = sprintf("-%s", $this->getDeviceName());
		return (FALSE !== file_put_contents($filename, $content)) ?
			TRUE : FALSE;
	}

	/**
	 * Get a list of all bond network interfaces.
	 * @return Returns an array containing the bond interface names,
	 *   otherwise FALSE.
	 */
	public static function enumerate() {
		$filename = "/sys/class/net/bonding_masters";
		if (!file_exists($filename))
			return [];
		$content = file_get_contents($filename);
		if (FALSE === $content)
			return FALSE;
		return array_map("trim", explode(" ", $content));
	}

	/**
	 * Get the next free device name.
	 * @return The next free device name, or FALSE on failure.
	 */
	public static function getNextDeviceName() {
		$result = FALSE;
		$deviceNames = self::enumerate();
		if (FALSE === $deviceNames)
			$deviceNames = [];
		for ($i = 0; $i <= 255; $i++) {
			$deviceName = sprintf("bond%d", $i);
			if (FALSE === in_array($deviceName, $deviceNames)) {
				$result = $deviceName;
				break;
			}
		}
		return $result;
	}
}
