<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2011 Volker Theile
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
require_once("openmediavault/object.inc");
require_once("openmediavault/session.inc");
require_once("openmediavault/rpc.inc");

class AuthenticationRpc extends OMVRpc {
	public function __construct() {
		$this->methodSchemata = array(
			"login" => array('{
				"type":"object",
				"properties":{
					"username":{"type":"string"},
					"password":{"type":"string"}
				}
			}')
		);
	}

	/**
	 * Login user
	 */
	function login($data) {
//		$this->debug(var_export(func_get_args(), TRUE));
		global $xmlConfig;
		// Validate method parameters
		$this->validateParams(__METHOD__, func_get_args());
		// Validate username and password:
		// 1. Check if user exists in configuration
		// 2. Validate password via PAM
		$authenticated = FALSE;
		$xpath = sprintf("//system/usermanagement/users/user[name='%s']",
		  $data['username']);
		if (TRUE === $xmlConfig->exists($xpath)) {
			if (pam_auth($data['username'], $data['password'])) {
				$session = &OMVSession::getInstance();
				if ($session->isAuthenticated()) {
					// Is the current session registered to the user to be
					// authenticated?
					if ($session->getUsername() !== $data['username']) {
						throw new OMVException(
						  OMVErrorMsg::E_SESSION_ALREADY_AUTHENTICATED);
					}
				} else {
					// Initialize session.
					$role = ($data['username'] === "admin") ?
					  OMV_ROLE_ADMINISTRATOR : OMV_ROLE_USER;
					$session->initialize($data['username'], $role);
				}
				$authenticated = $session->isAuthenticated();
			}
		}
		if (FALSE === $authenticated) {
			syslog(LOG_ALERT, sprintf("Unauthorized login attempt from %s ".
			  "[username='%s', user-agent=%s]", $_SERVER['REMOTE_ADDR'],
			  $data['username'], $_SERVER['HTTP_USER_AGENT']));
		}
		return array(
			"authenticated" => $authenticated,
			"username" => $data['username']
		);
	}

	/**
	 * Logout user
	 */
	function logout() {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Check permissions
		$this->validateSession();
		$session = &OMVSession::getInstance();
		$session->destroy();
	}
}
?>
