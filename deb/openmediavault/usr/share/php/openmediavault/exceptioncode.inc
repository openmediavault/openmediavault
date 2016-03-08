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
namespace OMV;

class ExceptionCode {
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
	const E_CONFIG_UNMET_DEPENDENCY = 3010;
	const E_CONFIG_DB_SET_FAILED = 3011;
	const E_CONFIG_DB_GET_FAILED = 3012;
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
	// Data model
	const E_DATAMODEL_LOAD_FAILED = 8000;
	const E_DATAMODEL_NOT_EXISTS = 8001;
}
