<?php
/**
 * This file is part of OpenMediaVault.
 *
 * @license   https://www.gnu.org/licenses/gpl.html GPL Version 3
 * @author    Volker Theile <volker.theile@openmediavault.org>
 * @copyright Copyright (c) 2009-2026 Volker Theile
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
 * along with OpenMediaVault. If not, see <https://www.gnu.org/licenses/>.
 */
require_once("openmediavault/totp.php");

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
		$this->registerMethod("verifyTotp");
	}

	/**
	 * Login user (step 1 of 1, or step 1 of 2 when MFA is enabled).
	 *
	 * When the user has TOTP MFA enabled the response will contain
	 * mfaRequired=TRUE and authenticated=FALSE. The client must then call
	 * verifyTotp() with the code from the authenticator app to complete login.
	 *
	 * @param params An array containing:
	 *   \em username The name of the user.
	 *   \em password The user's password.
	 * @param context The context of the caller.
	 * @return An array with:
	 *   \em authenticated  TRUE when fully logged in (no MFA, or MFA skipped).
	 *   \em mfaRequired    TRUE when a TOTP code is still required.
	 *   \em username       The authenticated username.
	 *   \em permissions    User permissions object.
	 *   \em sessionid      Session ID to send in subsequent requests.
	 */
	final public function login($params, $context) {
		// Validate the parameters of the RPC service method.
		$this->validateMethodParams($params, "rpc.session.login");
		$authenticated = FALSE;
		$permissions   = [];
		$sessionId     = "";
		$server        = new \OMV\SuperGlobalServer();
		// Strip whitespaces from the user name.
		if (is_string($params['username'])) {
			$params['username'] = trim($params['username']);
		}
		// Authenticate the given user via PAM. The OMV engine RPC is
		// executed in a separate context that can read the shadow file.
		$object = \OMV\Rpc\Rpc::call("UserMgmt", "authUser", $params,
			$this->getAdminContext(), \OMV\Rpc\Rpc::MODE_REMOTE, TRUE);
		if (!is_null($object) && (TRUE === $object['authenticated'])) {
			$permissions = $object['permissions'];
			$role = ("admin" == $permissions['role']) ?
				OMV_ROLE_ADMINISTRATOR : OMV_ROLE_USER;
			$session = &\OMV\Session::getInstance();
			// Check whether this user has TOTP MFA enabled. If so, initialize
			// a pending session and return early — the client must submit a
			// TOTP code via verifyTotp() before receiving a full session.
			if (\OMV\Totp::isEnabledForUser($params['username'])) {
				// Reject if an existing authenticated session belongs to a
				// different user (mirrors the check on the non-MFA path).
				if ($session->isAuthenticated() &&
					$session->getUsername() !== $params['username']) {
					$session->commit();
					throw new \OMV\HttpErrorException(400,
						_("Another user is already authenticated."));
				}
				if (!$session->isMfaPending()) {
					$sessionId = $session->initializeMfaPending(
						$params['username'], $role);
				} else {
					// Session is already pending; return its existing ID so
					// the client can continue submitting TOTP codes.
					$sessionId = session_id();
				}
				$session->commit();
				// Log the intermediate step so administrators can see it.
				$prd   = new \OMV\ProductInfo();
				$ident = mb_strtolower(sprintf("%s-webgui", $prd->getName()));
				openlog($ident, LOG_PID | LOG_PERROR, LOG_AUTH);
				$_ = defer("closelog");
				syslog(LOG_INFO, sprintf("MFA challenge issued to %s ".
					"[username=%s, user-agent=%s]", $server->getClientIP(),
					$params['username'], $server->getUserAgent()));
				return [
					"authenticated" => FALSE,
					"mfaRequired"   => TRUE,
					"username"       => $params['username'],
					"permissions"    => $permissions,
					"sessionid"      => $sessionId,
				];
			}
			// No MFA — proceed with normal session initialization.
			if ($session->isAuthenticated()) {
				// Reject if the existing session belongs to a different user.
				if ($session->getUsername() !== $params['username']) {
					$session->commit();
					throw new \OMV\HttpErrorException(400,
					    _("Another user is already authenticated."));
				}
			} else {
				$sessionId = $session->initialize($params['username'], $role);
			}
			$authenticated = $session->isAuthenticated();
			$session->commit();
		}
		// Log the outcome and handle first-login notification/cookie.
		$this->finalizeLogin($authenticated, $params['username']);
		return [
			"authenticated" => $authenticated,
			"mfaRequired"   => FALSE,
			"username"      => $params['username'],
			"permissions"   => $permissions,
			"sessionid"     => $sessionId,
		];
	}

	/**
	 * Complete login for users with TOTP MFA enabled (step 2 of 2).
	 *
	 * Must be called after a successful login() that returned mfaRequired=TRUE.
	 * The session must still be active (use the sessionid from login()). On
	 * success the session is promoted to fully authenticated.
	 *
	 * A maximum of 5 failed attempts is allowed per pending session; exceeding
	 * this limit destroys the session and requires starting over.
	 *
	 * @param params An array containing:
	 *   \em code The 6-digit TOTP code from the user's authenticator app.
	 * @param context The context of the caller.
	 * @return An array with:
	 *   \em authenticated  TRUE on success.
	 *   \em username       The authenticated username.
	 *   \em permissions    User permissions object.
	 *   \em sessionid      The (unchanged) session ID.
	 */
	final public function verifyTotp($params, $context) {
		// Validate input schema.
		$this->validateMethodParams($params, "rpc.session.verifytotp");
		$session = &\OMV\Session::getInstance();
		// This method only applies to sessions waiting for TOTP verification.
		if (!$session->isMfaPending()) {
			throw new \OMV\HttpErrorException(400,
				_("No MFA verification is pending for this session."));
		}
		$username = $session->getUsername();
		$server   = new \OMV\SuperGlobalServer();
		// Retrieve the stored TOTP secret and verify the submitted code.
		$secret = \OMV\Totp::getSecretForUser($username);
		$code = trim($params['code']);
		if (empty($secret) || !\OMV\Totp::verifyCode($secret, $code)) {
			// Record the failure and enforce a retry limit.
			$attempts = $session->incrementTotpAttempts();
			$session->commit();
			$prd   = new \OMV\ProductInfo();
			$ident = mb_strtolower(sprintf("%s-webgui", $prd->getName()));
			openlog($ident, LOG_PID | LOG_PERROR, LOG_AUTH);
			$_ = defer("closelog");
			// After 5 failed attempts, invalidate the pending session to
			// prevent an attacker from cycling through all 1,000,000 codes.
			if ($attempts >= 5) {
				syslog(LOG_ALERT, sprintf("TOTP brute-force lockout from %s ".
					"[username=%s, attempts=%d]", $server->getClientIP(),
					$username, $attempts));
				$session->destroy();
				throw new \OMV\HttpErrorException(401,
					_("Too many failed TOTP attempts. Please log in again."));
			}
			syslog(LOG_ALERT, sprintf("Invalid TOTP code from %s ".
				"[username=%s, attempt=%d]", $server->getClientIP(),
				$username, $attempts));
			throw new \OMV\HttpErrorException(400,
				_("Invalid verification code. Please try again."));
		}
		// Code is valid — promote the session to fully authenticated.
		$session->completeMfaAuthentication();
		$sessionId = session_id();
		// Rebuild the permissions the same way login() does.
		$isAdmin = ($session->getRole() === OMV_ROLE_ADMINISTRATOR);
		$role = $isAdmin ? "admin" : "user";
		$permissions = ["role" => $role];
		$session->commit();
		// Log the successful full login and send the first-login notification.
		$this->finalizeLogin(TRUE, $username);
		return [
			"authenticated" => TRUE,
			"mfaRequired"   => FALSE,
			"username"      => $username,
			"permissions"   => $permissions,
			"sessionid"     => $sessionId,
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

	/**
	 * Log the final outcome of a login attempt and, on success, send the
	 * first-login email notification and set the browser-recognition cookie.
	 *
	 * Extracted into a helper so both login() (no-MFA path) and verifyTotp()
	 * (MFA path) share identical post-authentication behaviour.
	 *
	 * @param bool   $authenticated Whether authentication succeeded.
	 * @param string $username      The username that was authenticated.
	 */
	private function finalizeLogin(
		bool $authenticated,
		string $username
	): void {
		$server = new \OMV\SuperGlobalServer();
		$prd    = new \OMV\ProductInfo();
		$ident  = mb_strtolower(sprintf("%s-webgui", $prd->getName()));
		$prefix = mb_strtoupper(sprintf("%s-LOGIN", $prd->getName()));
		openlog($ident, LOG_PID | LOG_PERROR, LOG_AUTH);
		$_ = defer("closelog");
		if (TRUE === $authenticated) {
			syslog(LOG_ALERT, sprintf("Authorized login from %s ".
				"[username=%s, user-agent=%s]", $server->getClientIP(),
				$username, $server->getUserAgent()));
			// Detect whether this browser has logged in before by looking for
			// the recognition cookie set on a prior login. The cookie holds a
			// bcrypt hash of the username hex-encoded to avoid dot→underscore
			// conversion by PHP.
			$firstLogIn = TRUE;
			foreach ($_COOKIE as $cookiek => $cookiev) {
				$regex = sprintf('/^%s-(.+)$/', $prefix);
				if (1 !== preg_match($regex, $cookiek, $matches)) {
					continue;
				}
				$hashedUsername = hex2bin($matches[1]);
				if (FALSE === $hashedUsername) {
					continue;
				}
				if (password_verify($username, $hashedUsername)) {
					$firstLogIn = FALSE;
					break;
				}
			}
			if (TRUE === $firstLogIn) {
				// Send notification email if the feature is enabled.
				try {
					$notificationEnabled = \OMV\Rpc\Rpc::call("Notification",
						"isEnabled", ["id" => "authentication"],
						$this->getAdminContext(), \OMV\Rpc\Rpc::MODE_REMOTE);
				} catch (\Exception $e) {
					$notificationEnabled = TRUE;
				}
				if (TRUE == $notificationEnabled) {
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
						$username, \OMV\System\Net\Dns::getFqdn(),
						$server->getServerAddr(), date('r'), $server->getClientIP(),
						$server->getUserAgent(), $prd->getName());
					$mail = new \OMV\Email("root", $username, $subject, $message);
					$mail->send();
				}
				// Set the browser-recognition cookie (expires in 60 days).
				$hashedUsername = password_hash($username, PASSWORD_DEFAULT);
				$key   = sprintf("%s-%s", $prefix, bin2hex($hashedUsername));
				$value = array_rand_value(\OMV\Environment::get("OMV_DUNE_QUOTES"));
				setcookie($key, $value, [
					'expires'  => time() + 60 * 60 * 24 * 60,
					'httponly' => TRUE,
					'samesite' => 'Strict',
				]);
			}
		} else {
			syslog(LOG_ALERT, sprintf("Unauthorized login attempt from %s ".
				"[username=%s, user-agent=%s]", $server->getClientIP(),
				$username, $server->getUserAgent()));
			throw new \OMV\HttpErrorException(400,
				_("Incorrect username or password."));
		}
	}
}
