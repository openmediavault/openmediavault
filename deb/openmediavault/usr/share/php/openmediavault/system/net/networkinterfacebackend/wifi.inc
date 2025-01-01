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
 * Implements the network interface backend for Wi-Fi devices.
 * @ingroup api
 */
class Wifi extends BackendAbstract {
	// Examples:
	// - wlan1
	// - wlp3s0
	// - wlx984827050421
	//
	// Predictable network interface device name types:
	// - BCMA bus core number
	//   b<number>
	// - CCW bus group name, without leading zeros [s390]
	//   c<bus_id>
	// - on-board device index number
	//   o<index>[n<phys_port_name>|d<dev_port>]
	// - hotplug slot index number
	//   s<slot>[f<function>][n<phys_port_name>|d<dev_port>]
	// - MAC address
	//   x<MAC>
	// - PCI geographical location
	//   [P<domain>]p<bus>s<slot>[f<function>][n<phys_port_name>|d<dev_port>]
	// - USB port number chain
	//   [P<domain>]p<bus>s<slot>[f<function>][u<port>][..][c<config>][i<interface>]
	//
	// Understanding systemd’s predictable network device names:
	// - https://www.freedesktop.org/wiki/Software/systemd/PredictableNetworkInterfaceNames
	// - https://github.com/systemd/systemd/blob/v242/src/udev/udev-builtin-net_id.c
	protected $regex = "/^wlan[0-9]+|wl(b\d+|c\d+|o\d+(n\S+|d\d+)?|".
		"s\d+(f\d+)?(n\S+|d\d+)?|".
		"x[\da-f]{12}|(P\d+)?p\d+s\d+(f\d+)?(n\S+|d\d+)?|".
		"(P\d+)?p\d+s\d+(f\d+)?(u\d+)*(c\d+)?(i\d+)?)$/i";

	function getType() {
		return OMV_NETWORK_INTERFACE_TYPE_WIFI;
	}

	function enumerate() {
		return $this->enumerateSysFs($this->regex);
	}

	function isTypeOf($deviceName) {
		return $this->isTypeOfByName($deviceName, $this->regex);
	}

	function getImpl($args) {
		return new \OMV\System\Net\NetworkInterfaceWifi($args);
	}
}
