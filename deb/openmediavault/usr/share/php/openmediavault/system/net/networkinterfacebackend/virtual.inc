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
 * Implements the network interface backend for virtual devices,
   e.g. TUN/TAP or virtual ethernet devices.
 * @ingroup api
 */
class Virtual extends BackendAbstract {
	// Example:
	// drwxr-xr-x  2 root root 0 Sep 29 10:07 .
	// drwxr-xr-x 50 root root 0 Sep 29 10:07 ..
	// lrwxrwxrwx  1 root root 0 Sep 29 10:07 veth1078b035 -> ../../devices/virtual/net/veth1078b03
	protected $regex = "/^(tun|tap)[0-9]+|veth[a-z0-9]+$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_VIRTUAL;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}

	function getImpl($args) {
		return new \OMV\System\Net\NetworkInterfaceVirtual($args);
	}
}
