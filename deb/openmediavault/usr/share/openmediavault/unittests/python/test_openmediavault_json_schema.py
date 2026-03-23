# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2026 Volker Theile
#
# OpenMediaVault is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# OpenMediaVault is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.
import unittest

import openmediavault.json.schema


class SchemaTestCase(unittest.TestCase):
    def _get_schema(self):
        return openmediavault.json.Schema(
            {
                "type": "object",
                "properties": {
                    "name": {"type": "string", "required": True},
                    "price": {"type": "number", "minimum": 35, "maximum": 40},
                    "ntp": {
                        "type": "object",
                        "properties": {
                            "enable": {"type": "boolean", "default": False},
                            "timeservers": {
                                "type": "string",
                                "default": "pool.ntp.org",
                            },
                            "clients": {
                                "type": "string",
                                "default": "",
                            },
                        },
                    },
                    "privilege": {
                        "type": "array",
                        "items": {
                            "type": "object",
                            "properties": {
                                "type": {
                                    "type": "string",
                                    "enum": ["user", "group"],
                                },
                                "name": {"type": "string"},
                                "perms": {"type": "integer", "enum": [0, 5, 7]},
                            },
                        },
                    },
                    "slaves": {
                        "type": "string",
                        "pattern": "^(((eth|wlan)\\d+|(en|wl)\\S+),)*((eth|wlan)\\d+|(en|wl)\\S+)$",
                    },
                },
            }
        )

    def test_as_dict(self):
        schema = self._get_schema()
        self.assertIsInstance(schema.as_dict(), dict)

    def test_get_by_path_1(self):
        schema = self._get_schema()
        self.assertIsInstance(schema.get_by_path("price"), dict)

    def test_get_by_path_2(self):
        schema = self._get_schema()
        self.assertEqual(
            schema.get_by_path("ntp.enable"),
            {"type": "boolean", "default": False},
        )

    def test_get_by_path_fail(self):
        schema = self._get_schema()
        self.assertRaises(
            openmediavault.json.SchemaPathException,
            lambda: schema.get_by_path("a.b.c"),
        )

    def test_validate_fail(self):
        schema = self._get_schema()
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema.validate({"price": 38}),
        )

    def test_validate_maximum_fail(self):
        schema = self._get_schema()
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema.validate({"name": "Apple", "price": 41}),
        )

    def test_validate_minimum_fail(self):
        schema = self._get_schema()
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema.validate({"name": "Eggs", "price": 34.99}),
        )

    def test_check_format_unknown_fail(self):
        schema = openmediavault.json.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaException,
            lambda: schema._check_format("abc", {"format": "abc"}, "abc"),
        )

    def test_check_format_regex(self):
        schema = openmediavault.json.Schema({})
        schema._check_format(
            r'^\d{4}-\d{2}-\d{2}$', {"format": "regex"}, "field1"
        )

    def test_check_format_email(self):
        schema = openmediavault.json.Schema({})
        schema._check_format("test@test.com", {"format": "email"}, "field2")

    def test_check_format_hostname(self):
        schema = openmediavault.json.Schema({})
        schema._check_format("myvault", {"format": "host-name"}, "field3")

    def test_check_format_hostname_fail(self):
        schema = openmediavault.json.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "myvault.local", {"format": "host-name"}, "field3"
            ),
        )

    def test_check_format_regex(self):
        schema = openmediavault.json.Schema({})
        patterns = [
            r"^((\S+\s+)*\S+)?$",
            r"^([a-fA-F0-9]{2}(:[a-fA-F0-9]{2}){5})?$",
            r"^(((eth|venet|wlan)\d+|(en|veth|wl)\S+|(bond)\d+)(\.\d+)?,)*((eth|venet|wlan)\d+|(en|veth|wl)\S+|(bond)\d+)(\.\d+)?$",
            r"^(\d+[KMGTP]?)?$",
            r"^(([a-zA-Z_]+)(=([\w@:\/]+))?[,])*([a-zA-Z_]+)(=([\w@:\/]+))?$",
            r"^(!?(tcp|udp|icmp|icmpv6))|all$",
            r"^((\S+[,;]\s*)*\S+)?$",
            r"^(br\d+)?$",
            r"^[0-9]|[1-5][0-9]$",
            r"^[0-9]|1[0-9]|2[0-3]|[*]$",
            r"^[1-9]|[12][0-9]|3[01]$",
        ]
        for i, pattern in enumerate(patterns):
            schema._check_format(pattern, {"format": "regex"}, f"core_{i}")

    def test_check_format_regex_fail(self):
        schema = openmediavault.json.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format("(invalid[", {"format": "regex"}, "field1"),
        )

    def test_check_format_regex_fail_2(self):
        schema = openmediavault.json.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                r"^([a-fA-F0-9]{2}(:[a-fA-F0-9]{2}){5}?$",
                {"format": "regex"},
                "broken_mac",
            ),
        )
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                r"^(([a-zA-Z_]+)(=([\w@:\/]+))?[,])*([a-zA-Z_]+)(=([\w@:\/]+))?$)",
                {"format": "regex"},
                "broken_exportopts",
            ),
        )

    def test_check_format_uri(self):
        schema = openmediavault.json.Schema({})
        schema._check_format(
            "https://www.example.com/path", {"format": "uri"}, "field1"
        )

    def test_check_format_uri_no_path(self):
        schema = openmediavault.json.Schema({})
        schema._check_format(
            "https://www.example.com", {"format": "uri"}, "field1"
        )

    def test_check_format_uri_with_path_only(self):
        schema = openmediavault.json.Schema({})
        schema._check_format(
            "mailto:user@example.com", {"format": "uri"}, "field1"
        )

    def test_check_format_uri_fail_empty(self):
        schema = openmediavault.json.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format("", {"format": "uri"}, "field1"),
        )

    def test_check_format_uri_fail_no_scheme(self):
        schema = openmediavault.json.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "www.example.com", {"format": "uri"}, "field1"
            ),
        )

    def test_check_format_datetime(self):
        schema = openmediavault.json.Schema({})
        schema._check_format(
            "2025-03-23T12:00:00Z", {"format": "date-time"}, "field1"
        )

    def test_check_format_datetime_fail(self):
        schema = openmediavault.json.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "not-a-date", {"format": "date-time"}, "field1"
            ),
        )

    def test_check_format_date(self):
        schema = openmediavault.json.Schema({})
        schema._check_format("2025-03-23", {"format": "date"}, "field1")

    def test_check_format_date_fail(self):
        schema = openmediavault.json.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "23-03-2025", {"format": "date"}, "field1"
            ),
        )

    def test_check_format_time(self):
        schema = openmediavault.json.Schema({})
        schema._check_format("12:30:59", {"format": "time"}, "field1")

    def test_check_format_time_fail(self):
        schema = openmediavault.json.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "12:30", {"format": "time"}, "field1"
            ),
        )

    def test_check_format_email_fail(self):
        schema = openmediavault.json.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "not-an-email", {"format": "email"}, "field1"
            ),
        )

    def test_check_format_ipv4(self):
        schema = openmediavault.json.Schema({})
        schema._check_format(
            "192.168.1.1", {"format": "ipv4"}, "field1"
        )

    def test_check_format_ipv4_fail(self):
        schema = openmediavault.json.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "999.999.999.999", {"format": "ipv4"}, "field1"
            ),
        )

    def test_check_format_ipv6(self):
        schema = openmediavault.json.Schema({})
        schema._check_format("::1", {"format": "ipv6"}, "field1")

    def test_check_format_ipv6_fail(self):
        schema = openmediavault.json.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_format(
                "not-an-ipv6", {"format": "ipv6"}, "field1"
            ),
        )

    def test_check_one_of(self):
        schema = openmediavault.json.Schema({})
        schema._check_format(
            "192.168.10.101",
            {
                "type": "string",
                "oneOf": [
                    {"type": "string", "format": "ipv6"},
                    {"type": "string", "format": "ipv4"},
                ],
            },
            "field3",
        )

    def test_validate_pattern(self):
        schema = self._get_schema()
        schema.validate({"name": "Eggs", "slaves": "eth0"})

    def test_validate_pattern_fail(self):
        schema = self._get_schema()
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema.validate({"name": "Eggs", "slaves": "xyz0"}),
        )

    def test_check_items_1(self):
        schema = openmediavault.json.Schema({})
        schema._check_items(
            ["192.168.10.101"],
            {"type": "array", "items": {"type": "string", "format": "ipv4"}},
            "foo",
        )

    def test_check_items_2(self):
        schema = openmediavault.json.Schema({})
        schema._check_items(
            ["192.168.10.101"],
            {
                "type": "array",
                "items": [
                    {"type": "string", "format": "ipv4"},
                    {"type": "string", "format": "ipv6"},
                ],
            },
            "xyz",
        )

    def test_check_items_3(self):
        schema = openmediavault.json.Schema({})
        self.assertRaises(
            openmediavault.json.SchemaValidationException,
            lambda: schema._check_items(
                [10, "192.168.10.101"],
                {
                    "type": "array",
                    "items": [
                        {"type": "string", "format": "ipv4"},
                        {"type": "string", "format": "ipv6"},
                    ],
                },
                "bar",
            ),
        )


if __name__ == "__main__":
    unittest.main()
