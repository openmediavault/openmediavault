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
 * This class provides a simple interface to handle Linux network interfaces.
 * @ingroup api
 */
class NetworkInterface {
	protected $name = "";
	protected $ip = [];
	protected $regex = [ // deprecated
		"ipv4" => '\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}',
		"ipv4cidr" => '(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\/(\d{1,2})',
		"ipv6" => '[a-f0-9:.]+',
		"ipv6cidr" => '([a-f0-9:.]+)\/(\d{1,3})',
		"state" => 'UP|DOWN|UNKNOWN'
	];
	private $dataCached = FALSE;

	/**
	 * Constructor
	 * @param name The network interface name, e.g. eth0, ethX, ...
	 */
	public function __construct($name) {
		$this->name = $name;
	}

	protected function isCached() {
		return $this->dataCached;
	}

	protected function setCached($cached) {
		return $this->dataCached = $cached;
	}

	/**
	 * Get the network interface configuration
	 * @private
	 * @throw \OMV\ExecException
	 */
	private function getData() {
		if (FALSE !== $this->isCached())
			return;

		$cmdArgs = [];
		$cmdArgs[] = "-json";
		$cmdArgs[] = "addr";
		$cmdArgs[] = "show";
		$cmdArgs[] = "dev";
		$cmdArgs[] = escapeshellarg($this->getDeviceName());
		$cmd = new \OMV\System\Process("ip", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		// [{
		//     "ifindex": 5,
		//     "link_index": 3,
		//     "ifname": "veth46f8a39b",
		//     "flags": [ "BROADCAST","MULTICAST","UP","LOWER_UP" ],
		//     "mtu": 1500,
		//     "qdisc": "noqueue",
		//     "operstate": "UP",
		//     "group": "default",
		//     "link_type": "ether",
		//     "address": "b2:cc:f1:82:12:f8",
		//     "broadcast": "ff:ff:ff:ff:ff:ff",
		//     "link_netnsid": 0,
		//     "addr_info": [{
		//         "family": "inet",
		//         "local": "172.16.16.1",
		//         "prefixlen": 32,
		//         "scope": "global",
		//         "label": "veth46f8a39b",
		//         "valid_life_time": 4294967295,
		//         "preferred_life_time": 4294967295
		//     },{
		//         "family": "inet6",
		//         "local": "fe80::b0cc:f1ff:fe82:12f8",
		//         "prefixlen": 64,
		//         "scope": "link",
		//         "valid_life_time": 4294967295,
		//         "preferred_life_time": 4294967295
		//     }]
		// }]
		$this->ip = json_decode_safe(implode("", $output), TRUE)[0];
		$this->setCached(TRUE);
	}

	/**
	 * Refresh the cached information.
	 * @return void
	 */
	public function refresh() {
		$this->setCached(FALSE);
		$this->getData();
	}

	/**
	 * Get the interface type, e.g. 'ethernet', 'wifi', 'vlan',
	 * 'bond', 'bridge', 'virtual', 'loopback' or 'unknown'.
	 */
	public function getType() {
		return "unknown";
	}

	/**
	 * Get the description of the network interface device.
	 * @return A description string.
	 */
	public function getDescription() {
		return sprintf("%s [%s]", $this->getDeviceName(),
			$this->getMAC());
	}

	/**
	 * Get the network interface name, e.g. eth0 or ethx.
	 * @return The network interface name.
	 */
	public function getDeviceName() {
		return $this->name;
	}

	/**
	 * Check whether the network interface exists.
	 * @return TRUE if the network interface exists, otherwise FALSE.
	 */
	public function exists() {
		try {
			$this->getData();
		} catch(\Exception $e) {
			return FALSE;
		}
		return TRUE;
	}

	/**
	 * Delete the network interface.
	 * @throw \OMV\ExecException
	 */
	public function delete() {
		// Bring the network interface down.
		$cmdArgs = [];
		$cmdArgs[] = "link";
		$cmdArgs[] = "set";
		$cmdArgs[] = "down";
		$cmdArgs[] = escapeshellarg($this->getDeviceName());
		$cmd = new \OMV\System\Process("ip", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
		// Remove all addresses.
		$cmdArgs = [];
		$cmdArgs[] = "addr";
		$cmdArgs[] = "flush";
		$cmdArgs[] = "dev";
		$cmdArgs[] = escapeshellarg($this->getDeviceName());
		$cmd = new \OMV\System\Process("ip", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute();
	}

	/**
	 * Helper method to get the first found address info matching the
	 * specified family and scope.
	 * @param string $family The address family, e.g. "inet" or "inet6".
	 * @param string $scope The scope, e.g. "global" or "link".
	 * @return The address info as array.
	 * @throw \OMV\ExecException
	 */
	protected function getAddrInfo($family, $scope): array {
		$this->getData();
		$addrInfo = array_value($this->ip, "addr_info", []);
		return array_search_ex(
			array_filter_ex($addrInfo, "family", $family, []),
			"scope", $scope, []);
	}

	/**
	 * Get the network interface IPv4 address.
	 * @return The network interface IPv4 address, otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function getIP() {
		$addrInfo = $this->getAddrInfo("inet", "global");
		return array_value($addrInfo, "local", FALSE);
	}

	/**
	 * Get the network interface IPv6 address.
	 * @return The network interface IPv6 address, otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function getIP6() {
		$addrInfo = $this->getAddrInfo("inet6", "global");
		if (FALSE !== ($ipAddr = array_value(
				$addrInfo, "local", FALSE))) {
			return $ipAddr;
		}
		$addrInfo = $this->getAddrInfo("inet6", "link");
		return array_value($addrInfo, "local", FALSE);
	}

	/**
	 * Get the network interface IPv4 prefix length.
	 * @return The network interface prefix, otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function getPrefix() {
		$addrInfo = $this->getAddrInfo("inet", "global");
		return array_value($addrInfo, "prefixlen", FALSE);
	}

	/**
	 * Get the network interface IPv6 prefix length.
	 * @return The network interface IPv6 mask/prefix length as integer,
	 *   otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function getPrefix6() {
		$addrInfo = $this->getAddrInfo("inet6", "global");
		if (FALSE !== ($ipAddr = array_value(
				$addrInfo, "prefixlen", FALSE))) {
			return $ipAddr;
		}
		$addrInfo = $this->getAddrInfo("inet6", "link");
		return array_value($addrInfo, "prefixlen", FALSE);
	}

	/**
	 * Get the network interface IPv4 netmask.
	 * @return The network interface netmask, otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function getNetmask() {
		if (FALSE === ($prefix = $this->getPrefix()))
			return FALSE;
		return long2ip(ip2long("255.255.255.255") << (32 - $prefix));
	}

	/**
	 * Get the network interface IPv6 mask/prefix length.
	 * @return The network interface IPv6 mask/prefix length as integer,
	 *   otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function getNetmask6() {
		if (FALSE === ($prefix = $this->getPrefix6()))
			return FALSE;
        $netmask = str_repeat("1", $prefix) . str_repeat("0", 128 - $prefix);
        $netmaskOctets = str_split($netmask, 16);
        $hexOctets = array_map(function($binaryOctet) {
                return sprintf('%04x', bindec($binaryOctet));
            },
            $netmaskOctets);
        return implode(':', $hexOctets);
	}

	/**
	 * Get the network interface MAC address.
	 * @return The network interface MAC address, otherwise FALSE.
	 */
	public function getMAC() {
		return $this->getSysFsProperty($this->getDeviceName(),
			"address");
	}

	/**
	 * Get the network interface MTU.
	 * @return The network interface MTU, otherwise FALSE.
	 */
	public function getMTU() {
		return $this->getSysFsProperty($this->getDeviceName(),
			"mtu");
	}

	/**
	 * Get the network interface IPv4 default gateway.
	 * @return The interface default gateway, or FALSE on failure.
	 * @throw \OMV\ExecException
	 */
	public function getGateway() {
		$cmdArgs = [];
		$cmdArgs[] = "-j";
		$cmdArgs[] = "-4";
		$cmdArgs[] = "route";
		$cmdArgs[] = "show";
		$cmdArgs[] = "dev";
		$cmdArgs[] = escapeshellarg($this->getDeviceName());
		$cmd = new \OMV\System\Process("ip", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$output = json_decode_safe(implode("", $output), TRUE);
		// Parse command output:
		// [{
		//     "dst": "default",
		//     "gateway": "192.168.121.1",
		//     "protocol": "dhcp",
		//     "prefsrc": "192.168.121.162",
		//     "metric": 100,
		//     "flags": []
		// },{
		//     "dst": "192.168.121.0/24",
		//     "protocol": "kernel",
		//     "scope": "link",
		//     "prefsrc": "192.168.121.162",
		//     "flags": []
		// },{
		//     "dst": "192.168.121.1",
		//     "protocol": "dhcp",
		//     "scope": "link",
		//     "prefsrc": "192.168.121.162",
		//     "metric": 100,
		//     "flags": []
		// }]
		return array_value(
			array_search_ex($output, "dst", "default", []),
			"gateway", FALSE);
	}

	/**
	 * Get the network interface IPv6 default gateway.
	 * @return The interface default gateway, or FALSE on failure.
	 * @throw \OMV\ExecException
	 */
	public function getGateway6() {
		$cmdArgs = [];
		$cmdArgs[] = "-j";
		$cmdArgs[] = "-6";
		$cmdArgs[] = "route";
		$cmdArgs[] = "show";
		$cmdArgs[] = "dev";
		$cmdArgs[] = escapeshellarg($this->getDeviceName());
		$cmd = new \OMV\System\Process("ip", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$output = json_decode_safe(implode("", $output), TRUE);
		// Parse command output:
		// [{
		//     "dst": "fe80::/64",
		//     "protocol": "kernel",
		//     "metric": 256,
		//     "flags": [ ],
		//     "pref": "medium"
		// }]
		return array_value(
			array_search_ex($output, "dst", "default", []),
			"gateway", FALSE);
	}

	/**
	 * Get the network interface link state.
	 * @return TRUE if link is established, otherwise FALSE.
	 */
	public function getLink() {
		if (FALSE === ($carrier = $this->getCarrier()))
			return FALSE;
		return (1 == $carrier) ? TRUE : FALSE;
	}

	/**
	 * Get the network interface statistics.
	 * @return The network interface statistics, otherwise FALSE.
	 * @code
	 * array(
	 *   rx_bytes => xxx,
	 *   rx_packets => xxx,
	 *   rx_errors => xxx,
	 *   rx_dropped => xxx,
	 *   rx_fifo_errors => xxx,
	 *   rx_frame_errors => xxx,
	 *   rx_compressed => xxx,
	 *   rx_multicast => xxx,
	 *   tx_bytes => xxx,
	 *   tx_packets => xxx,
	 *   tx_errors => xxx,
	 *   tx_dropped => xxx,
	 *   tx_fifo_errors => xxx,
	 *   tx_collisions => xxx,
	 *   tx_carrier_errors => xxx,
	 *   tx_compressed => xxx
	 * )
	 * @endcode
	 */
	public function getStatistics() {
		// Parse command output:
		// Example 1:
		// Inter-|   Receive                                                |  Transmit
		//  face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
		//     lo:  156976     947    0    0    0     0          0         0   156976     947    0    0    0     0       0          0
		//   eth0:162137708  168681    0    0    0     0          0         0 15969317  103565    0    0    0     0       0          0
		//   eth1:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
		//
		// Example 2:
		// Inter-|   Receive                                                |  Transmit
		//  face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
		//     lo:  814023     633    0    0    0     0          0         0   814023     633    0    0    0     0       0          0
		//   eth0: 1697980   10474    0    0    0     0          0      4398   103460     588    0    0    0     0       0          0
		//    wan: 1740240   11291    0    3    0     0          0      4740    84719     537    0    0    0     0       0          0
		//   lan0:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
		//   lan1:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
		//   lan2:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
		//   ext1:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
		//   ext2:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
		$regex = sprintf('/^\s*%s:(.*)$/im', $this->getDeviceName());
		if (1 !== preg_match($regex, file_get_contents("/proc/net/dev"),
				$matches)) {
			return FALSE;
		}
		$data = preg_split("/[\s]+/", $matches[1]);
		return [
			"rx_bytes" => $data[0],
			"rx_packets" => $data[1],
			"rx_errors" => $data[2],
			"rx_dropped" => $data[3],
			"rx_fifo_errors" => $data[4],
			"rx_frame_errors" => $data[5],
			"rx_compressed" => $data[6],
			"rx_multicast" => $data[7],
			"tx_bytes" => $data[8],
			"tx_packets" => $data[9],
			"tx_errors" => $data[10],
			"tx_dropped" => $data[11],
			"tx_fifo_errors" => $data[12],
			"tx_collisions" => $data[13],
			"tx_carrier_errors" => $data[14],
			"tx_compressed" => $data[15]
		];
	}

	/**
	 * Get the network interface operation state.
	 * @return The network interface state, e.g. 'NOTPRESENT', 'DOWN',
	 *   'LOWERLAYERDOWN', 'TESTING', 'DORMANT', 'UP' or 'UNKNOWN'.
	 */
	public function getState() {
		return mb_strtoupper($this->getOperState());
	}

	/**
	 * Get the network interface operation state, e.g. 'unknown',
	 * 'notpresent', 'down', 'lowerlayerdown', 'testing', 'dormant'
	 * or 'up'.
	 * @see https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
	 * @return The network interface operation state, otherwise 'unknown'.
	 */
	public function getOperState() {
		return $this->getSysFsProperty($this->getDeviceName(),
			"operstate", "unknown");
	}

	/**
	 * Get the network interface duplex value.
	 * @see https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
	 * @return The network interface duplex value as string, otherwise FALSE.
	 */
	public function getDuplex() {
		return $this->getSysFsProperty($this->getDeviceName(),
			"duplex");
	}

	/**
	 * Get the network interface speed value.
	 * @see https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
	 * @return The network interface speed as number in Mbits/sec,
	 *   otherwise FALSE.
	 */
	public function getSpeed() {
		return $this->getSysFsPropertyInt($this->getDeviceName(),
			"speed");
	}

	/**
	 * Get the network interface physical link state.
	 * @see https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
	 * @return The network interface physical link state, otherwise FALSE.
	 */
	public function getCarrier() {
		return $this->getSysFsPropertyInt($this->getDeviceName(),
			"carrier");
	}

	/**
	 * Get the system-wide interface unique index identifier as a
	 * decimal number.
	 * @see https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
	 * @return Returns the unique index identifier or FALSE on failure.
	 */
	public function getIfIndex() {
		if ($this->isCached()) {
			$result = array_value($this->ip, "ifindex", FALSE);
			if (FALSE !== $result) {
				return $result;
			}
		}
		return $this->getSysFsPropertyInt($this->getDeviceName(),
			"ifindex");
	}

	/**
	 * Get the system-wide interface unique index identifier the
	 * interface is linked to.
	 * @see https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
	 * @return Returns the unique index identifier the interface is linked
	 *   to or FALSE on failure.
	 */
	public function getIfLink() {
		if ($this->isCached()) {
			$result = array_value($this->ip, "link_index", FALSE);
			if (FALSE !== $result) {
				return $result;
			}
		}
		return $this->getSysFsPropertyInt($this->getDeviceName(),
			"iflink");
	}

	/**
	 * Check if the network interface is physical.
	 * @return Returns TRUE if it is a physical network interface,
	 *   otherwise FALSE.
	 */
	public function isPhysical(): bool {
		// Physical interfaces have the same 'ifindex' and 'iflink' values.
		// See https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
		return $this->getIfIndex() === $this->getIfLink();
	}

	/**
	 * Helper function to get a property from sysfs.
	 * @param string $deviceName The device name, e.g. `eth0` or `ext1`.
	 * @param string $propName The property name, e.g. `address`.
     * @param mixed $default The default value. Defaults to FALSE.
	 * @param bool $trim Trim the property value. Defaults to TRUE.
	 * @return Returns the value as string, otherwise the specified
	 *   default value.
	 */
	protected function getSysFsProperty($deviceName, $propName,
			$default = FALSE, $trim = TRUE) {
		$path = sprintf("/sys/class/net/%s/%s", $deviceName, $propName);
		if (!file_exists($path)) {
			return $default;
		}
		$result = file_get_contents($path);
		if (FALSE === $result) {
			return $default;
		}
		return (TRUE === $trim) ? trim($result) : $result;
	}

	/**
	 * Helper function to get a property from sysfs.
	 * @param string $deviceName The device name, e.g. `eth0` or `ext1`.
	 * @param string $propName The property name, e.g. `speed`.
	 * @return Returns the value as integer, otherwise FALSE.
	 */
	protected function getSysFsPropertyInt($deviceName, $propName) {
		$result = $this->getSysFsProperty($deviceName, $propName);
		return (FALSE === $result) ? FALSE : intval($result);
	}
}
