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

class test_openmediavault_json_schema extends \PHPUnit\Framework\TestCase {
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
						],
						"clients" => [
							"type" => "string",
							"default" => ""
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
				],
				"slaves" => [
					"type" => "string",
					"pattern" => "^(((eth|wlan)\\d+|(en|wl)\\S+),)*((eth|wlan)\\d+|(en|wl)\\S+)$"
				],
				"upsname" => [
					"type" => "string",
					"pattern" => "^[a-z0-9_-]+$"
				],
				"devices" => [
					"type" => "string",
					"pattern" => "^(.+[,;])*.+$"
				],
				"hostname" => [
					"type" => "string",
					"format" => "hostname"
				]
			]
		]);
	}

	public function testGetAssoc() {
		$schema = $this->getSchema();
		$this->assertIsArray($schema->getAssoc());
	}

	public function testGetAssocByPath1() {
		$schema = $this->getSchema();
		$this->assertIsArray($schema->getAssocByPath("price"));
	}

	public function testGetAssocByPath2() {
		$schema = $this->getSchema();
		$this->assertEquals($schema->getAssocByPath("ntp.enable"), [
			"type" => "boolean",
			"default" => FALSE
		]);
	}

	public function testGetAssocByPathFail() {
		$schema = $this->getSchema();
		$this->expectException(\OMV\Json\SchemaPathException::class);
		$schema->getAssocByPath("a.b.c");
	}

	public function testValidateFail() {
		$schema = $this->getSchema();
		$this->expectException(\OMV\Json\SchemaValidationException::class);
		$schema->validate(["price" => 38]);
	}

	public function testValidateMaximumFail() {
		$schema = $this->getSchema();
		$this->expectException(\OMV\Json\SchemaValidationException::class);
		$schema->validate(["name" => "Apple", "price" => 41]);
	}

	public function testValidateMinimumFail() {
		$schema = $this->getSchema();
		$this->expectException(\OMV\Json\SchemaValidationException::class);
		$schema->validate(["name" => "Eggs", "price" => 34.99]);
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testValidatePattern1() {
		$schema = $this->getSchema();
		$schema->validate(["name" => "Eggs", "slaves" => "eth0"]);
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testValidatePattern2() {
		$schema = $this->getSchema();
		$schema->validate([
			"name" => "foo",
			"upsname" => utf8_encode("foo-bar_1234")
		]);
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testValidatePattern3() {
		$schema = $this->getSchema();
		$schema->validate([
			"name" => "bar",
			"devices" => utf8_encode("/dev/vdb")
		]);
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testValidatePattern4() {
		$schema = $this->getSchema();
		$schema->validate([
			"name" => "xyz",
			"hostname" => utf8_encode("omv4box")
		]);
	}

	public function testValidatePatternFail1() {
		$schema = $this->getSchema();
		$this->expectException(\OMV\Json\SchemaValidationException::class);
		$schema->validate([
			"name" => "Eggs",
			"slaves" => utf8_encode("xyz0")
		]);
	}

	public function testValidatePatternFail2() {
		$schema = $this->getSchema();
		$this->expectException(\OMV\Json\SchemaValidationException::class);
		$schema->validate([
			"name" => "foo",
			"upsname" => utf8_encode("ε体λñι語ά_1234")
		]);
	}

	public function testValidatePatternFail3() {
		$schema = $this->getSchema();
		$this->expectException(\OMV\Json\SchemaValidationException::class);
		$schema->validate([
			"name" => "xyz",
			"hostname" => utf8_encode("ε体λñ-ι語ά1234")
		]);
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testValidateItems1() {
		$schema = new \OMV\Json\Schema([
			"type" => "array",
			"items" => [
				"type" => "string",
				"format" => "ipv4"
			]
		]);
		$schema->validate([ "192.168.10.101" ]);
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testValidateItems2() {
		$schema = new \OMV\Json\Schema([
			"type" => "array",
			"items" => [[
				"type" => "string",
				"format" => "ipv4"
			],[
				"type" => "string",
				"format" => "ipv6"
			]]
		]);
		$schema->validate([ "192.168.10.101" ]);
	}

	public function testValidateItemsFail() {
		$schema = new \OMV\Json\Schema([
			"type" => "array",
			"items" => [[
				"type" => "string",
				"format" => "ipv4"
			],[
				"type" => "string",
				"format" => "ipv6"
			]]
		]);
		$this->expectException(\OMV\Json\SchemaValidationException::class);
		$schema->validate([ 10, "192.168.10.101" ]);
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testValidateOneOf1() {
		$schema = new \OMV\Json\Schema([
			"type" => "string",
			"oneOf" => [[
				"type" => "string",
				"format" => "ipv6"
			],[
				"type" => "string",
				"format" => "ipv4"
			]]
		]);
		$schema->validate("192.168.10.101");
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testValidateOneOf2() {
		$schema = new \OMV\Json\Schema([
			"type" => "string",
			"oneOf" => [[
				"type" => "string",
				"format" => "email"
			],[
				"type" => "string",
				"format" => "ipv4"
			]]
		]);
		$schema->validate("test@test.com");
	}

	public function testValidateOneOfFail() {
		$schema = new \OMV\Json\Schema([
			"type" => "string",
			"oneOf" => [[
				"type" => "string",
				"format" => "email"
			],[
				"type" => "string",
				"format" => "ipv4"
			]]
		]);
		$this->expectException(\OMV\Json\SchemaValidationException::class);
		$schema->validate("xyz");
	}
}
