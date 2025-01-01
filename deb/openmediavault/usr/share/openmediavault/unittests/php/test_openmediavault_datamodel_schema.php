#!/usr/bin/phpunit -c/etc/openmediavault
<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2025 Volker Theile
 *
 * OpenMediaVault is free software => you can redistribute it and/or modify
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

class Schema extends \OMV\DataModel\Schema {
	// Make the method public to access it for testing.
	public function checkFormat($value, $schema, $name) {
		parent::checkFormat($value, $schema, $name);
	}
}

class test_openmediavault_datamodel_schema extends \PHPUnit\Framework\TestCase {
	protected function getSchema() {
		return new Schema([
			"type" => "object",
			"properties" => [
				"fsname" => [
					"type" => "string",
					"oneOf" => [[
						"type" => "string",
						"format" => "fsuuid"
					],[
						"type" => "string",
						"format" => "devicefile"
					],[
						"type" => "string",
						"format" => "dirpath"
					]]
				]
			]
		]);
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testCheckFormatFsuuid1() {
		# EXT2/3/4, JFS, XFS
		$schema = $this->getSchema();
		$schema->checkFormat("113dbaac-e496-11e6-ac68-73bc0f572bae",
			[ "format" => "fsuuid" ], "field1");
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testCheckFormatFsuuid2() {
		# FAT
		$schema = $this->getSchema();
		$schema->checkFormat("7A48-BA97",
			[ "format" => "fsuuid" ], "field1");
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testCheckFormatFsuuid3() {
		# NTFS
		$schema = $this->getSchema();
		$schema->checkFormat("2ED43920D438EC29",
			[ "format" => "fsuuid" ], "field1");
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testCheckFormatDevicefile1() {
		$schema = $this->getSchema();
		$schema->checkFormat("/dev/sda1",
			[ "format" => "devicefile" ], "field1");
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testCheckFormatDevicefile2() {
		$schema = $this->getSchema();
		$schema->checkFormat("/dev/disk/by-id/wwn-0x5020c298d81c1c3a",
			[ "format" => "devicefile" ], "field1");
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testCheckFormatDirpath1() {
		$schema = $this->getSchema();
		$schema->checkFormat("/media/a/b/c/@data",
			[ "format" => "dirpath" ], "field1");
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testCheckFormatDirpath2() {
		$schema = $this->getSchema();
		$schema->checkFormat("Library/App Support/Logs/",
			[ "format" => "dirpath" ], "field1");
	}

	public function testCheckFormatDirpathFail() {
		$schema = $this->getSchema();
		$this->expectException(\OMV\Json\SchemaValidationException::class);
		$schema->checkFormat("/media/a/../../b/c",
			[ "format" => "dirpath" ], "field1");
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testCheckFormatNetbios1() {
		$schema = $this->getSchema();
		$schema->checkFormat("WORKGROUP",
			[ "format" => "netbiosname" ], "field1");
	}

	/**
	 * @doesNotPerformAssertions
	 */
	public function testCheckFormatNetbios2() {
		$schema = $this->getSchema();
		$schema->checkFormat("F!OO-%&B^AR_",
			[ "format" => "netbiosname" ], "field1");
	}

	public function testCheckFormatNetbios3() {
		$schema = $this->getSchema();
		$this->expectException(\OMV\Json\SchemaValidationException::class);
		$schema->checkFormat("REWARDEDIVANSNAS",
			[ "format" => "netbiosname" ], "field1");
	}

	public function testCheckFormatNetbios4() {
		$schema = $this->getSchema();
		$this->expectException(\OMV\Json\SchemaValidationException::class);
		$schema->checkFormat("FOO]BAR",
			[ "format" => "netbiosname" ], "field1");
	}
}
