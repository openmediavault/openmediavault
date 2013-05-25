<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2013 Volker Theile
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

/**
 * This class provides all pre-defined error messages.
 * @ingroup api
 */
class OMVErrorMsg {
	// RPC
	const E_RPC_GET_PARAMS_FAILED = 1000;
	const E_RPC_INVALID_PARAMS = 1001;
	const E_RPC_INVALID_CONTEXT = 1002;
	// RPC services
	const E_RPC_SERVICE_NOT_FOUND = 2000;
	const E_RPC_SERVICE_METHOD_NOT_EXISTS = 2001;
	const E_RPC_SERVICE_METHOD_INVALID_PARAMS = 2002;
	const E_RPC_SERVICE_METHOD_INVALID_CONTEXT = 2003;
	const E_RPC_SERVICE_METHOD_SCHEMA_NOT_DEFINED = 2004;
	const E_RPC_SERVICE_ALREADY_EXISTS = 2005;
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
	const E_CONFIG_DIRTY = 3009;
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
	const E_MISC_DEVICE_NOT_FOUND = 6002;
	const E_MISC_DEVICE_NOT_SUPPORTED = 6003;
	// Engined
	const E_ENGINED_CREATE_SOCKET = 7000;
	const E_ENGINED_CONNECT_SOCKET = 7001;
	const E_ENGINED_WRITE_SOCKET = 7002;
	const E_ENGINED_READ_SOCKET = 7003;

	static $msg = array(
		self::E_RPC_GET_PARAMS_FAILED => "Failed to get RPC parameters",
		self::E_RPC_INVALID_PARAMS => "Invalid RPC parameters: %s",
		self::E_RPC_INVALID_CONTEXT => "Invalid context (key '%s' not found)",
		self::E_RPC_SERVICE_METHOD_NOT_EXISTS => "The method '%s' does not exist for RPC service '%s'",
		self::E_RPC_SERVICE_METHOD_INVALID_PARAMS => "Invalid method parameter: %s",
		self::E_RPC_SERVICE_METHOD_INVALID_CONTEXT => "Invalid context",
		self::E_RPC_SERVICE_METHOD_SCHEMA_NOT_DEFINED => "No schema defined for method %s",
		self::E_RPC_SERVICE_NOT_FOUND => "RPC service not found (name=%s)",
		self::E_RPC_SERVICE_ALREADY_EXISTS => "RPC service already exists (name=%s, class=%s)",
		self::E_CONFIG_OBJECT_NOT_FOUND => "Configuration object not found (xpath=%s)",
		self::E_CONFIG_LOAD_FAILED => "Failed to load configuration (%s)",
		self::E_CONFIG_SAVE_FAILED => "Failed to save configuration (%s)",
		self::E_CONFIG_SET_OBJECT_FAILED => "Failed to set configuration",
		self::E_CONFIG_GET_OBJECT_FAILED => "Failed to get configuration (xpath=%s)",
		self::E_CONFIG_INVALID_XPATH => "Invalid XPath",
		self::E_CONFIG_OBJECT_UNIQUENESS => "The configuration object is not unique: %s",
		self::E_CONFIG_OBJECT_INVALID => "The configuration object is invalid: %s",
		self::E_CONFIG_OBJECT_INUSE => "The configuration object is in use",
		self::E_CONFIG_DIRTY => "Please apply the configuration first.", 
		self::E_EXEC_FAILED => "Failed to execute command '%s': %s",
		self::E_EXEC_MISC => "%s",
		self::E_SESSION_NOT_AUTHENTICATED => "Session not authenticated",
		self::E_SESSION_TIMEOUT => "Session expired",
		self::E_SESSION_INVALID_IPADDRESS => "Invalid IP address",
		self::E_SESSION_INVALID_USERAGENT => "Invalid User-Agent",
		self::E_SESSION_INVALID_USER => "Invalid user",
		self::E_SESSION_ALREADY_AUTHENTICATED => "Another user is already authenticated",
		self::E_MISC_FAILURE => "%s",
		self::E_MISC_OPERATION_DENIED => "The operation is denied",
		self::E_MISC_DEVICE_NOT_FOUND => "Device '%s' not found",
		self::E_MISC_DEVICE_NOT_SUPPORTED => "Device '%s' not supported",
		self::E_ENGINED_CREATE_SOCKET => "Failed to create socket: %s",
		self::E_ENGINED_CONNECT_SOCKET => "Failed to connect to socket: %s",
		self::E_ENGINED_WRITE_SOCKET => "Failed to write to socket: %s",
		self::E_ENGINED_READ_SOCKET => "Failed to read from socket: %s"
	);

	/**
	 * Get the message for the given error code.
	 * @param code The error code.
	 * @return The error message.
	 */
	static function getMessage($code) {
		$msg = "";
		if(array_key_exists($code, self::$msg)) {
			$msg = sprintf("%s", self::$msg[$code]);
		} else {
			$msg = sprintf(gettext("Error #%d"), $code);
		}
		return $msg;
	}
}

/**
 * OMVException is the base class for all exceptions.
 * @ingroup api
 */
class OMVException extends Exception {
	/**
	 * Constructor
	 * @param code The exception code.
	 * @param args The additional function arguments.
	 */
	public function __construct() {
		$args = func_get_args();
		$code = array_shift($args);
		$format = OMVErrorMsg::getMessage($code);
		$message = vsprintf($format, $args);
		parent::__construct($message, $code);
	}
}

/**
 * OMVException2 is the base class for all redirected exceptions.
 * @ingroup api
 */
class OMVException2 extends Exception {
	private $trace; /**< The exception stack trace. */

	/**
	 * Constructor
	 * @param code The exception code.
	 * @param message The exception message to throw.
	 * @param trace The exception stack trace.
	 */
	public function __construct($code, $message, $trace) {
		$this->trace = $trace;
		parent::__construct($message, $code);
	}

	/**
	 * String representation of the exception.
	 * @return Returns the string representation of the exception.
	 */
	public function __toString() {
		return $this->trace;
	}
}
?>
