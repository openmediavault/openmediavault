#!/usr/bin/phpunit -c/etc/openmediavault
<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2020 Volker Theile
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
require_once("openmediavault/functions.inc");

class test_openmediavault_functions extends \PHPUnit\Framework\TestCase {
	private function getDict() {
		return [
			'x' => 3,
			'a' => [
				'b' => [
					'c' => 100
				]
			],
			'k' => ['u', 'i', 'o'],
			'y' => [
				'z' => [[
					'aa' => '1',
					'bb' => '2',
					'cc' => '3'
				],[
					'aa' => '11',
					'bb' => '22',
					'cc' => '33'
				]]
			]
		];
	}

	private function getFlatDict() {
		return [
			'x' => 3,
			'a.b.c' => 100,
			'k.0' => 'u',
			'k.1' => 'i',
			'k.2' => 'o',
			'y.z.0.aa' => 1,
			'y.z.0.bb' => 2,
			'y.z.0.cc' => 3,
			'y.z.1.aa' => 11,
			'y.z.1.bb' => 22,
			'y.z.1.cc' => 33
		];
	}

	public function test_array_flatten() {
		$d_flat = array_flatten($this->getDict());
		$this->assertEquals($d_flat, $this->getFlatDict());
	}

	public function test_array_expand() {
		$d = array_expand($this->getFlatDict());
		$this->assertEquals($d, $this->getDict());
	}

	public function test_array_sort_key() {
		$d = [
			['id' => 3, 'text' => 'c'],
			['id' => 1, 'text' => 'a'],
			['id' => 2, 'text' => 'b']
		];
		$this->assertTrue(array_sort_key($d, "id"));
		$this->assertEquals($d, [
			['id' => 1, 'text' => 'a'],
			['id' => 2, 'text' => 'b'],
			['id' => 3, 'text' => 'c']
		]);
	}

	public function test_array_sort_key_fail() {
		$d = [1, 2, 3];
		$this->assertFalse(array_sort_key($d, "id"));
	}

	public function test_array_value() {
		$d = ["a" => 1, "c" => "3"];
		$this->assertEquals(array_value($d, "a", 0), 1);
		$this->assertEquals(array_value($d, "b", "2"), "2");
		$this->assertEquals(array_value($d, "c"), "3");
		$this->assertNull(array_value($d, "d"));
	}

	public function test_array_keys_exists() {
		$d = ["a" => 1, "b" => 2, "c" => "3"];
		$this->assertTrue(array_keys_exists(["a", "c"], $d));
		$this->assertFalse(array_keys_exists(["a", "d"], $d, $missing));
		$this->assertEquals(["d"], $missing);
	}

	public function test_array_filter_ex() {
		$d = [
			['id' => 3, 'text' => 'c'],
			['id' => 1, 'text' => 'a'],
			['id' => 2, 'text' => 'b'],
			['id' => 4, 'text' => 'a']
		];
		$d_filtered = array_filter_ex($d, "text", "a");
		$this->assertEquals($d_filtered, [
			['id' => 1, 'text' => 'a'],
			['id' => 4, 'text' => 'a']
		]);
		$d_filtered = array_filter_ex($d, "id", 5);
		$this->assertEmpty($d_filtered);
		$d = ["a" => 1, "c" => "3"];
		$d_filtered = array_filter_ex($d, "prio", 10);
		$this->assertEmpty($d_filtered);
	}

	public function test_array_filter_ex_fail() {
		$d = array_filter_ex([1, 2, 3], "text", "b");
		$this->assertNull($d);
	}

	public function test_boolvalEx() {
		$this->assertTrue(boolvalEx(TRUE));
		$this->assertTrue(boolvalEx("1"));
		$this->assertTrue(boolvalEx("on"));
		$this->assertTrue(boolvalEx("yes"));
		$this->assertTrue(boolvalEx("y"));
		$this->assertTrue(boolvalEx("true"));
		$this->assertFalse(boolvalEx("ja"));
		$this->assertFalse(boolvalEx("nein"));
		$this->assertFalse(boolvalEx("0"));
		$this->assertFalse(boolvalEx("no"));
		$this->assertFalse(boolvalEx("false"));
		$this->assertFalse(boolvalEx(NULL));
		$this->assertFalse(boolvalEx(FALSE));
	}

