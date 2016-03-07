<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2016 Volker Theile
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

require_once("openmediavault/object.inc");
require_once("openmediavault/functions.inc");
require_once("openmediavault/json/schema.inc");

class Schema extends \OMV\Json\Schema {
	final public function getTypeByPath($name) {
		if (FALSE === ($schema = $this->getAssocByPath($name)))
			return FALSE;
		return $schema['type'];
	}

	/**
	 * Add some more formats than JSON schema supports.
	 */
	protected function checkFormat($value, $schema, $name) {
		if (TRUE === parent::checkFormat($value, $schema, $name))
			return TRUE;
		$this->clearLastError();
		$valid = TRUE;
		switch ($schema['format']) {
		case "ipv4":
			if (!filter_var($value, FILTER_VALIDATE_IP, FILTER_FLAG_IPV4)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' is not an IPv4 address.",
				  $name, $value));
			}
			break;
		case "uuidv4":
			if (!is_uuid($value)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' is not an UUIDv4.",
				  $name, $value));
			}
			break;
		case "fsuuid":
			if (!is_fs_uuid($value)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' is not a valid filesystem UUID.",
				  $name, $value));
			}
			break;
		case "devicefile":
			if (!preg_match('/^\/dev(\/disk\/by-id)?\/.+$/i', $value)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' is no device file name.",
				  $name, $value));
			}
			break;
		case "sshpubkey-openssh":
			if (!preg_match('/^ssh-rsa AAAA[0-9A-Za-z+\/]+[=]{0,3}\s*'.
			  '([^@]+@[^@]+|.+)*$/', $value)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' is no SSH public key (OpenSSH).",
				  $name, $value));
			}
			break;
		case "sshpubkey-RFC4716":
			if (!preg_match('/^---- BEGIN SSH2 PUBLIC KEY ----(\n|\r|\f)(.+)'.
		      '(\n|\r|\f)---- END SSH2 PUBLIC KEY ----$/sm', $value)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' is no SSH public key (RFC4716).",
				  $name, $value));
			}
			break;
		case "sshprivkey-RSA":
			if (!preg_match(/^-----BEGIN RSA PRIVATE KEY-----(\n|\r|\f)(.+)'.
		      '(\n|\r|\f)-----END RSA PRIVATE KEY-----$/sm, $value)) {
				$valid = FALSE;
				$this->setLastError(
				  sprintf("%s: The value '%s' is no SSH private key (RSA).",
				  $name, $value));
			}
			break;
		default:
			$valid = FALSE;
			$this->setLastError(
			  sprintf("%s: The format '%s' is not defined.",
			  $name, $schema['format']));
			break;
		}
		return $valid;
	}
}
