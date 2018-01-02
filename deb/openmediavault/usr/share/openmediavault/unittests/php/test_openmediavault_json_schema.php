#!/usr/bin/phpunit -c/etc/openmediavault
<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2018 Volker Theile
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

class test_openmediavault_json_schema extends \PHPUnit_Framework_TestCase {
	protected function getSchema() {
		return new \OMV\Json\Schema([
			"type" => "object",
			"properties" => [
				"name" => [
					"type" => "string",
					"required" => TRUE
				],
				"price" => [
					"type" => "number",
					"minimum" => 35,
					"maximum" => 40
				],
				"ntp" => [
					"type" => "object",
					"properties" => [
						"enable" => [
							"type" => "boolean",
							"default" => FALSE
						],
						"timeservers" => [
							"type" => "string",
							"default" => "pool.ntp.org"
						]
					]
				],
				"privilege" => [
					"type" => "array",
					"items" => [
						"type" => "object",
						"properties" => [
							"type" => [
								"type" => "string",
								"enum" => [ "user", "group" ]
							],
							"name" => [
								"type" => "string"
							],
							"perms" => [
								"type" => "integer",
								"enum" => [ 0, 5, 7 ]
							]
						]
					]
				]
			]
		]);
	}

	public function testGetAssoc() {
		$schema = $this->getSchema();
		$this->assertInternalType("array", $schema->getAssoc());
	}

	public function testGetAssocByPath1() {
		$schema = $this->getSchema();
		$this->assertInternalType("array", $schema->getAssocByPath("price"));
	}

	public function testGetAssocByPath2() {
		$schema = $this->getSchema();
		$this->assertEquals($schema->getAssocByPath("ntp.enable"), [
			"type" => "boolean",
			"default" => FALSE
		]);
	}

	/**
	 * @expectedException OMV\Json\SchemaPathException
	 */
	public function testGetAssocByPathFail() {
		$schema = $this->getSchema();
		$schema->getAssocByPath("a.b.c");
	}

	/**
	 * @expectedException OMV\Json\SchemaValidationException
	 */
	public function testValidate() {
		$schema = $this->getSchema();
		$schema->validate(["price" => 38]);
	}

	/**
	 * @expectedException OMV\Json\SchemaValidationException
	 */
	public function testValidateMaximum() {
		$schema = $this->getSchema();
		$schema->validate(["name" => "Apple", "price" => 41]);
	}

	/**
	 * @expectedException OMV\Json\SchemaValidationException
	 */
	public function testValidateMinimum() {
		$schema = $this->getSchema();
		$schema->validate(["name" => "Eggs", "price" => 34.99]);
	}
}
