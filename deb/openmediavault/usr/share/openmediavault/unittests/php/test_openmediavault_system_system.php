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

class SystemTest extends \OMV\System\System {
	public static function getRootDeviceFile() {
		return "/dev/sda1";
	}
}

class test_openmediavault_system_system extends \PHPUnit\Framework\TestCase {
	public function testIsRootDeviceFile1(): void {
		$this->assertTrue(SystemTest::isRootDeviceFile("/dev/root"));
	}

	public function testIsRootDeviceFile2(): void {
		$this->assertTrue(SystemTest::isRootDeviceFile("/dev/sda1"));
	}

	public function testIsRootDeviceFile3(): void {
		$this->assertTrue(SystemTest::isRootDeviceFile("/dev/sda", FALSE));
	}

	public function testIsRootDeviceFile4(): void {
		$this->assertFalse(SystemTest::isRootDeviceFile("/dev/sda"));
	}

	public function testIsRootDeviceFile5(): void {
		$this->assertFalse(SystemTest::isRootDeviceFile("/dev/sdb", FALSE));
	}

	public function testIsRootDeviceFile6(): void {
		$this->assertFalse(SystemTest::isRootDeviceFile("/dev/sdb2"));
	}
}
