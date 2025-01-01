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

class test_openmediavault_system_net_networkinterfacebackend extends \PHPUnit\Framework\TestCase {
	public function test_isTypeOf_ethernet() {
		$backend = new \OMV\System\Net\NetworkInterfaceBackend\Ethernet;
		$this->assertTrue($backend->isTypeOf("enp2s0"));
		$this->assertFalse($backend->isTypeOf("br1"));
	}

	public function test_isTypeOf_wifi() {
		$backend = new \OMV\System\Net\NetworkInterfaceBackend\Wifi;
		$this->assertTrue($backend->isTypeOf("wlan0"));
		$this->assertTrue($backend->isTypeOf("wlp3s0"));
		$this->assertFalse($backend->isTypeOf("venet2"));
	}

	public function test_isTypeOf_bond() {
		$backend = new \OMV\System\Net\NetworkInterfaceBackend\Bond;
		$this->assertTrue($backend->isTypeOf("bond0"));
		$this->assertFalse($backend->isTypeOf("wlan1"));
	}

	public function test_isTypeOf_bridge() {
		$backend = new \OMV\System\Net\NetworkInterfaceBackend\Bridge;
		$this->assertTrue($backend->isTypeOf("br2"));
		$this->assertFalse($backend->isTypeOf("thunderbolt0"));
	}

	public function test_isTypeOf_vlan() {
		$backend = new \OMV\System\Net\NetworkInterfaceBackend\Vlan;
		$this->assertTrue($backend->isTypeOf("eth0.1"));
		$this->assertFalse($backend->isTypeOf("ens1"));
	}
}
