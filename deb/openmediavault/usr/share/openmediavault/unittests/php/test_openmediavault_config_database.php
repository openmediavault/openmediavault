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
	public function setUp() {
		// Tell the database implementation to use the test database.
		\OMV\Environment::set("OMV_CONFIG_FILE", sprintf(
			"%s/../data/config.xml", getcwd()));
		// Setup the model manager.
		$modelMngr = \OMV\DataModel\Manager::getInstance();
		$modelMngr->load();
	}

	public function testConstructor() {
		$db = \OMV\Config\Database::getInstance();
		$this->assertInstanceOf("OMV\Config\Database", $db);
	}

	public function testGet1() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.time");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertEquals($object->get("ntp.timeservers"), "pool.ntp.org");
	}

	public function testGet2() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.apt.distribution");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertFalse($object->get("proposed"));
	}

	public function testGetIterable() {
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->get("conf.system.notification.notification");
		$this->assertInternalType("array", $objects);
		$this->assertTrue(0 < count($objects));
		$this->assertInstanceOf("OMV\Config\ConfigObject", $objects[0]);
	}

	public function testGetByFilter1() {
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.system.notification.notification", [
			'operator' => 'stringEquals',
			'arg0' => 'uuid',
			'arg1' => '03dc067d-1310-45b5-899f-b471a0ae9233'
		]);
		$this->assertInternalType("array", $objects);
		$this->assertEquals(count($objects), 1);
		$this->assertInstanceOf("OMV\Config\ConfigObject", $objects[0]);
	}

	public function testGetByFilter2() {
		$db = \OMV\Config\Database::getInstance();
		$objects = $db->getByFilter("conf.system.notification.notification", [
			'operator' => 'stringContains',
			'arg0' => 'id',
			'arg1' => 'monit'
		]);
		$this->assertInternalType("array", $objects);
		$this->assertEquals(count($objects), 5);
	}

	public function testExists() {
		$db = \OMV\Config\Database::getInstance();
		$this->assertTrue($db->exists(
			"conf.system.notification.notification", [
				'operator' => 'stringEquals',
				'arg0' => 'id',
				'arg1' => 'smartmontools'
			]));
	}

	public function testIsUnique() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.notification.notification",
			"c1cd54af-660d-4311-8e21-2a19420355bb");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertTrue($db->isUnique($object, "uuid"));
	}

	public function testIsReferenced1() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.sharedfolder",
			"339bd101-5744-4017-9392-01a156f15ab9");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertTrue($db->isReferenced($object));
	}

	public function testIsReferenced2() {
		$db = \OMV\Config\Database::getInstance();
		$object = $db->get("conf.system.sharedfolder",
			"91fe93fc-ef9d-11e6-9b06-000c2900c2de");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertFalse($db->isReferenced($object));
	}
}
