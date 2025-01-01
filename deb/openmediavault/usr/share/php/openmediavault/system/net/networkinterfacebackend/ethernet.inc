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
 * Implements the network interface backend for ethernet devices.
 * @ingroup api
 */
class Ethernet extends BackendAbstract {
	// Physical ethernet devices
	//
	// Examples:
	// - eth0
	// - enp2s0
	// - enp2s0f0, enp5165p0s0
	// - enx00259025963a
	// - ens1
	// - eno16777736
	// - encf5f0, enx026d3c00000a
	// - end0
	// - enX0
	//
	// Predictable network interface device name types:
	// - BCMA bus core number
	//   b<number>
	// - CCW bus group name, without leading zeros [s390]
	//   c<bus_id>
	// - on-board device index number
	//   o<index>[n<phys_port_name>|d<dev_port>]
	// - hot-plug slot index number
	//   s<slot>[f<function>][n<phys_port_name>|d<dev_port>]
	// - MAC address
	//   x<MAC>
	// - PCI geographical location
	//   [P<domain>]p<bus>s<slot>[f<function>][n<phys_port_name>|d<dev_port>]
	// - USB port number chain
	//   [P<domain>]p<bus>s<slot>[f<function>][u<port>][..][c<config>][i<interface>]
	// - Device tree
	//   d<number>
	// - VIF interface number (Xen)
	//   X<number>
	//
	// Understanding systemd’s predictable network device names:
	// - https://systemd.io/PREDICTABLE_INTERFACE_NAMES
	// - https://www.freedesktop.org/wiki/Software/systemd/PredictableNetworkInterfaceNames
	// - https://github.com/systemd/systemd/blob/v242/src/udev/udev-builtin-net_id.c
	// - https://www.freedesktop.org/software/systemd/man/latest/systemd.net-naming-scheme.html
	protected $regex = "/^eth[0-9]+|".
		"en(b\d+|c\d+|d\d+|o\d+(n\S+|d\d+)?|s\d+(f\d+)?(n\S+|d\d+)?|".
		"x[\da-f]{12}|(P\d+)?p\d+s\d+(f\d+)?(n\S+|d\d+)?|".
		"(P\d+)?p\d+s\d+(f\d+)?(u\d+)*(c\d+)?(i\d+)?|X\d+)$/i";

	// Distributed Switch Architecture (DSA)
	//
	// Examples:
	// - ext0
	// - lan1
	// - wan
	//
	// Documentation:
	// - https://docs.kernel.org/networking/dsa/dsa.html
	// - https://docs.kernel.org/networking/dsa/configuration.html
	protected $regexDsa = "/^(ext|lan)[0-9]+|wan$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_ETHERNET;
	}

	function enumerate() {
		// Enumerate physical ethernet devices.
		$result = $this->enumerateSysFs($this->regex);

		// Enumerate DSA ethernet devices.
		//
		// Example:
		// drwxr-xr-x  2 root root 0 Dec 22  2022 .
		// drwxr-xr-x 67 root root 0 Dec 22  2022 ..
		// lrwxrwxrwx  1 root root 0 Dec 22  2022 eth0 -> ../../devices/platform/scb/fd580000.ethernet/net/eth0
		// lrwxrwxrwx  1 root root 0 Sep 18 17:38 ext1 -> ../../devices/platform/switch0/net/ext1
		// lrwxrwxrwx  1 root root 0 Sep 18 17:38 ext2 -> ../../devices/platform/switch0/net/ext2
		// lrwxrwxrwx  1 root root 0 Sep 18 17:38 lan0 -> ../../devices/platform/switch0/net/lan0
		// lrwxrwxrwx  1 root root 0 Sep 18 17:38 lan1 -> ../../devices/platform/switch0/net/lan1
		// lrwxrwxrwx  1 root root 0 Sep 18 17:38 lan2 -> ../../devices/platform/switch0/net/lan2
		// lrwxrwxrwx  1 root root 0 Dec 22  2022 lo -> ../../devices/virtual/net/lo
		// lrwxrwxrwx  1 root root 0 Sep 18 17:38 wan -> ../../devices/platform/switch0/net/wan
		$result = array_merge($result, $this->enumerateSysFs($this->regexDsa));
		// Remove `Master` devices.
		foreach ([ ...$result ] as $deviceName) {
			$path = sprintf("/sys/class/net/%s/dsa", $deviceName);
			if (is_dir($path)) {
				$result = array_diff($result, [ $deviceName ]);
			}
		}
		return array_values($result); // Re-index array.
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex) ||
			$this->isTypeOfByName($deviceName, $this->regexDsa);
	}

	function getImpl($args) {
		return new \OMV\System\Net\NetworkInterfaceEthernet($args);
	}
}
