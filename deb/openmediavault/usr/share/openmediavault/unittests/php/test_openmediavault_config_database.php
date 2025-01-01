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

class test_openmediavault_config_database extends \PHPUnit\Framework\TestCase {
	protected function setUp(): void {
		// Tell the database implementation to use the test database.
		\OMV\Environment::set("OMV_CONFIG_FILE", sprintf(
			"%s/../data/config.xml", getcwd()));
		// Setup the model manager.
		$modelMngr = \OMV\DataModel\Manager::getInstance();
		$modelMngr->load();
	}

	/**
	 * Create a copy of the database and use this. This is useful if the
	 * database is modified during the test.
	 */
	private function useTmpConfigDatabase() {
		$configFile = sprintf("%s/../data/config.xml", getcwd());
		$tmpConfigFile = tempnam("/tmp", "cfg");
		copy($configFile, $tmpConfigFile);
		\OMV\Environment::set("OMV_CONFIG_FILE", $tmpConfigFile);
	}

	public function testConstructor() {
		$db = new \OMV\Config\Database();
		$this->assertInstanceOf("OMV\Config\Database", $db);
	}

	public function testGet1() {
		$db = new \OMV\Config\Database();
		$object = $db->get("conf.system.time");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertEquals($object->get("ntp.timeservers"), "pool.ntp.org");
		$this->assertEquals($object->get("ntp.clients"), "");
	}

	public function testGet2() {
		$db = new \OMV\Config\Database();
		$object = $db->get("conf.system.apt.distribution");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertFalse($object->get("proposed"));
	}

	public function testGetFail() {
		$db = new \OMV\Config\Database();
		$this->expectException(\OMV\Config\DatabaseException::class);
		$db->get("conf.system.notification.notification",
			"c1cd54af-0000-1111-2222-2a19420355bb");
	}

	public function testGetIterable() {
		$db = new \OMV\Config\Database();
		$objects = $db->get("conf.system.notification.notification");
		$this->assertIsArray($objects);
		$this->assertTrue(0 < count($objects));
		$this->assertInstanceOf("OMV\Config\ConfigObject", $objects[0]);
	}

	public function testGetByFilter1() {
		$db = new \OMV\Config\Database();
		$objects = $db->getByFilter("conf.system.notification.notification", [
			'operator' => 'stringEquals',
			'arg0' => 'uuid',
			'arg1' => '03dc067d-1310-45b5-899f-b471a0ae9233'
		]);
		$this->assertIsArray($objects);
		$this->assertEquals(count($objects), 1);
		$this->assertInstanceOf("OMV\Config\ConfigObject", $objects[0]);
	}

	public function testGetByFilter2() {
		$db = new \OMV\Config\Database();
		$objects = $db->getByFilter("conf.system.notification.notification", [
			'operator' => 'stringContains',
			'arg0' => 'id',
			'arg1' => 'monit'
		]);
		$this->assertIsArray($objects);
		$this->assertEquals(count($objects), 5);
	}

	public function testGetByFilter3() {
		$db = new \OMV\Config\Database();
		$objects = $db->getByFilter("conf.service.smartmontools.device", [
			'operator' => 'distinct',
			'arg0' => 'enable'
		]);
		$this->assertIsArray($objects);
		$this->assertEquals(count($objects), 2);
	}

	public function testExists() {
		$db = new \OMV\Config\Database();
		$this->assertTrue($db->exists(
			"conf.system.notification.notification", [
				'operator' => 'stringEquals',
				'arg0' => 'id',
				'arg1' => 'smartmontools'
			]));
	}

	public function testIsUnique() {
		$db = new \OMV\Config\Database();
		$object = $db->get("conf.system.notification.notification",
			"c1cd54af-660d-4311-8e21-2a19420355bb");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertTrue($db->isUnique($object, "uuid"));
	}

	public function testIsReferenced1() {
		$db = new \OMV\Config\Database();
		$object = $db->get("conf.system.sharedfolder",
			"339bd101-5744-4017-9392-01a156f15ab9");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertTrue($db->isReferenced($object));
	}

	public function testIsReferenced2() {
		$db = new \OMV\Config\Database();
		$object = $db->get("conf.system.sharedfolder",
			"91fe93fc-ef9d-11e6-9b06-000c2900c2de");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertFalse($db->isReferenced($object));
	}

