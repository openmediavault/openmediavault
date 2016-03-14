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
namespace OMV\System\Network\Backend;

require_once("openmediavault/globals.inc");
require_once("openmediavault/functions.inc");

/**
 * Implements the network interface backend for ethernet devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendEthernet extends OMVNetworkInterfaceBackendAbstract {
	protected $regex = "/^(eth|venet)[0-9]+|veth[a-z0-9]+$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_ETHERNET;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}
}

/**
 * Implements the network interface backend for Infiniband devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendInfiniband extends OMVNetworkInterfaceBackendEthernet {
	protected $regex = "/^ib[0-9]+$/i";
}

/**
 * Implements the network interface backend for bond devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendBond extends OMVNetworkInterfaceBackendAbstract {
	protected $regex = "/^bond[0-9]+$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_BOND;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}

	function enumerateSlaves() {
		if (FALSE === ($devs = $this->enumerate()))
			return FALSE;
		$result = array();
		foreach ($devs as $devk => $devv) {
			$object = $this->getImpl($devv);
			if (!$object->exists())
				continue;
			$slaves = $object->getSlaves();
			if (FALSE === $slaves)
				continue;
			$result = array_merge($result, $slaves);
		}
		return $result;
	}

	function getImpl($args) {
		return new OMVNetworkInterfaceBond($args);
	}
}

/**
 * Implements the network interface backend for VLAN devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendVlan extends OMVNetworkInterfaceBackendAbstract {
	protected $regex = "/^(\S+\d+)\.(\d+)$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_VLAN;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}
}

/**
 * Implements the network interface backend for bridge devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendBridge extends OMVNetworkInterfaceBackendAbstract {
	protected $regex = "/^br[0-9]+$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_BRIDGE;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}
}

/**
 * Implements the network interface backend for bridge devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendWireless extends OMVNetworkInterfaceBackendAbstract {
	protected $regex = "/^wlan[0-9]+$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_WIRELESS;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}
}

/**
 * Implements the network interface backend for TUN/TAP devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendVirtual extends OMVNetworkInterfaceBackendAbstract {
	protected $regex = "/^(tun|tap)[0-9]+$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_VIRTUAL;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}
}

/**
 * Implements the network interface backend for loopback devices.
 * @ingroup api
 */
class OMVNetworkInterfaceBackendLoopback extends OMVNetworkInterfaceBackendAbstract {
	protected $regex = "/^lo[0-9]*$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_LOOPBACK;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}
}

/**
 * This class provides a interface to handle Linux bond network interfaces.
 * @ingroup api
 */
class OMVNetworkInterfaceBond extends OMVNetworkInterface {
	/**
	 * Get the slave devices.
	 * @return A list of device names used in the array, otherwise FALSE.
	 */
	public function getSlaves() {
		$filename = sprintf("/sys/class/net/%s/bonding/slaves",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		$slaves = file($filename);
		return $slaves;
	}
}

/**
 * Helper class to get all existing Linux network interfaces.
 * @ingroup api
 */
class OMVNetworkInterfaces extends OMVObject {
	static private $backends = array();

	/**
	 * Register a storage device backend.
	 * @param backend The storage device backend object to register.
	 * @return TRUE if successful, otherwise FALSE.
	 */
	final static function registerBackend(OMVNetworkInterfaceBackendAbstract $backend) {
		if (!isset($backend))
			return FALSE;
		self::$backends[] = $backend;
		return TRUE;
	}

	/**
	 * Get the backend of the given network interface.
	 * @param deviceName Specifies the device name, e.g.
	 *   <ul>
	 *   \li eth0
	 *   \li wlan2
	 *   \li eth2.1
	 *   \li bond0
	 *   </ul>
	 * @return The backend that implements the given device, otherwise NULL.
	 */
	final static function getBackend($deviceName) {
		$result = NULL;
		foreach (self::$backends as $backendk => $backendv) {
			if (FALSE === $backendv->isTypeOf($deviceName))
				continue;
			$result = $backendv;
			break;
		}
		return $result;
	}

