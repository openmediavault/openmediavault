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
namespace OMV\System\Net\NetworkInterfaceBackend;

/**
 * The generic class that represents a network interface backend.
 * @ingroup api
 */
abstract class BackendAbstract {
	/**
	 * Get the type of the network interface backend, e.g.
	 * OMV_NETWORK_INTERFACE_TYPE_ETHERNET, OMV_NETWORK_INTERFACE_TYPE_BOND,
	 * OMV_NETWORK_INTERFACE_TYPE_VLAN, ...
	 */
	abstract function getType();

	/**
	 * Get a list of devices of the given network interface backend.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	abstract function enumerate();

	/**
	 * Get a list of all devices that are used by devices of this
	 * network interface backend.
	 * @return A list of devicefile names, otherwise FALSE.
	 */
	public function enumerateSlaves() {
		return [];
	}

	/**
	* Check whether the given device is represented by this network
	* interface backend.
	* @param deviceName Specifies the device name, e.g.
	*   <ul>
	*   \li eth1
	*   \li bond0
	*   \li eth0.1
	*   \li venet2
	*   </ul>
	* @return TRUE if successful, otherwise FALSE.
	*/
	public function isTypeOf($deviceName) {
		return FALSE;
	}

	/**
	 * Get the object of the class implementing the given network interface.
	 * @param args The arguments to the class constructor.
	 * @return The object of the class implementing the given network
	 *   interface type, otherwise NULL.
	 */
	public function getImpl($args) {
		return new \OMV\System\Net\NetworkInterface($args);
	}

	/**
	 * Helper function to enumerate the network interfaces represented by
	 * this backend via the /sys filesystem.
	 * @param regex The regular expression used to identify the devices
	 *   represented by this backend.
	 * @return A list of network interface names, otherwise FALSE.
	 */
	final protected function enumerateSysFs($regex) {
		$result = [];
		$dir = new \DirectoryIterator("/sys/class/net");
		foreach ($dir as $item) {
			// Skip everything that is not a symlink.
			if ($item->isDot() || !$item->isLink()) {
				continue;
			}
			// Validate network interface name.
			if (1 !== preg_match($regex, $item->getFilename())) {
				continue;
			}
			$result[] = $item->getFilename();
		}
		return $result;
	}

	/**
	 * Helper function to check whether the given device is represented by
	 * this backend.
	 * @param deviceName Specifies the network interface name.
	 * @param regex The regular expression used to identify the devices
	 *   represented by this storage backend.
	 */
	final protected function isTypeOfByName($deviceName, $regex) {
		// Check if the network interface name matches the given regular
		// expression.
		return (1 == preg_match($regex, $deviceName));
	}
}
