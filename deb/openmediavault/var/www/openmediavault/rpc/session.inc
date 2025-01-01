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
class OMVRpcServiceSession extends \OMV\Rpc\ServiceAbstract {
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
	 */
	final public function login($params, $context) {
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.session.login");
		$authenticated = FALSE;
		$permissions = [];
		$sessionId = "";
		$server = new \OMV\SuperGlobalServer();
		// Strip whitespaces from the user name.
		if (is_string($params['username'])) {
			$params['username'] = trim($params['username']);
		}
		// Authenticate the given user. Note, the OMV engine RPC is
		// executed in another context which allows reading the shadow
		// file, otherwise pam_auth() will fail.
		$object = \OMV\Rpc\Rpc::call("UserMgmt", "authUser", $params,
			$this->getAdminContext(), \OMV\Rpc\Rpc::MODE_REMOTE, TRUE);
		if (!is_null($object) && (TRUE === $object['authenticated'])) {
			$permissions = $object['permissions'];
			$session = &\OMV\Session::getInstance();
			if ($session->isAuthenticated()) {
				// Is the current session registered to the user to be
				// authenticated?
				if ($session->getUsername() !== $params['username']) {
					$session->commit();
					throw new \OMV\HttpErrorException(400,
					    _("Another user is already authenticated."));
				}
			} else {
				// Initialize session.
				$role = ("admin" == $permissions['role']) ?
					OMV_ROLE_ADMINISTRATOR : OMV_ROLE_USER;
				$sessionId = $session->initialize($params['username'],
					$role);
			}
			$authenticated = $session->isAuthenticated();
			$session->commit();
		}
		// Get the product information to customize the syslog identity
		// and cookie name.
		$prd = new \OMV\ProductInfo();
		$ident = mb_strtolower(sprintf("%s-webgui", $prd->getName()));
		$prefix = mb_strtoupper(sprintf("%s-LOGIN", $prd->getName()));
		// Open connection to system logger.
		openlog($ident, LOG_PID | LOG_PERROR, LOG_AUTH);
		if (TRUE === $authenticated) {
			// Log the successful login attempt via syslog.
			syslog(LOG_ALERT, sprintf("Authorized login from %s ".
				"[username=%s, user-agent=%s]", $server->getClientIP(),
				$params['username'], $server->getUserAgent()));
			// Send an email to the user if a new web browser is used to log
			// in to the control panel. If no special cookie is detected,
			// then it is assumed that the web browser has not been used to
			// log in until now. Note, the cookie expires after 60 days.
			// The salted user name hash is used as identifier.
			$firstLogIn = TRUE;
			foreach ($_COOKIE as $cookiek => $cookiev) {
				// Extract the hashed username. Note, the hash is encoded
				// to meet RFC 2616.
				$regex = sprintf('/^%s-(.+)$/', $prefix);
				if (1 !== preg_match($regex, $cookiek, $matches)) {
					continue;
				}
				$hashedUsername = urldecode($matches[1]);
				if (password_verify($params['username'], $hashedUsername)) {
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
					$params['username'], \OMV\System\Net\Dns::getFqdn(),
					$server->getServerAddr(), date('r'), $server->getClientIP(),
					$server->getUserAgent(), $prd->getName());
				$mail = new \OMV\Email("root", $params['username'],
					$subject, $message);
				$mail->send();
				// Set a cookie. It will expire after 60 days. The user name
				// is crypted/hashed to be unable to reverse engineer the
				// account names from the cookie. Finally encode the hashed
				// user name to meet RFC 2616.
				$hashedUsername = password_hash($params['username'],
					PASSWORD_DEFAULT);
				$key = sprintf("%s-%s", $prefix, urlencode($hashedUsername));
				$value = array_rand_value(\OMV\Environment::get(
					"OMV_DUNE_QUOTES"));
				$expire = time() + 60 * 60 * 24 * 60;
				setcookie($key, $value, [
					'expires' => $expire,
					'httponly' => TRUE,
					'samesite' => 'Strict'
				]);
			}
		} else {
			// Log the unauthorized login attempt via syslog.
			syslog(LOG_ALERT, sprintf("Unauthorized login attempt from %s ".
				"[username=%s, user-agent=%s]", $server->getClientIP(),
				$params['username'], $server->getUserAgent()));
			closelog();
			throw new \OMV\HttpErrorException(400,
				_("Incorrect username or password."));
		}
		closelog();
		return [
			"authenticated" => $authenticated,
			"username" => $params['username'],
			"permissions" => $permissions,
			"sessionid" => $sessionId
		];
	}

	/**
	 * Logout user.
	 */
	final public function logout($params, $context) {
		// Check the permissions and destroy the session.
		$session = &\OMV\Session::getInstance();
		$session->validate();
		$session->destroy();
	}
}
