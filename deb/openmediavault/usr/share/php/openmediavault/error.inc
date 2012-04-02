<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2012 Volker Theile
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

class OMVErrorMsg {
	// RPC
	const E_RPC_GET_PARAMS_FAILED = 1000;
	const E_RPC_INVALID_PARAMS = 1001;
	// RPC services
	const E_RPC_SERVICE_FILE_NOT_FOUND = 2000;
	const E_RPC_SERVICE_CLASS_NOT_FOUND = 2001;
	const E_RPC_SERVICE_METHOD_NOT_FOUND = 2002;
	const E_RPC_SERVICE_INVALID_PARAM = 2003;
	const E_RPC_SERVICE_INVALID_PERMISSION = 2004;
	const E_RPC_SERVICE_SCHEMA_NOT_DEFINED = 2005;
	// Configuration
	const E_CONFIG_OBJECT_NOT_FOUND = 3000;
	const E_CONFIG_SAVE_FAILED = 3001;
	const E_CONFIG_SET_OBJECT_FAILED = 3002;
	const E_CONFIG_GET_OBJECT_FAILED = 3003;
	const E_CONFIG_INVALID_XPATH = 3004;
	const E_CONFIG_OBJECT_UNIQUENESS = 3005;
	const E_CONFIG_OBJECT_INVALID = 3006;
	const E_CONFIG_OBJECT_INUSE = 3007;
	const E_CONFIG_LOAD_FAILED = 3008;
	// Exec
	const E_EXEC_FAILED = 4000;
	const E_EXEC_MISC = 4001;
	// Session
	const E_SESSION_NOT_AUTHENTICATED = 5000;
	const E_SESSION_TIMEOUT = 5001;
	const E_SESSION_INVALID_IPADDRESS = 5002;
	const E_SESSION_INVALID_USERAGENT = 5003;
	const E_SESSION_INVALID_USER = 5004;
	const E_SESSION_ALREADY_AUTHENTICATED = 5005;
	// Misc
	const E_MISC_FAILURE = 6000;
	const E_MISC_OPERATION_DENIED = 6001;
	const E_MISC_INVALID_PARAM = 6002;

	static $msg = array(
		self::E_RPC_GET_PARAMS_FAILED => "Failed to get RPC parameters",
		self::E_RPC_INVALID_PARAMS => "Invalid RPC parameters: %s",
		self::E_RPC_SERVICE_FILE_NOT_FOUND => "File '%s' not found",
		self::E_RPC_SERVICE_CLASS_NOT_FOUND => "Class '%s' not found",
		self::E_RPC_SERVICE_METHOD_NOT_FOUND => "The method '%s' does not exist for class '%s'",
		self::E_RPC_SERVICE_INVALID_PARAM => "Invalid method parameter: %s",
		self::E_RPC_SERVICE_INVALID_PERMISSION => "Invalid permission",
		self::E_RPC_SERVICE_SCHEMA_NOT_DEFINED => "No schema defined for method %s",
		self::E_CONFIG_OBJECT_NOT_FOUND => "Configuration object not found (xpath=%s)",
		self::E_CONFIG_LOAD_FAILED => "Failed to load configuration (%s)",
		self::E_CONFIG_SAVE_FAILED => "Failed to save configuration (%s)",
		self::E_CONFIG_SET_OBJECT_FAILED => "Failed to set configuration",
		self::E_CONFIG_GET_OBJECT_FAILED => "Failed to get configuration (xpath=%s)",
		self::E_CONFIG_INVALID_XPATH => "Invalid XPath",
		self::E_CONFIG_OBJECT_UNIQUENESS => "The configuration object is not unique: %s",
		self::E_CONFIG_OBJECT_INVALID => "The configuration object is invalid: %s",
		self::E_CONFIG_OBJECT_INUSE => "The configuration object is in use",
		self::E_EXEC_FAILED => "Failed to execute command '%s': %s",
		self::E_EXEC_MISC => "%s",
		self::E_SESSION_NOT_AUTHENTICATED => "Session not authenticated",
		self::E_SESSION_TIMEOUT => "Session timeout",
		self::E_SESSION_INVALID_IPADDRESS => "Invalid IP address",
		self::E_SESSION_INVALID_USERAGENT => "Invalid User-Agent",
		self::E_SESSION_INVALID_USER => "Invalid user",
		self::E_SESSION_ALREADY_AUTHENTICATED => "Another user is already authenticated",
		self::E_MISC_FAILURE => "%s",
		self::E_MISC_OPERATION_DENIED => "The operation is denied",
		self::E_MISC_INVALID_PARAM => "Invalid parameter (%s): %s"
	);

	static function getMessage($code) {
		$msg = "";
		if (array_key_exists($code, self::$msg)) {
			$msg = sprintf("%s", self::$msg[$code]);
		} else {
			$msg = sprintf(gettext("Error #%d"), $code);
		}
		return $msg;
	}
}

class OMVException extends Exception {
	/**
	 * Constructor
	 * @param code The exception code
	 */
	function __construct() {
		$args = func_get_args();
		$code = array_shift($args);
		$msg = OMVErrorMsg::getMessage($code);
		parent::__construct(vsprintf($msg, $args), $code);
	}
}
?>