	public function test_is_uuid() {
		$this->assertTrue(is_uuid("7d018540-f821-11e6-9e7a-334c295df4fc"));
		$this->assertFalse(is_uuid("7d018540-f821-11e6-9e7a=334c295df4fc"));
		$this->assertFalse(is_uuid("GTZ18540-f821-11e6-9e7a-334c295df4fc"));
		$this->assertFalse(is_uuid("11e6-9e7a-334c295df4fc"));
		$this->assertFalse(is_uuid("11e6"));
	}

	public function test_uuid_equals() {
		$this->assertTrue(uuid_equals("7d018540-f821-11e6-9e7a-334c295df4fc",
			"7d018540-f821-11e6-9e7a-334c295df4fc"));
		$this->assertFalse(uuid_equals("7d018540-f821-11e6-9e7a-334c295df4fc",
			"a771f738-f821-11e6-8097-6fc6ea7a7bb3"));
		$this->assertFalse(uuid_equals("7d018540-f821-11e6-9e7a-334c295df4fc",
			"a771f738"));
	}

	public function test_is_json_1() {
		$this->assertTrue(is_json('{"z": "test", "x": 0, "a.b.c": 123}'));
	}

	public function test_is_json_2() {
		$this->assertTrue(is_json('{"z": [1, 2, 3], "x": 3, "a.b.c": 100}'));
	}

	public function test_is_json_fail_1() {
		$this->assertFalse(is_json("'z': [1, 2, 3]"));
	}

	public function test_is_json_fail_2() {
		$this->assertFalse(is_json("xyz"));
	}

	public function test_is_json_fail_3() {
		$this->assertFalse(is_json(123));
	}

	public function test_build_path_1() {
		$path = build_path(DIRECTORY_SEPARATOR, "/usr", "local", "share");
		$this->assertEquals($path, "/usr/local/share");
	}

	public function test_build_path_2() {
		$path = build_path(DIRECTORY_SEPARATOR, "usr", "//local", "/share//");
		$this->assertEquals($path, "usr/local/share/");
	}

	public function test_build_path_3() {
		$path = build_path(DIRECTORY_SEPARATOR, "/", "");
		$this->assertEquals($path, "/");
	}

	public function test_build_path_4() {
		$path = build_path(DIRECTORY_SEPARATOR, "", "/");
		$this->assertEquals($path, "/");
	}

	public function test_build_path_5() {
		$path = build_path(DIRECTORY_SEPARATOR, "", "/", "/", "/usr", "");
		$this->assertEquals($path, "/usr");
	}

	public function test_build_path_6() {
		$path = build_path(DIRECTORY_SEPARATOR, ".", "/../", "../");
		$this->assertEquals($path, "./../../");
	}

	public function test_json_encode_safe() {
		$data = [
			"z" => [1, 2, 3],
			"x" => 3,
			"a.b.c" => 10.4,
			"k" => "äöü$%#".chr(0x09).chr(0x0A).chr(0x0C).chr(0x0D)
		];
		$json = json_encode_safe($data);
		$this->assertNotNull($json);
		$this->assertInternalType("string", $json);
	}

	public function test_json_decode_safe() {
		$json = '{"z": [1,2,3], "x": 3, "a.b.c": 10.4, "k": "\u00e4\u00f6\u00fc$%#\t\n\f\r"}';
		$data = json_decode_safe($json, TRUE);
		$this->assertInternalType("array", $data);
		$k = "äöü$%#".chr(0x09).chr(0x0A).chr(0x0C).chr(0x0D);
		$this->assertEquals($data['k'], $k);
	}

	public function test_array_unique_key() {
		$d = [
			['id' => 3, 'text' => 'c'],
			['id' => 1, 'text' => 'a'],
			['id' => 2, 'text' => 'c'],
			['id' => 4, 'text' => 'b'],
			['id' => 5, 'text' => 'b']
		];
		$d = array_unique_key($d, "text");
		$this->assertInternalType("array", $d);
		$this->assertEquals($d, [
			['id' => 3, 'text' => 'c'],
			['id' => 1, 'text' => 'a'],
			['id' => 4, 'text' => 'b']
		]);
	}

	public function test_array_unique_key_fail() {
		$d = array_unique_key([1, 2, 3], "text");
		$this->assertNull($d);
	}

