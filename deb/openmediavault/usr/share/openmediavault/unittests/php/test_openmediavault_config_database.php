#!/usr/bin/phpunit -c/etc/openmediavault
<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2017 Volker Theile
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

class test_openmediavault_config_database extends \PHPUnit_Framework_TestCase {
	public function testConstructor() {
		$db = \OMV\Config\Database::getInstance();
		$this->assertInstanceOf("OMV\Config\Database", $db);
	}

	public function testGet() {
		$modelMngr = \OMV\DataModel\Manager::getInstance();
		$modelMngr->load();
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.time");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertEquals($object->get("ntp.timeservers"), "pool.ntp.org");
	}
}