	/**
	 * Enumerate all network interfaces.
	 * @param type Defines the network interface type, e.g. ethernet, vlan or
	 *   bond. Defaults to OMV_NETWORK_INTERFACE_TYPE_ALL.
	 * @return A list of network interfaces, e.g. eth0 or lo, otherwise FALSE.
	 */
	public static function enumerate($type = OMV_NETWORK_INTERFACE_TYPE_ALL) {
		$result = array();
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($devs = $backendv->enumerate()))
				return FALSE;
			$result = array_unique(array_merge($result, $devs));
		}
		// Sort the devices using a "natural order" algorithm.
		if (!sort($result, SORT_NATURAL))
			return FALSE;
		return $result;
	}

	/**
	 * Enumerate all unused devices. This list contains all devices of the
	 * given type except the devices that are used by other network interfaces
	 * (e.g. bond interfaces).
	 * @param type Defines the network interface type. Defaults to
	 *   OMV_NETWORK_INTERFACE_TYPE_ALL.
	 * @return A list of network interfaces, otherwise FALSE.
	 */
	final static function enumerateUnused($type = OMV_NETWORK_INTERFACE_TYPE_ALL) {
		// Get all existing network interfaces of the given type.
		if (FALSE === ($result = self::enumerate($type)))
			return FALSE;
		// Remove used network interfaces.
		foreach (self::$backends as $backendk => $backendv) {
// Always collect and remove all slave devices.
//			if (!($type & $backendv->getType()))
//				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (empty($slaves))
				continue;
			$result = array_diff($result, $slaves);
		}
		// Sort the devices using a "natural order" algorithm.
		if (!sort($result, SORT_NATURAL))
			return FALSE;
		return $result;
	}

	/**
	 * Enumerate all used devices. The list contains all those devices that
	 * are used by the given network interfaces, e.g. all members of a bond
	 * interface.
	 * @param type Defines the network interface type. Defaults to
	 *   OMV_NETWORK_INTERFACE_TYPE_ALL.
	 * @return A list of network interfaces, otherwise FALSE.
	 */
	final static function enumerateUsed($type = OMV_NETWORK_INTERFACE_TYPE_ALL) {
		$result = array();
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (empty($slaves))
				continue;
			$result = array_unique(array_merge($result, $slaves));
		}
		// Sort the devices using a "natural order" algorithm.
		if (!sort($result, SORT_NATURAL))
			return FALSE;
		return $result;
	}

	/**
	 * Check if the given device is used by another network interface.
	 * @param deviceName Specifies the device name, e.g.
	 *   <ul>
	 *   \li eth0
	 *   \li wlan2
	 *   \li eth2.1
	 *   \li bond0
	 *   </ul>
	 * @param type Defines the network interface type. Defaults to
	 *   OMV_NETWORK_INTERFACE_TYPE_ALL.
	 * @return TRUE if the given device is used by another network
	 *   interface, otherwise FALSE.
	 */
	final static function isUsed($deviceName, $type = OMV_NETWORK_INTERFACE_TYPE_ALL) {
		$result = FALSE;
		foreach (self::$backends as $backendk => $backendv) {
			if (!($type & $backendv->getType()))
				continue;
			if (FALSE === ($slaves = $backendv->enumerateSlaves()))
				return FALSE;
			if (in_array($deviceName, $slaves)) {
				$result = TRUE;
				break;
			}
		}
		return $result;
	}

	/**
	 * Get the object of the class implementing the given filesystem.
	 * @param deviceName Specifies the device name, e.g.
	 *   <ul>
	 *   \li eth0
	 *   \li wlan2
	 *   \li eth2.1
	 *   \li bond0
	 *   </ul>
	 * @return The object of the class implementing the given network
	 *   interface, otherwise NULL.
	 */
	public static function getImpl($deviceName) {
		if (NULL == ($backend = self::getBackend($deviceName)))
			return NULL;
		return $backend->getImpl($deviceName);
	}
}
