<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   http://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2015 Volker Theile
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
require_once("openmediavault/globals.inc");
require_once("openmediavault/config.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/session.inc");
require_once("openmediavault/rpcservice.inc");
require_once("openmediavault/product.inc");

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
		$authenticated = FALSE;
		// Authenticate the given user. Note, the OMV engine RPC is
		// executed in another context which allows reading the shadow
		// file, otherwise pam_auth() will fail.
		$object = OMVRpc::exec("UserMgmt", "authUser", $params,
		  $this->getAdminContext(), OMV_RPC_MODE_REMOTE, TRUE);
		if (!is_null($object) && (TRUE === $object['authenticated'])) {
			$session = &OMVSession::getInstance();
			if ($session->isAuthenticated()) {
				// Is the current session registered to the user to be
				// authenticated?
				if ($session->getUsername() !== $params['username']) {
					$session->commit();
					throw new OMVException(
					  OMVErrorMsg::E_SESSION_ALREADY_AUTHENTICATED);
				}
			} else {
				// Initialize session.
				$role = ($params['username'] === "admin") ?
				  OMV_ROLE_ADMINISTRATOR : OMV_ROLE_USER;
				$session->initialize($params['username'], $role);
			}
			$authenticated = $session->isAuthenticated();
			$session->commit();
		}
		// Get the product information to customize the syslog identity
		// and cookie name.
		$prd = new OMVProduct();
		$ident = sprintf("%s-webgui", $prd->getName());
		// Open connection to system logger.
		openlog(strtolower($ident), LOG_PID | LOG_PERROR, LOG_AUTH);
		if (TRUE === $authenticated) {
			// Log the successful login attempt via syslog.
			syslog(LOG_ALERT, sprintf("Authorized login from %s ".
			  "[username=%s, user-agent=%s]", $_SERVER['REMOTE_ADDR'],
			  $params['username'], $_SERVER['HTTP_USER_AGENT']));
			// Send an email to the user if a new web browser is used to log
			// in to the control panel. If no special cookie is detected,
			// then it is assumed that the web browser has not been used to
			// log in until now. Note, the cookie expires after 60 days.
			// The salted user name hash is used as identifier.
			$firstLogIn = TRUE;
			foreach ($_COOKIE as $cookiek => $cookiev) {
				$regex = sprintf('/^X-%s-(.+)\$/', strtoupper($ident));
				if (1 !== preg_match($regex, $cookiek, $matches))
					continue;
				if (crypt($params['username'], $matches[1])) {
					$firstLogIn = FALSE;
					break;
				}
			}
			if (TRUE === $firstLogIn) {
				// Send the notification email to the user.
				$subject = sprintf("Your user account was used to log in to ".
				  "the %s control panel via a web browser.", $prd->getName());
				$message = sprintf("Dear %s,\n\n".
				  "your user account was used to log in to %s (%s) via a web browser.\n\n".
				  "Date and time: %s\n".
				  "Remote address: %s\n".
				  "User agent: %s\n\n".
				  "If you recently logged in to the control panel, you can disregard this email. ".
				  "If you have not logged in recently and believe someone may have accessed ".
				  "your account, you should reset your password.\n\n".
				  "Sincerely,\n".
				  "your %s system",
				  $params['username'], $_SERVER['HTTP_HOST'],
				  $_SERVER['SERVER_ADDR'], date('r'), $_SERVER['REMOTE_ADDR'],
				  $_SERVER['HTTP_USER_AGENT'], $prd->getName());
				OMVUtil::mail('root', $params['username'], $subject, $message);
				// Set a cookie. It will expire after 60 days.
				setcookie(sprintf("X-%s-%s", strtoupper($ident), crypt(
				  $params['username'])), array_rand_value(
				  $GLOBALS['OMV_DUNE_QUOTES']), time() + 60 * 60 * 24 * 60);
			}
		} else {
			// Log the unauthorized login attempt via syslog.
			syslog(LOG_ALERT, sprintf("Unauthorized login attempt from %s ".
			  "[username=%s, user-agent=%s]", $_SERVER['REMOTE_ADDR'],
			  $params['username'], $_SERVER['HTTP_USER_AGENT']));
		}
		closelog();
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
