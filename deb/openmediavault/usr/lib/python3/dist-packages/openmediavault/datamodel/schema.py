# -*- coding: utf-8 -*-
#
# This file is part of OpenMediaVault.
#
# @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
# @author    Volker Theile <volker.theile@openmediavault.org>
# @copyright Copyright (c) 2009-2025 Volker Theile
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
__all__ = ["Schema"]

import re

import openmediavault.json.schema
import openmediavault.stringutils


class Schema(openmediavault.json.Schema):
    # pylint: disable=too-many-branches
    def _check_format(self, value, schema, name):
        try:
            # Check the default format values defined by JSON schema.
            # Fall through for unknown types.
            super()._check_format(value, schema, name)
        except openmediavault.json.SchemaException:
            if "uuidv4" == schema['format']:
                if not openmediavault.stringutils.is_uuid4(value):
                    raise openmediavault.json.SchemaValidationException(
                        name, "The value '%s' is not a valid UUIDv4." % value
                    ) from None
            elif "fsuuid" == schema['format']:
                if not openmediavault.stringutils.is_fs_uuid(value):
                    raise openmediavault.json.SchemaValidationException(
                        name,
                        "The value '%s' is not a valid filesystem UUID."
                        % value,
                    ) from None
            elif "devicefile" == schema['format']:
                if not re.match(
                    r'^\/dev(\/disk\/by-id)?\/.+$', value, flags=re.IGNORECASE
                ):
                    raise openmediavault.json.SchemaValidationException(
                        name, "The value '%s' is no device file name." % value
                    ) from None
            elif "dirpath" == schema['format']:
                if not re.match(
                    r'^(?!.*[\/]\.{2}[\/])(?!\.{2}[\/])[-\w.\/@ ]+$',
                    value,
                    flags=re.UNICODE,
                ):
                    raise openmediavault.json.SchemaValidationException(
                        name,
                        "The value '%s' is not a valid directory path." % value,
                    ) from None
            elif "sshpubkey-openssh" == schema['format']:
                if not re.match(
                    r'^(sk-ssh-ed25519@openssh\.com|ssh-(rsa|ed25519)) AAAA[0-9A-Za-z+\/]+[=]{0,3}\s*'
                    r'([^@]+@[^@]+|.+)*$',
                    value,
                ):
                    raise openmediavault.json.SchemaValidationException(
                        name,
                        "The value '%s' is no SSH public key (OpenSSH)."
                        % value,
                    ) from None
            elif "sshpubkey-rfc4716" == schema['format']:
                if not re.match(
                    r'^---- BEGIN SSH2 PUBLIC KEY ----'
                    r'(\n|\r|\f)(.+)(\n|\r|\f)'
                    r'---- END SSH2 PUBLIC KEY ----$',
                    value,
                    flags=re.DOTALL | re.MULTILINE,
                ):
                    raise openmediavault.json.SchemaValidationException(
                        name,
                        "The value '%s' is no SSH public key (RFC4716)."
                        % value,
                    ) from None
            elif "sshprivkey-rsa" == schema['format']:
                # Deprecated: Use "sshprivkey-pem" instead.
                if not re.match(
                    r'^-----BEGIN RSA PRIVATE KEY-----'
                    r'(\n|\r|\f)(.+)(\n|\r|\f)'
                    r'-----END RSA PRIVATE KEY-----$',
                    value,
                    flags=re.DOTALL | re.MULTILINE,
                ):
                    raise openmediavault.json.SchemaValidationException(
                        name,
                        "The value '%s' is no SSH private key (PEM)." % value,
                    ) from None
            elif "sshprivkey-pem" == schema['format']:
                if not re.match(
                    r'^-----BEGIN (OPENSSH|RSA) PRIVATE KEY-----'
                    r'(\n|\r|\f)(.+)(\n|\r|\f)'
                    r'-----END (OPENSSH|RSA) PRIVATE KEY-----$',
                    value,
                    flags=re.DOTALL | re.MULTILINE,
                ):
                    raise openmediavault.json.SchemaValidationException(
                        name,
                        "The value '%s' is no SSH private key (PEM)." % value,
                    ) from None
            elif "pgppubkey" == schema['format']:
                if not re.match(
                    r'^-----BEGIN PGP PUBLIC KEY BLOCK-----'
                    r'(\n|\r|\f)(.+)(\n|\r|\f)'
                    r'-----END PGP PUBLIC KEY BLOCK-----$',
                    value,
                    flags=re.DOTALL | re.MULTILINE,
                ):
                    raise openmediavault.json.SchemaValidationException(
                        name,
                        "The value '%s' is no PGP public key." % value,
                    ) from None
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
                if not re.match(
                    r'^[^.]([^"/\\\[\]:+|<>=;,*?. ]+)?([.]'
                    r'[^"/\\\[\]:+|<>=;,*?. ]+)*$',
                    value,
                ):
                    raise openmediavault.json.SchemaValidationException(
                        name, "The value '%s' is not a valid share name." % value
                    ) from None
            elif "username" == schema['format']:
                if not re.match(r'^[_.A-Za-z0-9][-\@_.A-Za-z0-9]*\$?$', value):
                    raise openmediavault.json.SchemaValidationException(
                        name, "The value '%s' is not a valid user name." % value
                    )
            elif "domainname" == schema['format']:
                if not re.match(
                    r'^[a-zA-Z0-9]([-a-zA-Z0-9]{0,61}'
                    r'[a-zA-Z0-9])?([.][a-zA-Z0-9]([-a-zA-Z0-9]{0,61}'
                    r'[a-zA-Z0-9])?)*$',
                    value,
                ):
                    raise openmediavault.json.SchemaValidationException(
                        name, "The value '%s' is not a valid domain name." % value
                    ) from None
            elif "netbiosname" == schema['format']:
                # https://github.com/samba-team/samba/blob/samba-4.13.13/python/samba/__init__.py#L317
                if not re.match(r'^[\w !#$%&\'()\-.@^_{}~]{1,15}$', value):
                    raise openmediavault.json.SchemaValidationException(
                        name, "The value '%s' is not a valid NetBIOS name." % value
                    ) from None
            else:
                raise openmediavault.json.SchemaException(
                    "%s: The format '%s' is not defined."
                    % (name, schema['format'])
                ) from None
