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

class test_openmediavault_config_databasebackend extends \PHPUnit\Framework\TestCase {
	private $databaseBackend;

	protected function setUp(): void {
		$this->databaseBackend = new \OMV\Config\DatabaseBackend(
			sprintf("%s/../data/config.xml", getcwd()),
			FALSE);
		$this->databaseBackend->load();
	}

	public function testCompare1() {
		$this->assertEquals($this->databaseBackend->compare(
			"//system/time",
			[
				"timezone" => "Europe/Berlin",
				"ntp" => [
					"enable" => 0,
					"timeservers" => "pool.ntp.org",
					"clients" => ""
				]
			]
		), 0);
	}

	public function testCompare2() {
		$this->assertEquals($this->databaseBackend->compare(
			"//system/powermanagement",
			[
				"cpufreq" => 0,
				"powerbtn" => "nothing"
			]
		), -1);
	}
}
