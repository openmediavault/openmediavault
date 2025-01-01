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

class test_openmediavault_util_keyvaluefile extends \PHPUnit\Framework\TestCase {
	public function test_space_keyvalue_delimiter() {
		$file = new \OMV\Util\KeyValueFile(sprintf(
			"%s/../data/login.defs", getcwd()));
		$file->setKeyValueDelimiter("\\s");
		$file->setKeyCaseSensitiv(TRUE);
		$dict = $file->getAssoc();
		// Check if the returned value is an associative array.
		$this->assertIsArray($dict);
		// Check if some keys exist.
		$this->assertArrayHasKey("UID_MIN", $dict);
		$this->assertEquals($dict["UID_MIN"], "1000");
		$this->assertArrayHasKey("GID_MAX", $dict);
		$this->assertEquals($dict["GID_MAX"], "60000");
		$this->assertArrayHasKey("ENCRYPT_METHOD", $dict);
		$this->assertEquals($dict["ENCRYPT_METHOD"], "SHA512");
		$this->assertArrayNotHasKey("FAKE_SHELL", $dict);
	}
}