	public function test_escape_path() {
		$this->assertEquals(escape_path("/srv/dev-disk-by-label-xx yy"),
			"/srv/dev-disk-by-label-xx\\x20yy");
	}

	public function test_unescape_path() {
		$this->assertEquals(unescape_path("/srv/dev-disk-by-label-xx\\x20yy"),
			"/srv/dev-disk-by-label-xx yy");
	}

	public function test_array_remove_key_exists() {
		$d = ["a" => "xxx"];
		$this->assertTrue(array_remove_key($d, "a"));
		$this->assertEquals($d, []);
	}

	public function test_array_remove_key_not_exists() {
		$d = ["b" => "yyy"];
		$this->assertFalse(array_remove_key($d, "a"));
	}

	public function test_explode_safe_1() {
		$parts = explode_safe(",", "");
		$this->assertInternalType("array", $parts);
		$this->assertEmpty($parts);
	}

	public function test_explode_safe_2() {
		$parts = explode_safe(",", "1,2,3");
		$this->assertInternalType("array", $parts);
		$this->assertEquals($parts, ["1", "2", "3"]);
	}

	public function test_array_boolval_1() {
		$this->assertTrue(array_boolval(["a" => "1"], "a"));
	}

	public function test_array_boolval_2() {
		$this->assertTrue(array_boolval(["a" => "ON"], "a"));
	}

	public function test_array_boolval_3() {
		$this->assertTrue(array_boolval(["a" => "yes"], "a"));
	}

	public function test_array_boolval_4() {
		$this->assertTrue(array_boolval(["a" => "Y"], "a"));
	}

	public function test_array_boolval_5() {
		$this->assertTrue(array_boolval(["a" => "true"], "a"));
	}

	public function test_array_boolval_6() {
		$this->assertTrue(array_boolval(["a" => TRUE], "a"));
	}

	public function test_array_boolval_7() {
		$this->assertTrue(array_boolval(["a" => 1], "a"));
	}

	public function test_array_boolval_8() {
		$this->assertFalse(array_boolval(["b" => "no"], "b"));
	}

	public function test_array_boolval_9() {
		$this->assertFalse(array_boolval(["b" => FALSE], "b"));
	}

	public function test_array_boolval_10() {
		$this->assertFalse(array_boolval(["b" => 0], "b"));
	}

	public function test_array_boolval_11() {
		$this->assertFalse(array_boolval(["a" => "b"], "c"));
	}

	public function test_array_boolval_12() {
		$this->assertFalse(array_boolval("a", "c"));
	}

	public function test_array_boolval_13() {
		$this->assertTrue(array_boolval("a", "c", TRUE));
	}

	public function test_array_sort_key_1() {
		$d = [
			['id' => 3, 'text' => 'b'],
			['id' => 1, 'text' => 'a'],
			['id' => 2, 'text' => 'c']
		];
		$this->assertTrue(array_sort_key($d, "id"));
		$this->assertEquals($d, [
			['id' => 1, 'text' => 'a'],
			['id' => 2, 'text' => 'c'],
			['id' => 3, 'text' => 'b']
		]);
	}

	public function test_array_sort_key_2() {
		$d = [
			['id' => 3, 'text' => 'b'],
			['id' => 1, 'text' => 'a'],
			['id' => 2, 'text' => 'c']
		];
		$this->assertTrue(array_sort_key($d, "text"));
		$this->assertEquals($d, [
			['id' => 1, 'text' => 'a'],
			['id' => 3, 'text' => 'b'],
			['id' => 2, 'text' => 'c']
		]);
	}

	public function test_is_assoc_array_1() {
		$this->assertTrue(is_assoc_array([
			'foo' => 1, 'bar' => 'test', 'baz' => ['id' => 1]
		]));
	}

	public function test_is_assoc_array_2() {
		$this->assertFalse(is_assoc_array([
			['id' => 3, 'text' => 'b'],
			['id' => 1, 'text' => 'a'],
			['id' => 2, 'text' => 'c']
		]));
	}

	public function test_is_assoc_array_3() {
		$this->assertFalse(is_assoc_array(['foo', 'bar']));
	}

	public function test_is_assoc_array_4() {
		$this->assertFalse(is_assoc_array('foo'));
	}
}
