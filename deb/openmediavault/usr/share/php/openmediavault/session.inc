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
require_once("openmediavault/object.inc");
require_once("openmediavault/globals.inc");
require_once("openmediavault/error.inc");
require_once("openmediavault/config.inc");

/**
 * @ingroup api
 */
class OMVSession extends OMVObject {
	private static $instance = NULL;

	/**
	 * Returns the session singleton.
	 * @return The session object.
	 */
	public static function &getInstance() {
		if(is_null(self::$instance)) {
			self::$instance = new OMVSession();
		}
		return self::$instance;
	}

	/**
	 * Start session.
	 */
	public function start() {
		session_start();
	}

	/**
	 * Write session data and end session.
	 */
	public function commit() {
		session_commit();
	}

	/**
	 * Destroy session.
	 */
	public function destroy() {
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
		if(array_key_exists("REMOTE_ADDR", $_SERVER))
			$_SESSION['ipaddress'] = $_SERVER['REMOTE_ADDR'];
		if(array_key_exists("HTTP_USER_AGENT", $_SERVER))
			$_SESSION['useragent'] = $_SERVER['HTTP_USER_AGENT'];
		$this->updateLastAccess();
		return session_id();
	}

	/**
	 * Get the role of the user from this session.
	 * @return Returns the user role, otherwise FALSE.
	 */
	public function getRole() {
		if(!isset($_SESSION['role']))
			return FALSE;
		return $_SESSION['role'];
	}

	/**
	 * Is this session authenticated?
	 * @return Returns TRUE if the session is authenticated, otherwise FALSE.
	 */
	public function isAuthenticated() {
		if(!isset($_SESSION['authenticated']) || !$_SESSION['authenticated'])
			return FALSE;
	 	return TRUE;
	}

	/**
	 * Get the name of the user from this session.
	 * @return Returns the current user name, otherwise FALSE.
	 */
	public function getUsername() {
		if(!isset($_SESSION['username']))
			return FALSE;
	 	return $_SESSION['username'];
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
	public function isTimeout() {
		global $xmlConfig;
		if(!isset($_SESSION['lastaccess']))
			return FALSE;
		// Get session timeout from configuration. Default is 5 minutes.
		$timeout = 300;
		$object = $xmlConfig->get("//webadmin");
		if(!is_null($object) && array_key_exists("timeout", $object)) {
			$timeout = intval($object['timeout']) * 60;
			// If the timeout value is set the 0 the session timeout
			// validation is disabled.
			if(0 == $timeout)
				return FALSE;

		}
		return ((time() - $_SESSION['lastaccess']) > $timeout) ? TRUE : FALSE;
	}

	/**
	 * Do some checks. In case of an error an exception is thrown.
	 */
	public function validate() {
		$exception = null;
		if(array_key_exists("REMOTE_ADDR", $_SERVER)) {
			if($_SESSION['ipaddress'] !== $_SERVER['REMOTE_ADDR']) {
				$exception = new OMVException(
				  OMVErrorMsg::E_SESSION_INVALID_IPADDRESS);
			}
		}
		if(array_key_exists("HTTP_USER_AGENT", $_SERVER)) {
			if($_SESSION['useragent'] !== $_SERVER['HTTP_USER_AGENT']) {
				$exception = new OMVException(
				  OMVErrorMsg::E_SESSION_INVALID_USERAGENT);
			}
		}
		if($this->isTimeout()) {
			$exception = new OMVException(OMVErrorMsg::E_SESSION_TIMEOUT);
		} else {
			// Check if user still exists, maybe it has been deleted by the
			// administrator in the meanwhile.
			$userinfo = posix_getpwnam($_SESSION['username']);
			if($userinfo === FALSE) {
				$exception = new OMVException(
				  OMVErrorMsg::E_SESSION_INVALID_USER);
			}
		}
		if(!is_null($exception)) {
			$this->destroy();
			throw $exception;
		}
	}

	/**
	 * Dump the current session informations.
	 */
	public function dump() {
		$this->debug(var_export(array(
			  "id" => session_id(),
			  "data" => $_SESSION
		  ), TRUE));
	}
}
?>