	public function testDelete() {
		$this->useTmpConfigDatabase();
		$db = new \OMV\Config\Database();
		$db->getBackend()->setVersioning(FALSE);
		$object = $db->get("conf.system.notification.notification",
			"03dc067d-1310-45b5-899f-b471a0ae9233");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$db->delete($object, TRUE);
		# Ensure that the object does not exist anymore.
		$this->assertFalse($db->exists(
			$object->getModel()->getId(), [
				'operator' => 'stringEquals',
				'arg0' => $object->getModel()->getIdProperty(),
				'arg1' => $object->get($object->getModel()->getIdProperty())
			]));
	}

	public function testDeleteByFilter() {
		$this->useTmpConfigDatabase();
		$db = new \OMV\Config\Database();
		$db->getBackend()->setVersioning(FALSE);
		$db->deleteByFilter(
			"conf.system.notification.notification", [
				'operator' => 'stringContains',
				'arg0' => 'id',
				'arg1' => 'monit'
			]);
		// Check the number of remaining configuration objects.
		$objects = $db->get("conf.system.notification.notification");
		$this->assertIsArray($objects);
		$this->assertEquals(count($objects), 3);
	}

	public function testUpdate() {
		$this->useTmpConfigDatabase();
		$db = new \OMV\Config\Database();
		$db->getBackend()->setVersioning(FALSE);
		$object = $db->get("conf.system.apt.distribution");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertFalse($object->get("proposed"));
		$object->set("proposed", TRUE);
		$db->set($object);
		$object = $db->get("conf.system.apt.distribution");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertTrue($object->get("proposed"));
	}

	public function testSetNewIterable() {
		$this->useTmpConfigDatabase();
		$db = new \OMV\Config\Database();
		$db->getBackend()->setVersioning(FALSE);
		// Check the current number of configuration objects.
		$objects = $db->get("conf.system.notification.notification");
		$this->assertIsArray($objects);
		$this->assertEquals(count($objects), 8);
		// Create a new configuration object.
		$newObject = new \OMV\Config\ConfigObject(
			"conf.system.notification.notification");
		$newObject->setAssoc([
			'uuid' => \OMV\Environment::get('OMV_CONFIGOBJECT_NEW_UUID'),
			'id' => 'test',
			'enable' => FALSE
		]);
		$db->set($newObject);
		// Validate the configuration object.
		$this->assertNotEquals($newObject->get("uuid"),
			\OMV\Environment::get('OMV_CONFIGOBJECT_NEW_UUID'));
		// Check whether the new configuration object was stored.
		$objects = $db->get("conf.system.notification.notification");
		$this->assertIsArray($objects);
		$this->assertEquals(count($objects), 9);
		// Get the configuration object to validate its properties.
		$object = $db->get("conf.system.notification.notification",
			$newObject->get("uuid"));
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertEquals($object->get("id"), "test");
		$this->assertFalse($object->get("enable"));
	}

	public function testSetFail() {
		$this->useTmpConfigDatabase();
		$db = new \OMV\Config\Database();
		$db->getBackend()->setVersioning(FALSE);
		// Try to put an object that does not exist.
		$newObject = new \OMV\Config\ConfigObject(
			"conf.system.notification.notification");
		$newObject->setAssoc([
			'uuid' => '2f6bffd8-f5c2-11e6-9584-17a40dfa0331',
			'id' => 'xyz',
			'enable' => TRUE
		]);
		$this->expectException(\OMV\Config\DatabaseException::class);
		$db->set($newObject);
	}

	public function testUpdateIterable() {
		$this->useTmpConfigDatabase();
		$db = new \OMV\Config\Database();
		$db->getBackend()->setVersioning(FALSE);
		// Get the configuration object.
		$object = $db->get("conf.system.notification.notification",
			"c1cd54af-660d-4311-8e21-2a19420355bb");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertTrue($object->get("enable"));
		// Modify a property and put the configuration object.
		$object->set("enable", FALSE);
		$db->set($object);
		// Get the configuration object to validate its properties.
		$object = $db->get("conf.system.notification.notification",
			"c1cd54af-660d-4311-8e21-2a19420355bb");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertFalse($object->get("enable"));
	}

	public function testGetIdentifiable() {
		// Get the configuration object.
		$db = new \OMV\Config\Database();
		$object = $db->get("conf.system.sharedfolder",
			"339bd101-5744-4017-9392-01a156f15ab9");
		$this->assertInstanceOf("OMV\Config\ConfigObject", $object);
		$this->assertIsArray($object->get("privileges.privilege"));
		$this->assertEquals($object->get("privileges.privilege.0.perms"), 7);
		$this->assertEquals($object->get("privileges.privilege.1.name"),
			"test2");
	}
}
