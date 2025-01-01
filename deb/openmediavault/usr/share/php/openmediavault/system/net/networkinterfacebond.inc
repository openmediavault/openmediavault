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
namespace OMV\System\Net;

require_once("openmediavault/functions.inc");

/**
 * This class provides a interface to handle Linux bond network
 * interfaces.
 * @ingroup api
 */
class NetworkInterfaceBond extends NetworkInterface {
	public function getType() {
		return "bond";
	}

	/**
	 * Get the slave devices.
	 * @return A list of device names used in the array, otherwise FALSE.
	 */
	public function getSlaves() {
		$result = $this->getSysFsProperty($this->getDeviceName(),
			"bonding/slaves", "");
		return array_map("trim", explode_safe(" ", $result));
	}

	/**
	 * Delete the network interface.
	 * @throw \OMV\ExecException
	 */
	public function delete() {
		parent::delete();
		$cmdArgs = [];
		$cmdArgs[] = "link";
		$cmdArgs[] = "del";
		$cmdArgs[] = "dev";
		$cmdArgs[] = escapeshellarg($this->getDeviceName());
		$cmd = new \OMV\System\Process("ip", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Get a list of all bond network interfaces.
	 * @return Returns an array containing the bond interface names.
	 */
	public static function enumerate() {
		$filename = "/sys/class/net/bonding_masters";
		if (!file_exists($filename))
			return [];
		$content = file_get_contents($filename);
		if (FALSE === $content)
			return [];
		return array_map("trim", explode_safe(" ", $content));
	}

	/**
	 * Get the next available device name.
	 * @return Returns a device name, or FALSE on failure.
	 */
	public static function getNextDeviceName() {
		$result = FALSE;
		$deviceNames = self::enumerate();
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
