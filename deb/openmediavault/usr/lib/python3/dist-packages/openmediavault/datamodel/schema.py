# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2017 Volker Theile
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
# along with OpenMediaVault. If not, see <http://www.gnu.org/licenses/>.
__all__ = [ "Schema" ]

import re
import openmediavault.string
import openmediavault.json

class Schema(openmediavault.json.Schema):
	def _check_format(self, value, schema, name):
		try:
			# Check the default format values defined by JSON schema.
			# Fall through for unknown types.
			super()._check_format(value, schema, name)
		except openmediavault.json.SchemaException as e:
			if "uuidv4" == schema['format']:
				if not openmediavault.string.is_uuid4(value):
					raise openmediavault.json.SchemaValidationException(name,
						"The value '%s' is not a valid UUIDv4." % value)
			elif "fsuuid" == schema['format']:
				if not openmediavault.string.is_fs_uuid(value):
					raise openmediavault.json.SchemaValidationException(name,
						"The value '%s' is not a valid filesystem UUID." %
						value)
			elif "devicefile" == schema['format']:
				if not re.match(r'/^\/dev(\/disk\/by-id)?\/.+$/i', value):
					raise openmediavault.json.SchemaValidationException(name,
						"The value '%s' is no device file name." % value)
			elif "dirpath" == schema['format']:
				if not re.match(r'/^(?!.*[\/]\.{2}[\/])(?!\.{2}[\/])' \
					'[-\w.\/]+$/u', value):
					raise openmediavault.json.SchemaValidationException(name,
						"The value '%s' is no valid directory path." % value)
			elif "sshpubkey-openssh" == schema['format']:
				if not re.match(r'/^ssh-rsa AAAA[0-9A-Za-z+\/]+[=]{0,3}\s*' \
					'([^@]+@[^@]+|.+)*$/', value):
					raise openmediavault.json.SchemaValidationException(name,
						"The value '%s' is no SSH public key (OpenSSH)." %
						value)
			elif "sshpubkey-rfc4716" == schema['format']:
				if not re.match(r'/^---- BEGIN SSH2 PUBLIC KEY ----' \
					'(\n|\r|\f)(.+)(\n|\r|\f)' \
					'---- END SSH2 PUBLIC KEY ----$/sm', value):
					raise openmediavault.json.SchemaValidationException(name,
						"The value '%s' is no SSH public key (RFC4716)." %
						value)
			elif "sshprivkey-rsa" == schema['format']:
				if not re.match(r'/^-----BEGIN RSA PRIVATE KEY-----' \
					'(\n|\r|\f)(.+)(\n|\r|\f)' \
					'-----END RSA PRIVATE KEY-----$/sm', value):
					raise openmediavault.json.SchemaValidationException(name,
						"The value '%s' is no SSH private key (RSA)." % value)
			elif "sharename" == schema['format']:
				# We are using the SMB/CIFS file/directory naming convention
				# for this:
				# All characters are legal in the basename and extension
				# except the space character (0x20) and:
				# "./\[]:+|<>=;,*?
				# A share name or server or workstation name SHOULD not
				# begin with a period (“.”) nor should it include two
				# adjacent periods (“..”).
				# References:
				# http://tools.ietf.org/html/draft-leach-cifs-v1-spec-01
				# http://msdn.microsoft.com/en-us/library/aa365247%28VS.85%29.aspx
				if not re.match(r'/^[^.]([^"/\\\[\]:+|<>=;,*?. ]+){0,1}([.]' \
					'[^"/\\\[\]:+|<>=;,*?. ]+){0,}$/', value):
					raise openmediavault.json.SchemaValidationException(name,
						"The value '%s' is no valid share name." % value)
			elif "username" == schema['format']:
				if not re.match(r'/^[_.A-Za-z0-9][-\@_.A-Za-z0-9]*\$?$/', value):
					raise openmediavault.json.SchemaValidationException(name,
						"The value '%s' is no valid user name." % value)
			elif "domainname" == schema['format']:
				if not re.match(r'/^[a-zA-Z0-9]([-a-zA-Z0-9]{0,61}' \
					'[a-zA-Z0-9])?([.][a-zA-Z0-9]([-a-zA-Z0-9]{0,61}' \
					'[a-zA-Z0-9])?)*$/', value):
					raise openmediavault.json.SchemaValidationException(name,
						"The value '%s' is no valid domain name." % value)
			else:
				raise SchemaException(
					"%s: The format '%s' is not defined." %
					(name, schema['format']))

if __name__ == "__main__":
	import unittest

	class SchemaTestCase(unittest.TestCase):
		def _get_schema(self):
			return Schema({
				"type": "object",
				"properties": {
					"fsname": {
						"type": "string",
						"oneOf": [{
							"type": "string",
							"format": "fsuuid"
						},{
							"type": "string",
							"format": "devicefile"
						},{
							"type": "string",
							"format": "dirpath"
						}]
					}
				}
			})

		def test_check_format_fsuuid_1(self):
			# EXT2/3/4, JFS, XFS
			schema = Schema({})
			schema._check_format("113dbaac-e496-11e6-ac68-73bc0f572bae",
				{ "format": "fsuuid" }, "field1")

		def test_check_format_fsuuid_2(self):
			# FAT
			schema = Schema({})
			schema._check_format("7A48-BA97",
				{ "format": "fsuuid" }, "field1")

		def test_check_format_fsuuid_3(self):
			# NTFS
			schema = Schema({})
			schema._check_format("2ED43920D438EC29",
				{ "format": "fsuuid" }, "field1")

		def test_check_format_fsuuid_3(self):
			# ISO9660
			schema = Schema({})
			schema._check_format("2015-01-13-21-48-46-00",
				{ "format": "fsuuid" }, "field1")

		def test_check_format_devicefile_1(self):
			schema = Schema({})
			schema._check_format("/dev/sda1",
				{ "format": "devicefile" }, "field1")

		def test_check_format_devicefile_2(self):
			schema = Schema({})
			schema._check_format("/dev/disk/by-id/wwn-0x5020c298d81c1c3a",
				{ "format": "devicefile" }, "field1")

		def test_check_format_dirpath(self):
			schema = Schema({})
			schema._check_format("/media/a/b/c",
				{ "format": "dirpath" }, "field1")

	unittest.main()
