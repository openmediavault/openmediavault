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
namespace OMV\System\Net;

require_once("openmediavault/functions.inc");

/**
 * This class provides a simple interface to handle Linux network interfaces.
 * @ingroup api
 */
class NetworkInterface {
	protected $name = "";
	protected $ip = null;
	protected $regex = [
		"ipv4" => '\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}',
		"ipv4cidr" => '(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})\/(\d{1,2})',
		"ipv6" => '[a-f0-9:.]+',
		"ipv6cidr" => '([a-f0-9:.]+)\/(\d{1,3})',
		"state" => 'UP|DOWN|UNKNOWN'
	];
	private $dataCached = FALSE;

	/**
	 * Constructor
	 * @param name The network interface name, e.g. eth0, ethx, ...
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
		$cmdArgs[] = "addr";
		$cmdArgs[] = "show";
		$cmdArgs[] = "dev";
		$cmdArgs[] = escapeshellarg($this->getDeviceName());
		$cmd = new \OMV\System\Process("ip", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		// Process command output for easier parsing.
		// 2: ens6: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP group default qlen 1000
		//     link/ether 52:54:00:a6:76:53 brd ff:ff:ff:ff:ff:ff
		//     inet 192.168.121.38/24 brd 192.168.121.255 scope global dynamic ens6
		//        valid_lft 3583sec preferred_lft 3583sec
		//     inet6 ::ffff:192.168.121.38/64 scope global
		//        valid_lft forever preferred_lft forever
		//     inet6 fe80::5054:ff:fea6:7653/64 scope link
		//        valid_lft forever preferred_lft forever
		foreach ($output as $outputk => &$outputv)
			$outputv = trim($outputv);
		$this->ip = implode("|", $output);

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
	 * Get the network interface IPv4 address.
	 * @return The network interface IPv4 address, otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function getIP() {
		$this->getData();
		$regex = sprintf("/inet\s+%s\s+.+\s+scope global/i",
			$this->regex['ipv4cidr']);
		if (1 !== preg_match($regex, $this->ip, $matches))
			return FALSE;
		return $matches[1];
	}

	/**
	 * Get the network interface IPv6 address.
	 * @return The network interface IPv6 address, otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function getIP6() {
		$this->getData();
		$regex = sprintf("/inet6\s+%s\s+scope global/i",
			$this->regex['ipv6cidr']);
		if (1 !== preg_match($regex, $this->ip, $matches))
			return FALSE;
		return $matches[1];
	}

	/**
	 * Get the network interface IPv4 prefix length.
	 * @return The network interface prefix, otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function getPrefix() {
		$this->getData();
		$regex = sprintf("/inet\s+%s\s+.+\s+scope global/i",
			$this->regex['ipv4cidr']);
		if (1 !== preg_match($regex, $this->ip, $matches))
			return FALSE;
		return intval($matches[2]);
	}

	/**
	 * Get the network interface IPv6 prefix length.
	 * @return The network interface IPv6 mask/prefix length as integer,
	 *   otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function getPrefix6() {
		$this->getData();
		$regex = sprintf("/inet6\s+%s\s+scope global/i",
			$this->regex['ipv6cidr']);
		if (1 !== preg_match($regex, $this->ip, $matches))
			return FALSE;
		return intval($matches[2]);
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
		// ToDo: Return the real netmask and not the prefix length here.
		return $this->getPrefix6();
	}

	/**
	 * Get the network interface MAC address.
	 * @return The network interface MAC address, otherwise FALSE.
	 */
	public function getMAC() {
		$filename = sprintf("/sys/class/net/%s/address",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return trim(file_get_contents($filename));
	}

	/**
	 * Get the network interface MTU.
	 * @return The network interface MTU, otherwise FALSE.
	 */
	public function getMTU() {
		$filename = sprintf("/sys/class/net/%s/mtu",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return trim(file_get_contents($filename));
	}

	/**
	 * Get the network interface IPv4 default gateway.
	 * @return The interface default gateway, or FALSE on failure.
	 * @throw \OMV\ExecException
	 */
	public function getGateway() {
		$cmdArgs = [];
		$cmdArgs[] = "-4";
		$cmdArgs[] = "route";
		$cmdArgs[] = "show";
		$cmdArgs[] = "dev";
		$cmdArgs[] = escapeshellarg($this->getDeviceName());
		$cmd = new \OMV\System\Process("ip", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$output = implode("\n", $output);
		// Parse command output:
		// 192.168.178.0/24  proto kernel  scope link  src 192.168.178.21
		// default via 192.168.178.1
		$regex = sprintf('/default via\s+(%s)/im', $this->regex['ipv4']);
		if (1 !== preg_match($regex, $output, $matches))
			return FALSE;
		return $matches[1];
	}

	/**
	 * Get the network interface IPv6 default gateway.
	 * @return The interface default gateway, or FALSE on failure.
	 * @throw \OMV\ExecException
	 */
	public function getGateway6() {
		$cmdArgs = [];
		$cmdArgs[] = "-6";
		$cmdArgs[] = "route";
		$cmdArgs[] = "show";
		$cmdArgs[] = "dev";
		$cmdArgs[] = escapeshellarg($this->getDeviceName());
		$cmd = new \OMV\System\Process("ip", $cmdArgs);
		$cmd->setRedirect2to1();
		$cmd->execute($output);
		$output = implode("\n", $output);
		// Parse command output:
		// fe80::/64  proto kernel  metric 256  mtu 1500 advmss 1440 hoplimit 4294967295
		// default via fe80::21e:13ff:fef9:af0  metric 1024  mtu 1500 advmss 1440 hoplimit 4294967295
		$regex = sprintf('/default via\s+(%s)\s+/im', $this->regex['ipv6']);
		if (1 !== preg_match($regex, $output, $matches))
			return FALSE;
		return $matches[1];
	}

	/**
	 * Get the network interface state.
	 * @return The network interface state, e.g. 'UP', 'DOWN' or 'UNKNOWN',
	 *   otherwise FALSE.
	 * @throw \OMV\ExecException
	 */
	public function getState() {
		$this->getData();
		if (1 !== preg_match("/state ({$this->regex['state']})/i",
		  $this->ip, $matches))
			return FALSE;
		return $matches[1];
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
		// Inter-|   Receive                                                |  Transmit
		//  face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
		//     lo:  156976     947    0    0    0     0          0         0   156976     947    0    0    0     0       0          0
		//   eth0:162137708  168681    0    0    0     0          0         0 15969317  103565    0    0    0     0       0          0
		//   eth1:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
		$regex = sprintf('/^\s*%s:(.*)$/im', $this->getDeviceName());
		if (1 !== preg_match($regex, file_get_contents("/proc/net/dev"),
		  $matches))
			return FALSE;
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
	 * @return The network interface operation state, otherwise FALSE.
	 */
	public function getOperState() {
		$filename = sprintf("/sys/class/net/%s/operstate",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return trim(file_get_contents($filename));
	}

	/**
	 * Get the network interface duplex value.
	 * @see https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
	 * @return The network interface duplex value as string, otherwise FALSE.
	 */
	public function getDuplex() {
		$filename = sprintf("/sys/class/net/%s/duplex",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return trim(file_get_contents($filename));
	}

	/**
	 * Get the network interface speed value.
	 * @see https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
	 * @return The network interface speed as number in Mbits/sec,
	 *   otherwise FALSE.
	 */
	public function getSpeed() {
		$filename = sprintf("/sys/class/net/%s/speed",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return intval(file_get_contents($filename));
	}

	/**
	 * Get the network interface physical link state.
	 * @see https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
	 * @return The network interface physical link state, otherwise FALSE.
	 */
	public function getCarrier() {
		$filename = sprintf("/sys/class/net/%s/carrier",
		  $this->getDeviceName());
		if (!file_exists($filename))
			return FALSE;
		return intval(file_get_contents($filename));
	}
}
