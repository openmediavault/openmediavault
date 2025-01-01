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

class test_openmediavault_config_configobject extends \PHPUnit\Framework\TestCase {
	protected function setUp(): void {
		// Tell the database implementation to use the test database.
		\OMV\Environment::set("OMV_CONFIG_FILE", sprintf(
			"%s/../data/config.xml", getcwd()));
		// Setup the model manager.
		$modelMngr = \OMV\DataModel\Manager::getInstance();
		$modelMngr->load();
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testConstructor() {
		$object = new \OMV\Config\ConfigObject("conf.service.ftp.share");
	}

	public function testGetDefaults() {
		$object = new \OMV\Config\ConfigObject("conf.service.ftp.share");
		$defaults = $object->getDefaultsAssoc();
		$this->assertEquals($defaults, [
			'comment' => '',
			'enable' => FALSE,
			'uuid' => 'fa4b1c66-ef79-11e5-87a0-0002b3a176b4',
			'sharedfolderref' => '',
			'extraoptions' => ''
		]);
	}

	public function testGetSet1() {
		$object = new \OMV\Config\ConfigObject("conf.service.ftp.share");
		$object->set("comment", "test");
		$this->assertEquals($object->get("comment"), "test");
	}

	public function testGetSet2() {
		$object = new \OMV\Config\ConfigObject("conf.system.time");
		$this->assertEquals($object->get("timezone"), "Etc/UTC");
		$this->assertFalse($object->get("ntp.enable"));
		$this->assertEquals($object->get("ntp.timeservers"),
			"pool.ntp.org,pool1.ntp.org;pool2.ntp.org");
		$this->assertEquals($object->get("ntp.clients"), "");
	}

	public function testGetSet3() {
		$object = new \OMV\Config\ConfigObject("conf.system.sharedfolder");
		$privileges = $object->get("privileges.privilege");
		$this->assertIsArray($privileges);
		$this->assertEquals($privileges, []);
	}

	public function testSetAssoc() {
		$object = new \OMV\Config\ConfigObject("conf.service.ftp.share");
		$object->setAssoc([
			"comment" => "test",
			"enable" => TRUE
		]);
		$this->assertTrue($object->get("enable"));
	}

	public function testIsEmpty() {
		$object = new \OMV\Config\ConfigObject("conf.service.ftp.share");
		$this->assertTrue($object->isEmpty("comment"));
	}

	public function testNotIsEmpty() {
		$object = new \OMV\Config\ConfigObject("conf.service.ftp.share");
		$object->set("comment", "test");
		$this->assertFalse($object->isEmpty("comment"));
	}
}
