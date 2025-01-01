#!/usr/bin/phpunit -c/etc/openmediavault
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
require_once("openmediavault/autoloader.inc");
require_once("openmediavault/globals.inc");

class NetworkInterfaceMock extends \OMV\System\Net\NetworkInterface {
	public function __construct() {
		parent::__construct("ens6");
		$this->ip = [
			"ifindex" => 2,
			"ifname" => "ens6",
			"flags" => ["BROADCAST", "MULTICAST", "UP", "LOWER_UP"],
			"mtu" => 1500,
			"qdisc" => "fq_codel",
			"operstate" => "UP",
			"group" => "default",
			"txqlen" => 1000,
			"link_type" => "ether",
			"address" => "52:54:00:a6:76:53",
			"broadcast" => "ff:ff:ff:ff:ff:ff",
			"altnames" => ["enp0s6"],
			"addr_info" => [
				[
					"family" => "inet",
					"local" => "192.168.121.38",
					"prefixlen" => 24,
					"broadcast" => "192.168.121.255",
					"scope" => "global",
					"dynamic" => true,
					"label" => "ens6",
					"valid_life_time" => 2325,
					"preferred_life_time" => 2325
				],
				[
					"family" => "inet6",
					"local" => "::ffff:192.168.121.38",
					"prefixlen" => 64,
					"scope" => "global",
					"valid_life_time" => 4294967295,
					"preferred_life_time" => 4294967295
				],
				[
					"family" => "inet6",
					"local" => "fe80::5054:ff:fea6:7653",
					"prefixlen" => 64,
					"scope" => "link",
					"valid_life_time" => 4294967295,
					"preferred_life_time" => 4294967295
				]
			]
		];
		$this->setCached(TRUE);
	}
}

class test_openmediavault_system_net extends \PHPUnit\Framework\TestCase {
	protected function getNetworkInterfaceMock() {
		return new NetworkInterfaceMock();
	}

	public function test_getDeviceName_mocked() {
		$mock = $this->getNetworkInterfaceMock();
		$this->assertEquals($mock->getDeviceName(), "ens6");
	}

	public function test_exists_mocked() {
		$mock = $this->getNetworkInterfaceMock();
		$this->assertTrue($mock->exists());
	}

	public function test_getIP_mocked() {
		$mock = $this->getNetworkInterfaceMock();
		$this->assertEquals($mock->getIP(), "192.168.121.38");
	}

	public function test_getIP6_mocked() {
		$mock = $this->getNetworkInterfaceMock();
		$this->assertEquals($mock->getIP6(), "::ffff:192.168.121.38");
	}

	public function test_getPrefix_mocked() {
		$mock = $this->getNetworkInterfaceMock();
		$this->assertEquals($mock->getPrefix(), 24);
	}

	public function test_getPrefix6_mocked() {
		$mock = $this->getNetworkInterfaceMock();
		$this->assertEquals($mock->getPrefix6(), 64);
	}

	public function test_getNetmask_mocked() {
		$mock = $this->getNetworkInterfaceMock();
		$this->assertEquals($mock->getNetmask(), "255.255.255.0");
	}

	public function test_getNetmask6_mocked() {
		$mock = $this->getNetworkInterfaceMock();
		$this->assertEquals($mock->getNetmask6(), 64);
	}

	public function test_getState_mocked() {
		$mock = $this->getNetworkInterfaceMock();
		$this->assertEquals($mock->getState(), "UP");
	}

	public function test_getType_mocked() {
		$mock = $this->getNetworkInterfaceMock();
		$this->assertEquals($mock->getType(), "unknown");
	}

	public function test_real_interface() {
		$backend = new \OMV\System\Net\NetworkInterfaceBackend\Ethernet();
		$mngr = \OMV\System\Net\NetworkInterfaceBackend\Manager::getInstance();
		$mngr->registerBackend($backend);
		// Get existing devices.
		$devs = $mngr->enumerate(OMV_NETWORK_INTERFACE_TYPE_ETHERNET);
		$this->assertIsArray($devs);
		$this->assertGreaterThan(1, count($devs));
		// Create an interface device object.
		$netIf = $mngr->getImpl($devs[0]);
		$this->assertInstanceOf("OMV\System\Net\NetworkInterface", $netIf);
		// Test various methods.
		// Assume the following:
		// - the interface is UP
		// - check only IPv4 method
		$this->assertTrue($netIf->exists());
		$this->assertIsString($netIf->getDeviceName());
		$this->assertIsString($netIf->getIP());
		$this->assertIsInt($netIf->getPrefix());
		$this->assertIsString($netIf->getNetmask());
		$this->assertIsString($netIf->getMAC());
		$this->assertIsString($netIf->getMTU());
		$this->assertIsString($netIf->getGateway());
		$this->assertEquals($netIf->getState(), "UP");
	}
}
