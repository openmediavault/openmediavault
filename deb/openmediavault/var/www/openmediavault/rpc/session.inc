<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2014 Volker Theile
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
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/session.inc");
require_once("openmediavault/rpcservice.inc");

class OMVRpcServiceSession extends OMVRpcServiceAbstract {
	/**
	 * Get the RPC service name.
	 */
	final public function getName() {
		return "Session";
	}

	/**
	 * Initialize the RPC service.
	 */
	final public function initialize() {
		$this->registerMethod("login");
		$this->registerMethod("logout");
	}

	/**
	 * Login user.
	 * @param params The method parameters containing the following fields:
	 *   \em username The name of the user.
	 *   \em password The password.
	 * @param context The context of the caller.
	 * @return An array containing the fields \em authenticated which is TRUE
	 *   if authentication was successful, otherwise FALSE. The name of the
	 *   user is in \em username.
	 * @throw E_RPC_SERVICE_METHOD_INVALID_CONTEXT
	 */
	final public function login($params, $context) {
//		$this->debug(var_export(func_get_args(), TRUE));
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, '{
			  "type":"object",
			  "properties":{
				  "username":{"type":"string"},
				  "password":{"type":"string"}
			  }
		  }');
		// Authenticate the given user.
		$object = OMVRpc::exec("UserMgmt", "authUser", $params,
		  $this->getAdminContext(), OMV_RPC_MODE_REMOTE);
		$authenticated = FALSE;
		if(!is_null($object) && (TRUE === $object['authenticated'])) {
			$session = &OMVSession::getInstance();
			if($session->isAuthenticated()) {
				// Is the current session registered to the user to be
				// authenticated?
				if($session->getUsername() !== $object['username']) {
					$session->commit();
					throw new OMVException(
					  OMVErrorMsg::E_SESSION_ALREADY_AUTHENTICATED);
				}
			} else {
				// Initialize session.
				$role = ($object['username'] === "admin") ?
				  OMV_ROLE_ADMINISTRATOR : OMV_ROLE_USER;
				$session->initialize($object['username'], $role);
			}
			$authenticated = $session->isAuthenticated();
			$session->commit();
		}
		if(FALSE === $authenticated) {
			syslog(LOG_ALERT, sprintf("Unauthorized login attempt from %s ".
			  "[username='%s', user-agent=%s]", $_SERVER['REMOTE_ADDR'],
			  $params['username'], $_SERVER['HTTP_USER_AGENT']));
		}
		return array(
			"authenticated" => $authenticated,
			"username" => $params['username']
		);
	}

	/**
	 * Logout user.
	 */
	final public function logout($params, $context) {
		// Check permissions
		$session = &OMVSession::getInstance();
		if(!$session->isAuthenticated()) {
			throw new OMVException(
			  OMVErrorMsg::E_SESSION_NOT_AUTHENTICATED);
		}
		$session->validate();
		$session->destroy();
	}
}

// Register the RPC service.
$rpcServiceMgr = &OMVRpcServiceMgr::getInstance();
$rpcServiceMgr->registerService(new OMVRpcServiceSession());
?>
