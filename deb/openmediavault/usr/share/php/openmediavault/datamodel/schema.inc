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
namespace OMV\DataModel;

require_once("openmediavault/functions.inc");

class Schema extends \OMV\Json\Schema {
	/**
	 * Add some more formats than JSON schema supports.
	 */
	protected function checkFormat($value, $schema, $name) {
		try {
			// Check the default format values defined by JSON schema.
			// Fall through for unknown types.
			parent::checkFormat($value, $schema, $name);
		} catch (\OMV\Json\SchemaException $e) {
			switch ($schema['format']) {
			case "uuidv4":
				if (!is_uuid($value)) {
					throw new \OMV\Json\SchemaValidationException(
					  "%s: The value '%s' is not an UUIDv4.",
					  $name, $value);
				}
				break;
			case "fsuuid":
				if (!is_fs_uuid($value)) {
					throw new \OMV\Json\SchemaValidationException(
					  "%s: The value '%s' is not a valid filesystem UUID.",
					  $name, $value);
				}
				break;
			case "devicefile":
				if (!preg_match('/^\/dev(\/disk\/by-id)?\/.+$/i', $value)) {
					throw new \OMV\Json\SchemaValidationException(
					  "%s: The value '%s' is not a device file.",
					  $name, $value);
				}
				break;
			case "dirpath":
				if (!preg_match('/^(?!.*[\/]\.{2}[\/])(?!\.{2}[\/])'.
				  '[-\w.\/@ ]+$/u', $value)) {
					throw new \OMV\Json\SchemaValidationException(
					  "%s: The value '%s' is not a valid directory path.",
					  $name, $value);
				}
				break;
			case "sshpubkey-openssh":
				if (!\OMV\Ssh\PublicKey::isOpenSSH($value)) {
					throw new \OMV\Json\SchemaValidationException(
					  "%s: The value '%s' is not a SSH public key (OpenSSH).",
					  $name, $value);
				}
				break;
			case "sshpubkey-rfc4716":
				if (!\OMV\Ssh\PublicKey::isRfc4716($value)) {
					throw new \OMV\Json\SchemaValidationException(
					  "%s: The value '%s' is not a SSH public key (RFC4716).",
					  $name, $value);
				}
				break;
			case "sshprivkey-rsa":
				// Deprecated: Use "sshprivkey-pem" instead.
				if (!preg_match('/^-----BEGIN RSA PRIVATE KEY-----(\n|\r|\f)(.+)'.
				  '(\n|\r|\f)-----END RSA PRIVATE KEY-----$/sm', $value)) {
					throw new \OMV\Json\SchemaValidationException(
					  "%s: The value '%s' is not a SSH private key (PEM).",
					  $name, $value);
				}
				break;
			case "sshprivkey-pem":
				if (!preg_match('/^-----BEGIN (OPENSSH|RSA) PRIVATE KEY-----(\n|\r|\f)(.+)'.
				  '(\n|\r|\f)-----END (OPENSSH|RSA) PRIVATE KEY-----$/sm', $value)) {
					throw new \OMV\Json\SchemaValidationException(
					  "%s: The value '%s' is not a SSH private key (PEM).",
					  $name, $value);
				}
				break;
			case "pgppubkey":
				if (!preg_match('/^-----BEGIN PGP PUBLIC KEY BLOCK-----(\n|\r|\f)(.+)'.
				  '(\n|\r|\f)-----END PGP PUBLIC KEY BLOCK-----$/sm', $value)) {
					throw new \OMV\Json\SchemaValidationException(
					  "%s: The value '%s' is not a PGP public key.",
					  $name, $value);
				}
				break;
			case "sharename":
				// We are using the SMB/CIFS file/directory naming convention
				// for this:
				// All characters are legal in the basename and extension
				// except the space character (0x20) and:
				// "./\[]:+|<>=;,*?
				// A share name or server or workstation name SHOULD not
				// begin with a period (“.”) nor should it include two
				// adjacent periods (“..”).
				// References:
				// http://tools.ietf.org/html/draft-leach-cifs-v1-spec-01
				// http://msdn.microsoft.com/en-us/library/aa365247%28VS.85%29.aspx
				if (!preg_match('/^[^.]([^"/\\\[\]:+|<>=;,*?. ]+){0,1}([.]'.
				  '[^"/\\\[\]:+|<>=;,*?. ]+){0,}$/', $value)) {
					throw new \OMV\Json\SchemaValidationException(
					  "%s: The value '%s' is not a valid share name.",
					  $name, $value);
				}
				break;
			case "username":
				// Regular expression taken from Debian adduser.
				if (!preg_match('/^[_.A-Za-z0-9][-\@_.A-Za-z0-9]*\$?$/',
				  $value)) {
					throw new \OMV\Json\SchemaValidationException(
					  "%s: The value '%s' is not a valid user name.",
					  $name, $value);
				}
				break;
			case "domainname":
				// http://shauninman.com/archive/2006/05/08/validating_domain_names
				if (!preg_match('/^[a-zA-Z0-9]([-a-zA-Z0-9]{0,61}'.
				  '[a-zA-Z0-9])?([.][a-zA-Z0-9]([-a-zA-Z0-9]{0,61}'.
				  '[a-zA-Z0-9])?)*$/', $value)) {
					throw new \OMV\Json\SchemaValidationException(
					  "%s: The value '%s' is not a valid domain name.",
					  $name, $value);
				}
				break;
			case "netbiosname":
				// https://github.com/samba-team/samba/blob/samba-4.13.13/python/samba/__init__.py#L317
				if (!preg_match('/^[\w !#$%&\'()\-.@^_{}~]{1,15}$/',
				  $value)) {
					throw new \OMV\Json\SchemaValidationException(
					  "%s: The value '%s' is not a valid NetBIOS name.",
					  $name, $value);
				}
				break;
			default:
				throw new \OMV\Json\SchemaException(
				  "%s: The format '%s' is not defined.",
				  $name, $schema['format']);
				break;
			}
		}
	}
}
