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

class test_openmediavault_system_systemctrl extends \PHPUnit\Framework\TestCase {
	public function testIsEnabled() {
		$systemCtl = new \OMV\System\SystemCtl("rsyslog.service");
		$this->assertTrue($systemCtl->isEnabled());
	}

	public function testIsNotEnabled() {
		$systemCtl = new \OMV\System\SystemCtl("ctrl-alt-del.target");
		$this->assertFalse($systemCtl->isEnabled());
	}

	public function testIsActive() {
		$systemCtl = new \OMV\System\SystemCtl("rsyslog.service");
		$this->assertTrue($systemCtl->isActive());
	}

	public function testEnableDisable() {
		$systemCtl = new \OMV\System\SystemCtl("chrony.service");
		$isEnabled = $systemCtl->isEnabled();
		$isActive = $systemCtl->isActive();
		$systemCtl->enable(TRUE);
		$this->assertTrue($systemCtl->isEnabled());
		$this->assertTrue($systemCtl->isActive());
		$systemCtl->disable();
		$this->assertFalse($systemCtl->isEnabled());
		$this->assertTrue($systemCtl->isActive());
		$systemCtl->disable(TRUE);
		$this->assertFalse($systemCtl->isEnabled());
		$this->assertFalse($systemCtl->isActive());
		if ($isEnabled) {
			$systemCtl->enable($isActive);
			$this->assertTrue($systemCtl->isEnabled());
			$this->assertTrue($systemCtl->isActive());
		}
	}

	public function testIsMasked() {
		$systemCtl = new \OMV\System\SystemCtl("hwclock.service");
		$this->assertTrue($systemCtl->isMasked());
	}

	public function testIsNotMasked() {
		$systemCtl = new \OMV\System\SystemCtl("default.target");
		$this->assertFalse($systemCtl->isMasked());
	}

	public function testMaskUnmask() {
		$systemCtl = new \OMV\System\SystemCtl("chrony.service");
		$isMasked = $systemCtl->isMasked();
		$systemCtl->mask();
		$this->assertTrue($systemCtl->isMasked());
		$systemCtl->unmask();
		$this->assertFalse($systemCtl->isMasked());
		if ($isMasked) {
			$systemCtl->mask();
			$this->assertTrue($systemCtl->isMasked());
		}
	}
}
