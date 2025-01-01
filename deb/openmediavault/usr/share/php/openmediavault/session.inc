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
namespace OMV;

require_once("openmediavault/functions.inc");

/**
 * @ingroup api
 */
class Session {
	/**
	 * Returns the session singleton.
	 * @return The session object.
	 */
	public static function &getInstance() {
		static $instance = NULL;
		if (!isset($instance)) {
			$instance = new Session();
		}
		return $instance;
	}

	/**
	 * Start session.
	 */
	public function start() {
		$sessionId = array_value($_SERVER,
			"HTTP_X_OPENMEDIAVAULT_SESSIONID", FALSE);
		if (FALSE !== $sessionId) {
			session_id($sessionId);
		}
		if (FALSE === session_start()) {
			throw new HttpErrorException(500,
				_("Failed to start session."));
		}
	}

	/**
	 * Write session data and end session.
	 */
	public function commit() {
		if (FALSE === session_commit()) {
			throw new HttpErrorException(500,
				_("Failed to write session data."));
		}
	}

	/**
	 * Destroy session.
	 */
	public function destroy() {
		// Destroy the session cookie.
		$params = session_get_cookie_params();
		if (is_assoc_array($params)) {
			setcookie(session_name(), "", time() - (60 * 60 * 24),
				$params["path"], $params["domain"],
				$params["secure"], $params["httponly"]
			);
		}
		// Destroy all data registered to this session.
		session_destroy();
	}

	/**
	 * Initialize the session.
	 * @param username The user name.
	 * @param role The user role.
	 * @return The session id.
	 */
	public function initialize($username, $role) {
		session_regenerate_id();
		$_SESSION['authenticated'] = TRUE;
		$_SESSION['username'] = $username;
		$_SESSION['role'] = $role;
		if (array_key_exists("REMOTE_ADDR", $_SERVER)) {
			$_SESSION['ipaddress'] = $_SERVER['REMOTE_ADDR'];
		}
		if (array_key_exists("HTTP_USER_AGENT", $_SERVER)) {
			$_SESSION['useragent'] = $_SERVER['HTTP_USER_AGENT'];
		}
		$this->updateLastAccess();
		return session_id();
	}

	/**
	 * Get the role of the user from this session.
	 * @return Returns the user role, otherwise FALSE.
	 */
	public function getRole() {
		return array_value($_SESSION, "role", FALSE);
	}

	/**
	 * Is this session authenticated?
	 * @return Returns TRUE if the session is authenticated, otherwise FALSE.
	 */
	public function isAuthenticated(): bool {
		return array_value($_SESSION, "authenticated", FALSE);
	}

	/**
	 * Get the name of the user from this session.
	 * @return Returns the current user name, otherwise FALSE.
	 */
	public function getUsername() {
		return array_value($_SESSION, "username", FALSE);
	}

	/**
	 * Update the time on which the last access took place.
	 */
	public function updateLastAccess() {
		$_SESSION['lastaccess'] = time();
	}

	/**
	 * Check if the last access if not older than the defined timeout value.
	 * @return TRUE if the session is timed out, otherwise FALSE.
	 */
	public function isTimeout(): bool {
		if (!isset($_SESSION['lastaccess'])) {
			return FALSE;
		}
		try {
			// Get session timeout from the configuration.
			$db = &\OMV\Config\Database::getInstance();
			$object = $db->get("conf.webadmin");
			$timeout = $object->get("timeout") * 60;
		} catch(\Exception $e) {
			// Set default to 5 minutes.
			$timeout = 300;
		}
		// If the timeout value is set the 0 the session timeout
		// validation is disabled.
		if (0 == $timeout) {
			return FALSE;
		}
		return ((time() - $_SESSION['lastaccess']) > $timeout) ? TRUE : FALSE;
	}

	/**
	 * Validate the authentication. In case of an error the session is
	 * destroyed and an exception is thrown.
	 */
	public function validateAuthentication() {
		if (!$this->isAuthenticated()) {
			$this->destroy();
			throw new HttpErrorException(401, _("Session not authenticated."));
		}
	}

	/**
	 * Validate the timeout. In case of an error the session is
	 * destroyed and an exception is thrown.
	 */
	public function validateTimeout() {
		if ($this->isTimeout()) {
			$this->destroy();
			throw new HttpErrorException(401, _("Session expired."));
		}
	}

	/**
	 * Validate the user. In case of an error the session is
	 * destroyed and an exception is thrown.
	 */
	public function validateUser() {
		// Check if user still exists, maybe it has been deleted by the
		// administrator in the meanwhile.
		$userInfo = posix_getpwnam($_SESSION['username']);
		if (FALSE === $userInfo) {
			$this->destroy();
			throw new HttpErrorException(401, _("Invalid user."));
		}
	}

	/**
	 * Validate the IP address. In case of an error the session is
	 * destroyed and an exception is thrown.
	 */
	public function validateIpAddress() {
		if (!array_key_exists("REMOTE_ADDR", $_SERVER)) {
			return;
		}
		if ($_SESSION['ipaddress'] == $_SERVER['REMOTE_ADDR']) {
			return;
		}
		$this->destroy();
		throw new HttpErrorException(401, _("Invalid IP address."));
	}

	/**
	 * Validate the browser user agent. In case of an error the session
	 * is destroyed and an exception is thrown.
	 */
	public function validateUserAgent() {
		if (!array_key_exists("HTTP_USER_AGENT", $_SERVER)) {
			return;
		}
		if ($_SESSION['useragent'] == $_SERVER['HTTP_USER_AGENT']) {
			return;
		}
		$this->destroy();
		throw new HttpErrorException(401, _("Invalid User-Agent."));
	}

	/**
	 * Check if the session is valid. In case of an error the session is
	 * destroyed and an exception is thrown.
	 */
	public function validate() {
		$this->validateAuthentication();
		// $this->validateIpAddress();
		$this->validateUserAgent();
		$this->validateTimeout();
		$this->validateUser();
	}

	/**
	 * Dump the current session information.
	 */
	public function dump() {
		syslog(LOG_DEBUG, var_export([
			"id" => session_id(),
			"data" => $_SESSION
		], TRUE));
	}
}
